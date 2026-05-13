#include "semantic/semantic_action_executor.h"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace compilerlab::semantic {

namespace {

struct ActionExpr {
    enum class Kind {
        Placeholder,
        Null,
        Identifier,
        Call,
    };

    Kind kind {Kind::Identifier};
    int placeholder_index {0};
    std::string text;
    std::vector<ActionExpr> args;
};

class ActionParser {
public:
    explicit ActionParser(std::string_view text)
        : text_(text) {
    }

    std::optional<ActionExpr> parse() {
        skip_space();
        if (match("$$")) {
            skip_space();
            if (!consume('=')) {
                return std::nullopt;
            }
        }

        skip_space();
        auto expr = parse_expr();
        if (!expr.has_value()) {
            return std::nullopt;
        }

        skip_space();
        consume(';');
        skip_space();
        if (!at_end()) {
            return std::nullopt;
        }
        return expr;
    }

private:
    std::string_view text_;
    std::size_t position_ {0};

    bool at_end() const {
        return position_ >= text_.size();
    }

    void skip_space() {
        while (!at_end() && std::isspace(static_cast<unsigned char>(text_[position_])) != 0) {
            ++position_;
        }
    }

    bool consume(char ch) {
        if (!at_end() && text_[position_] == ch) {
            ++position_;
            return true;
        }
        return false;
    }

    bool match(std::string_view prefix) {
        if (text_.substr(position_, prefix.size()) == prefix) {
            position_ += prefix.size();
            return true;
        }
        return false;
    }

    std::optional<ActionExpr> parse_expr() {
        skip_space();
        if (at_end()) {
            return std::nullopt;
        }

        if (consume('$')) {
            if (at_end() || std::isdigit(static_cast<unsigned char>(text_[position_])) == 0) {
                return std::nullopt;
            }
            int index = 0;
            while (!at_end() && std::isdigit(static_cast<unsigned char>(text_[position_])) != 0) {
                index = index * 10 + (text_[position_] - '0');
                ++position_;
            }
            ActionExpr expr;
            expr.kind = ActionExpr::Kind::Placeholder;
            expr.placeholder_index = index;
            return expr;
        }

        const auto identifier = parse_identifier();
        if (identifier.empty()) {
            return std::nullopt;
        }

        if (identifier == "nullptr") {
            ActionExpr expr;
            expr.kind = ActionExpr::Kind::Null;
            expr.text = "nullptr";
            return expr;
        }

        skip_space();
        if (consume('(')) {
            ActionExpr expr;
            expr.kind = ActionExpr::Kind::Call;
            expr.text = identifier;

            skip_space();
            if (consume(')')) {
                return expr;
            }

            while (true) {
                auto arg = parse_expr();
                if (!arg.has_value()) {
                    return std::nullopt;
                }
                expr.args.push_back(std::move(*arg));
                skip_space();
                if (consume(')')) {
                    break;
                }
                if (!consume(',')) {
                    return std::nullopt;
                }
                skip_space();
            }
            return expr;
        }

        ActionExpr expr;
        expr.kind = ActionExpr::Kind::Identifier;
        expr.text = identifier;
        return expr;
    }

    std::string parse_identifier() {
        skip_space();
        const auto start = position_;

        while (!at_end()) {
            const char ch = text_[position_];
            const bool ident_char =
                std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_' || ch == ':';
            if (!ident_char) {
                break;
            }
            ++position_;
        }

        if (start == position_) {
            return {};
        }
        return std::string(text_.substr(start, position_ - start));
    }
};

common::SourceSpan span_from_value(const SemanticValue& value) {
    if (const auto* token = value.get_if<common::Token>()) {
        return token->span;
    }
    if (const auto* program = value.get_if<ProgramPtr>(); program != nullptr && *program) {
        return (*program)->span;
    }
    if (const auto* decl = value.get_if<DeclPtr>(); decl != nullptr && *decl) {
        return (*decl)->span;
    }
    if (const auto* param = value.get_if<ParameterDeclPtr>(); param != nullptr && *param) {
        return (*param)->span;
    }
    if (const auto* stmt = value.get_if<StmtPtr>(); stmt != nullptr && *stmt) {
        return (*stmt)->span;
    }
    if (const auto* expr = value.get_if<ExprPtr>(); expr != nullptr && *expr) {
        return (*expr)->span;
    }
    return {};
}

std::string token_text(const SemanticValue& value) {
    if (const auto* token = value.get_if<common::Token>()) {
        return token->lexeme;
    }
    return {};
}

std::int64_t integer_token_value(const SemanticValue& value) {
    if (const auto* token = value.get_if<common::Token>()) {
        return std::strtoll(token->lexeme.c_str(), nullptr, 10);
    }
    return 0;
}

double float_token_value(const SemanticValue& value) {
    if (const auto* token = value.get_if<common::Token>()) {
        return std::strtod(token->lexeme.c_str(), nullptr);
    }
    return 0.0;
}

std::string enum_name(std::string_view text) {
    const auto pos = text.rfind("::");
    if (pos == std::string_view::npos) {
        return std::string(text);
    }
    return std::string(text.substr(pos + 2));
}

template <typename T>
const T* expect_value(const SemanticValue& value) {
    return value.get_if<T>();
}

}  // namespace

class ActionEvaluator {
public:
    ActionEvaluator(const SemanticActionExecutor& owner,
                    const runtime::ProductionInfo& production,
                    const std::vector<SemanticValue>& rhs_values)
        : owner_(owner), production_(production), rhs_values_(rhs_values) {
    }

    SemanticValue evaluate(const ActionExpr& expr) const {
        switch (expr.kind) {
            case ActionExpr::Kind::Placeholder:
                if (expr.placeholder_index <= 0 ||
                    static_cast<std::size_t>(expr.placeholder_index) > rhs_values_.size()) {
                    owner_.diagnostics().error("semantic action placeholder out of range in production: " +
                                               production_.debug_name);
                    return {};
                }
                return rhs_values_[static_cast<std::size_t>(expr.placeholder_index - 1)];

            case ActionExpr::Kind::Null:
                return SemanticValue {};

            case ActionExpr::Kind::Identifier:
                return evaluate_identifier(expr.text);

            case ActionExpr::Kind::Call:
                return evaluate_call(expr.text, expr.args);
        }
        return {};
    }

private:
    const SemanticActionExecutor& owner_;
    const runtime::ProductionInfo& production_;
    const std::vector<SemanticValue>& rhs_values_;

    SemanticValue evaluate_identifier(const std::string& text) const {
        const auto name = enum_name(text);
        if (name == "Int") {
            return TypeSpecifier::Int;
        }
        if (name == "Float") {
            return TypeSpecifier::Float;
        }
        if (name == "Void") {
            return TypeSpecifier::Void;
        }
        return std::string(name);
    }

    common::SourceSpan merge_rhs_spans() const {
        common::SourceSpan merged;
        for (const auto& value : rhs_values_) {
            merged = owner_.factory().merge(merged, span_from_value(value));
        }
        return merged;
    }

    DeclList expect_decl_list(const SemanticValue& value) const {
        if (const auto* list = expect_value<DeclList>(value)) {
            return *list;
        }
        owner_.diagnostics().error("expected declaration list in production: " + production_.debug_name);
        return {};
    }

    DeclPtr expect_decl(const SemanticValue& value) const {
        if (const auto* decl = expect_value<DeclPtr>(value)) {
            return *decl;
        }
        owner_.diagnostics().error("expected declaration value in production: " + production_.debug_name);
        return {};
    }

    ParameterList expect_param_list(const SemanticValue& value) const {
        if (const auto* list = expect_value<ParameterList>(value)) {
            return *list;
        }
        owner_.diagnostics().error("expected parameter list in production: " + production_.debug_name);
        return {};
    }

    ParameterDeclPtr expect_param(const SemanticValue& value) const {
        if (const auto* param = expect_value<ParameterDeclPtr>(value)) {
            return *param;
        }
        owner_.diagnostics().error("expected parameter value in production: " + production_.debug_name);
        return {};
    }

    StmtList expect_stmt_list(const SemanticValue& value) const {
        if (const auto* list = expect_value<StmtList>(value)) {
            return *list;
        }
        owner_.diagnostics().error("expected statement list in production: " + production_.debug_name);
        return {};
    }

    StmtPtr expect_stmt(const SemanticValue& value) const {
        if (value.empty()) {
            return nullptr;
        }
        if (const auto* stmt = expect_value<StmtPtr>(value)) {
            return *stmt;
        }
        owner_.diagnostics().error("expected statement value in production: " + production_.debug_name);
        return {};
    }

    BlockStmtPtr expect_block_stmt(const SemanticValue& value) const {
        const auto stmt = expect_stmt(value);
        if (!stmt) {
            return nullptr;
        }

        const auto block = std::dynamic_pointer_cast<BlockStmt>(stmt);
        if (!block) {
            owner_.diagnostics().error("expected block statement in production: " + production_.debug_name);
        }
        return block;
    }

    ExprList expect_expr_list(const SemanticValue& value) const {
        if (const auto* list = expect_value<ExprList>(value)) {
            return *list;
        }
        owner_.diagnostics().error("expected expression list in production: " + production_.debug_name);
        return {};
    }

    ExprPtr expect_expr(const SemanticValue& value) const {
        if (value.empty()) {
            return nullptr;
        }
        if (const auto* expr = expect_value<ExprPtr>(value)) {
            return *expr;
        }
        owner_.diagnostics().error("expected expression value in production: " + production_.debug_name);
        return {};
    }

    TypeSpecifier expect_type(const SemanticValue& value) const {
        if (const auto* specifier = expect_value<TypeSpecifier>(value)) {
            return *specifier;
        }
        owner_.diagnostics().error("expected type specifier in production: " + production_.debug_name);
        return TypeSpecifier::Int;
    }

    std::string expect_text(const SemanticValue& value) const {
        if (const auto* text = expect_value<std::string>(value)) {
            return *text;
        }
        if (const auto* token = expect_value<common::Token>(value)) {
            return token->lexeme;
        }
        owner_.diagnostics().error("expected identifier text in production: " + production_.debug_name);
        return {};
    }

    bool expect_arity(std::string_view function_name,
                      const std::vector<SemanticValue>& values,
                      std::size_t expected_count) const {
        if (values.size() == expected_count) {
            return true;
        }

        owner_.diagnostics().error("semantic action `" + std::string(function_name) +
                                   "` expected " + std::to_string(expected_count) +
                                   " argument(s) but received " + std::to_string(values.size()) +
                                   " in production: " + production_.debug_name);
        return false;
    }

    bool expect_arity_between(std::string_view function_name,
                              const std::vector<SemanticValue>& values,
                              std::size_t minimum_count,
                              std::size_t maximum_count) const {
        if (values.size() >= minimum_count && values.size() <= maximum_count) {
            return true;
        }

        owner_.diagnostics().error("semantic action `" + std::string(function_name) +
                                   "` expected between " + std::to_string(minimum_count) +
                                   " and " + std::to_string(maximum_count) +
                                   " argument(s) but received " + std::to_string(values.size()) +
                                   " in production: " + production_.debug_name);
        return false;
    }

    SemanticValue evaluate_call(const std::string& name, const std::vector<ActionExpr>& args) const {
        std::vector<SemanticValue> values;
        values.reserve(args.size());
        for (const auto& arg : args) {
            values.push_back(evaluate(arg));
        }

        const auto span = merge_rhs_spans();

        if (name == "make_program") {
            if (!expect_arity(name, values, 1)) return {};
            return owner_.factory().make_program(expect_decl_list(values[0]), span);
        }
        if (name == "append_decl") {
            if (!expect_arity(name, values, 2)) return {};
            auto list = expect_decl_list(values[0]);
            list.push_back(expect_decl(values[1]));
            return list;
        }
        if (name == "make_decl_list") {
            if (!expect_arity_between(name, values, 0, 1)) return {};
            DeclList list;
            if (!values.empty()) {
                list.push_back(expect_decl(values[0]));
            }
            return list;
        }
        if (name == "make_global_var_decl") {
            if (!expect_arity(name, values, 3)) return {};
            return DeclPtr(owner_.factory().make_var_decl(expect_type(values[0]),
                                                          expect_text(values[1]),
                                                          expect_expr(values[2]),
                                                          span));
        }
        if (name == "make_empty_initializer") {
            if (!expect_arity(name, values, 0)) return {};
            return SemanticValue {};
        }
        if (name == "make_function_decl") {
            if (!expect_arity(name, values, 4)) return {};
            return DeclPtr(owner_.factory().make_function(expect_type(values[0]),
                                                          expect_text(values[1]),
                                                          expect_param_list(values[2]),
                                                          expect_block_stmt(values[3]),
                                                          span));
        }
        if (name == "make_param_list") {
            if (!expect_arity_between(name, values, 0, 1)) return {};
            ParameterList list;
            if (!values.empty()) {
                list.push_back(expect_param(values[0]));
            }
            return list;
        }
        if (name == "append_param") {
            if (!expect_arity(name, values, 2)) return {};
            auto list = expect_param_list(values[0]);
            list.push_back(expect_param(values[1]));
            return list;
        }
        if (name == "make_parameter_decl") {
            if (!expect_arity(name, values, 2)) return {};
            return owner_.factory().make_parameter(expect_type(values[0]),
                                                   expect_text(values[1]),
                                                   span);
        }
        if (name == "make_block_stmt") {
            if (!expect_arity(name, values, 1)) return {};
            return StmtPtr(owner_.factory().make_block(expect_stmt_list(values[0]), span));
        }
        if (name == "make_stmt_list") {
            if (!expect_arity_between(name, values, 0, 1)) return {};
            StmtList list;
            if (!values.empty()) {
                list.push_back(expect_stmt(values[0]));
            }
            return list;
        }
        if (name == "append_stmt") {
            if (!expect_arity(name, values, 2)) return {};
            auto list = expect_stmt_list(values[0]);
            list.push_back(expect_stmt(values[1]));
            return list;
        }
        if (name == "make_local_decl_stmt") {
            if (!expect_arity(name, values, 3)) return {};
            auto decl = owner_.factory().make_var_decl(expect_type(values[0]),
                                                       expect_text(values[1]),
                                                       expect_expr(values[2]),
                                                       span);
            return StmtPtr(owner_.factory().make_decl_stmt(std::move(decl), span));
        }
        if (name == "make_expr_stmt") {
            if (!expect_arity(name, values, 1)) return {};
            return StmtPtr(owner_.factory().make_expr_stmt(expect_expr(values[0]), span));
        }
        if (name == "make_empty_expr_stmt") {
            if (!expect_arity(name, values, 0)) return {};
            return StmtPtr(owner_.factory().make_expr_stmt(nullptr, span));
        }
        if (name == "make_return_stmt") {
            if (!expect_arity(name, values, 1)) return {};
            return StmtPtr(owner_.factory().make_return(expect_expr(values[0]), span));
        }
        if (name == "make_if_stmt") {
            if (!expect_arity(name, values, 3)) return {};
            return StmtPtr(owner_.factory().make_if(expect_expr(values[0]),
                                                    expect_stmt(values[1]),
                                                    expect_stmt(values[2]),
                                                    span));
        }
        if (name == "make_while_stmt") {
            if (!expect_arity(name, values, 2)) return {};
            return StmtPtr(owner_.factory().make_while(expect_expr(values[0]),
                                                       expect_stmt(values[1]),
                                                       span));
        }
        if (name == "make_assign_expr") {
            if (!expect_arity(name, values, 2)) return {};
            return ExprPtr(owner_.factory().make_assign(expect_text(values[0]),
                                                        expect_expr(values[1]),
                                                        span));
        }
        if (name == "make_binary_expr") {
            if (!expect_arity(name, values, 3)) return {};
            const auto op_name = expect_text(values[0]);
            BinaryOperator op = BinaryOperator::Add;
            if (op_name == "Plus") op = BinaryOperator::Add;
            else if (op_name == "Minus") op = BinaryOperator::Subtract;
            else if (op_name == "Star") op = BinaryOperator::Multiply;
            else if (op_name == "Slash") op = BinaryOperator::Divide;
            else if (op_name == "Percent") op = BinaryOperator::Modulo;
            else if (op_name == "Less") op = BinaryOperator::Less;
            else if (op_name == "LessEqual") op = BinaryOperator::LessEqual;
            else if (op_name == "Greater") op = BinaryOperator::Greater;
            else if (op_name == "GreaterEqual") op = BinaryOperator::GreaterEqual;
            else if (op_name == "Equal") op = BinaryOperator::Equal;
            else if (op_name == "NotEqual") op = BinaryOperator::NotEqual;
            else if (op_name == "LogicalAnd") op = BinaryOperator::LogicalAnd;
            else if (op_name == "LogicalOr") op = BinaryOperator::LogicalOr;
            else owner_.diagnostics().error("unknown binary operator in semantic action: " + op_name);
            return ExprPtr(owner_.factory().make_binary(op,
                                                        expect_expr(values[1]),
                                                        expect_expr(values[2]),
                                                        span));
        }
        if (name == "make_unary_expr") {
            if (!expect_arity(name, values, 2)) return {};
            const auto op_name = expect_text(values[0]);
            UnaryOperator op = UnaryOperator::Negate;
            if (op_name == "Minus") op = UnaryOperator::Negate;
            else if (op_name == "LogicalNot") op = UnaryOperator::LogicalNot;
            else owner_.diagnostics().error("unknown unary operator in semantic action: " + op_name);
            return ExprPtr(owner_.factory().make_unary(op,
                                                       expect_expr(values[1]),
                                                       span));
        }
        if (name == "make_expr_list") {
            if (!expect_arity_between(name, values, 0, 1)) return {};
            ExprList list;
            if (!values.empty()) {
                list.push_back(expect_expr(values[0]));
            }
            return list;
        }
        if (name == "append_expr") {
            if (!expect_arity(name, values, 2)) return {};
            auto list = expect_expr_list(values[0]);
            list.push_back(expect_expr(values[1]));
            return list;
        }
        if (name == "make_identifier_expr") {
            if (!expect_arity(name, values, 1)) return {};
            return ExprPtr(owner_.factory().make_identifier(expect_text(values[0]), span));
        }
        if (name == "make_integer_literal_expr") {
            if (!expect_arity(name, values, 1)) return {};
            return ExprPtr(owner_.factory().make_integer_literal(integer_token_value(values[0]), span));
        }
        if (name == "make_float_literal_expr") {
            if (!expect_arity(name, values, 1)) return {};
            return ExprPtr(owner_.factory().make_float_literal(float_token_value(values[0]), span));
        }
        if (name == "make_call_expr") {
            if (!expect_arity(name, values, 2)) return {};
            return ExprPtr(owner_.factory().make_call(expect_text(values[0]),
                                                      expect_expr_list(values[1]),
                                                      span));
        }

        owner_.diagnostics().error("unsupported semantic action function `" + name +
                                   "` in production: " + production_.debug_name);
        return {};
    }
};

SemanticActionExecutor::SemanticActionExecutor(common::DiagnosticEngine& diagnostics)
    : diagnostics_(diagnostics) {
}

common::DiagnosticEngine& SemanticActionExecutor::diagnostics() const {
    return diagnostics_;
}

const AstFactory& SemanticActionExecutor::factory() const {
    return factory_;
}

SemanticValue SemanticActionExecutor::execute(const runtime::ProductionInfo& production,
                                              const std::vector<SemanticValue>& rhs_values) const {
    if (production.action_text.empty()) {
        if (rhs_values.empty()) {
            return {};
        }
        return rhs_values.back();
    }

    ActionParser parser(production.action_text);
    const auto action = parser.parse();
    if (!action.has_value()) {
        diagnostics_.error("failed to parse semantic action for production: " + production.debug_name);
        return {};
    }

    ActionEvaluator evaluator(*this, production, rhs_values);
    return evaluator.evaluate(*action);
}

}  // namespace compilerlab::semantic
