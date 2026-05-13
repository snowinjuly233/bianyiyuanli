#include "ir/ir_builder.h"

#include <cctype>
#include <unordered_map>

namespace compilerlab::ir {

namespace {

int temp_counter = 0;
int label_counter = 0;

std::string make_temp() {
    return "t" + std::to_string(temp_counter++);
}

std::string make_label() {
    return "L" + std::to_string(label_counter++);
}

void reset_counters() {
    temp_counter = 0;
    label_counter = 0;
}

}  // namespace

class IRBuilderImpl {
public:
    IRBuilderImpl(std::vector<Quad>& quads, common::DiagnosticEngine& diagnostics)
        : quads_(quads), diagnostics_(diagnostics) {}

    void build(const semantic::ProgramPtr& program) {
        if (!program) {
            diagnostics_.error("IR build received a null program");
            return;
        }

        reset_counters();

        for (const auto& decl : program->declarations) {
            build_decl(decl.get());
        }
    }

private:
    std::vector<Quad>& quads_;
    common::DiagnosticEngine& diagnostics_;
    std::unordered_map<const semantic::Expr*, std::string> expr_results_;

    static bool is_temp_name(const std::string& text) {
        if (text.size() < 2 || text.front() != 't') {
            return false;
        }
        for (std::size_t index = 1; index < text.size(); ++index) {
            if (!std::isdigit(static_cast<unsigned char>(text[index]))) {
                return false;
            }
        }
        return true;
    }

    static Operand make_label_operand(const std::string& text) {
        Operand op;
        op.kind = Operand::Kind::Label;
        op.text = text;
        return op;
    }

    static Operand make_reference_operand(const std::string& text) {
        Operand op;
        op.kind = is_temp_name(text) ? Operand::Kind::Temporary : Operand::Kind::Identifier;
        op.text = text;
        return op;
    }

    static Operand make_integer_operand(std::int64_t value) {
        Operand op;
        op.kind = Operand::Kind::IntegerLiteral;
        op.text = std::to_string(value);
        return op;
    }

    static Operand make_float_operand(double value) {
        Operand op;
        op.kind = Operand::Kind::FloatLiteral;
        op.text = std::to_string(value);
        return op;
    }

    static Operand make_temp_operand(const std::string& text) {
        Operand op;
        op.kind = Operand::Kind::Temporary;
        op.text = text;
        return op;
    }

    void emit(OpCode op, const Operand& arg1, const Operand& arg2, const Operand& result) {
        Quad q;
        q.op = op;
        q.arg1 = arg1;
        q.arg2 = arg2;
        q.result = result;
        quads_.push_back(q);
    }

    void emit_label(const std::string& label) {
        emit(OpCode::Label, {}, {}, make_label_operand(label));
    }

    void set_expr_result(const semantic::Expr* expr, const std::string& result) {
        expr_results_[expr] = result;
    }

    void build_decl(const semantic::Decl* decl) {
        if (!decl) return;

        switch (decl->kind) {
            case semantic::AstNodeKind::FunctionDecl:
                build_function_decl(static_cast<const semantic::FunctionDecl*>(decl));
                break;
            case semantic::AstNodeKind::VarDecl:
                build_var_decl(static_cast<const semantic::VarDecl*>(decl));
                break;
            default:
                break;
        }
    }

    void build_function_decl(const semantic::FunctionDecl* func) {
        if (!func) return;

        emit_label(func->name + "_entry");

        bool falls_through = true;
        if (func->body) {
            falls_through = build_stmt(func->body.get());
        }

        if (falls_through) {
            emit(OpCode::Return, {}, {}, {});
        }
    }

    void build_var_decl(const semantic::VarDecl* var) {
        if (!var) return;

        if (var->initializer) {
            std::string result = build_expr(var->initializer.get());
            emit(OpCode::Assign, make_reference_operand(result), {}, make_reference_operand(var->name));
        }
    }

    bool build_stmt(const semantic::Stmt* stmt) {
        if (!stmt) return true;

        switch (stmt->kind) {
            case semantic::AstNodeKind::BlockStmt:
                return build_block_stmt(static_cast<const semantic::BlockStmt*>(stmt));
            case semantic::AstNodeKind::DeclStmt:
                return build_decl_stmt(static_cast<const semantic::DeclStmt*>(stmt));
            case semantic::AstNodeKind::ExprStmt:
                return build_expr_stmt(static_cast<const semantic::ExprStmt*>(stmt));
            case semantic::AstNodeKind::IfStmt:
                return build_if_stmt(static_cast<const semantic::IfStmt*>(stmt));
            case semantic::AstNodeKind::WhileStmt:
                return build_while_stmt(static_cast<const semantic::WhileStmt*>(stmt));
            case semantic::AstNodeKind::ReturnStmt:
                return build_return_stmt(static_cast<const semantic::ReturnStmt*>(stmt));
            default:
                return true;
        }
    }

    bool build_block_stmt(const semantic::BlockStmt* block) {
        if (!block) return true;

        for (const auto& stmt : block->statements) {
            if (!build_stmt(stmt.get())) {
                return false;
            }
        }
        return true;
    }

    bool build_decl_stmt(const semantic::DeclStmt* decl_stmt) {
        if (!decl_stmt) return true;
        build_decl(decl_stmt->declaration.get());
        return true;
    }

    bool build_expr_stmt(const semantic::ExprStmt* expr_stmt) {
        if (!expr_stmt) return true;
        if (expr_stmt->expression) {
            build_expr(expr_stmt->expression.get());
        }
        return true;
    }

    bool build_if_stmt(const semantic::IfStmt* if_stmt) {
        if (!if_stmt) return true;

        std::string else_label = make_label();
        std::string end_label = make_label();

        if (if_stmt->condition) {
            std::string cond_result = build_expr(if_stmt->condition.get());
            const auto false_target = if_stmt->else_branch ? else_label : end_label;
            emit(OpCode::JumpIfFalse,
                 make_reference_operand(cond_result),
                 {},
                 make_label_operand(false_target));
        }

        bool then_falls_through = true;
        if (if_stmt->then_branch) {
            then_falls_through = build_stmt(if_stmt->then_branch.get());
        }

        if (if_stmt->else_branch) {
            if (then_falls_through) {
                emit(OpCode::Jump, {}, {}, make_label_operand(end_label));
            }
            emit_label(else_label);
            const bool else_falls_through = build_stmt(if_stmt->else_branch.get());
            emit_label(end_label);
            return then_falls_through || else_falls_through;
        }

        emit_label(end_label);
        return true;
    }

    bool build_while_stmt(const semantic::WhileStmt* while_stmt) {
        if (!while_stmt) return true;

        std::string loop_label = make_label();
        std::string end_label = make_label();

        emit_label(loop_label);

        if (while_stmt->condition) {
            std::string cond_result = build_expr(while_stmt->condition.get());
            emit(OpCode::JumpIfFalse,
                 make_reference_operand(cond_result),
                 {},
                 make_label_operand(end_label));
        }

        if (while_stmt->body) {
            build_stmt(while_stmt->body.get());
        }

        emit(OpCode::Jump, {}, {}, make_label_operand(loop_label));
        emit_label(end_label);
        return true;
    }

    bool build_return_stmt(const semantic::ReturnStmt* return_stmt) {
        if (!return_stmt) return false;

        if (return_stmt->value) {
            std::string result = build_expr(return_stmt->value.get());
            emit(OpCode::Return, make_reference_operand(result), {}, {});
        } else {
            emit(OpCode::Return, {}, {}, {});
        }
        return false;
    }

    std::string build_expr(const semantic::Expr* expr) {
        if (!expr) return make_temp();

        switch (expr->kind) {
            case semantic::AstNodeKind::IntegerLiteralExpr: {
                const auto* lit = static_cast<const semantic::IntegerLiteralExpr*>(expr);
                std::string temp = make_temp();
                emit(OpCode::Assign, make_integer_operand(lit->value), {}, make_temp_operand(temp));
                set_expr_result(expr, temp);
                return temp;
            }
            case semantic::AstNodeKind::FloatLiteralExpr: {
                const auto* lit = static_cast<const semantic::FloatLiteralExpr*>(expr);
                std::string temp = make_temp();
                emit(OpCode::Assign, make_float_operand(lit->value), {}, make_temp_operand(temp));
                set_expr_result(expr, temp);
                return temp;
            }
            case semantic::AstNodeKind::IdentifierExpr: {
                const auto* ident = static_cast<const semantic::IdentifierExpr*>(expr);
                set_expr_result(expr, ident->name);
                return ident->name;
            }
            case semantic::AstNodeKind::BinaryExpr:
                return build_binary_expr(static_cast<const semantic::BinaryExpr*>(expr));
            case semantic::AstNodeKind::UnaryExpr:
                return build_unary_expr(static_cast<const semantic::UnaryExpr*>(expr));
            case semantic::AstNodeKind::AssignExpr:
                return build_assign_expr(static_cast<const semantic::AssignExpr*>(expr));
            case semantic::AstNodeKind::CallExpr:
                return build_call_expr(static_cast<const semantic::CallExpr*>(expr));
            default:
                return make_temp();
        }
    }

    std::string build_binary_expr(const semantic::BinaryExpr* bin_expr) {
        if (!bin_expr) return make_temp();

        std::string lhs_result = build_expr(bin_expr->lhs.get());
        std::string rhs_result = build_expr(bin_expr->rhs.get());

        std::string temp = make_temp();
        OpCode op = translate_binary_op(bin_expr->op);
        emit(op,
             make_reference_operand(lhs_result),
             make_reference_operand(rhs_result),
             make_temp_operand(temp));

        set_expr_result(bin_expr, temp);
        return temp;
    }

    std::string build_unary_expr(const semantic::UnaryExpr* unary_expr) {
        if (!unary_expr) return make_temp();

        if (unary_expr->op == semantic::UnaryOperator::Negate) {
            std::string operand_result = build_expr(unary_expr->operand.get());

            std::string zero_temp = make_temp();
            emit(OpCode::Assign, make_integer_operand(0), {}, make_temp_operand(zero_temp));

            std::string temp = make_temp();
            emit(OpCode::Subtract,
                 make_reference_operand(zero_temp),
                 make_reference_operand(operand_result),
                 make_temp_operand(temp));

            set_expr_result(unary_expr, temp);
            return temp;
        }

        if (unary_expr->op == semantic::UnaryOperator::LogicalNot) {
            std::string operand_result = build_expr(unary_expr->operand.get());

            std::string temp = make_temp();
            emit(OpCode::LogicalNot,
                 make_reference_operand(operand_result),
                 {},
                 make_temp_operand(temp));

            set_expr_result(unary_expr, temp);
            return temp;
        }

        return make_temp();
    }

    std::string build_assign_expr(const semantic::AssignExpr* assign_expr) {
        if (!assign_expr) return make_temp();

        std::string value_result = build_expr(assign_expr->value.get());
        emit(OpCode::Assign,
             make_reference_operand(value_result),
             {},
             make_reference_operand(assign_expr->target_name));

        set_expr_result(assign_expr, assign_expr->target_name);
        return assign_expr->target_name;
    }

    std::string build_call_expr(const semantic::CallExpr* call_expr) {
        if (!call_expr) return make_temp();

        for (const auto& arg : call_expr->arguments) {
            std::string arg_result = build_expr(arg.get());
            emit(OpCode::Param, make_reference_operand(arg_result), {}, {});
        }

        std::string temp = make_temp();
        emit(OpCode::Call,
             make_reference_operand(call_expr->callee),
             make_integer_operand(static_cast<std::int64_t>(call_expr->arguments.size())),
             make_temp_operand(temp));

        set_expr_result(call_expr, temp);
        return temp;
    }

    OpCode translate_binary_op(semantic::BinaryOperator op) {
        switch (op) {
            case semantic::BinaryOperator::Add: return OpCode::Add;
            case semantic::BinaryOperator::Subtract: return OpCode::Subtract;
            case semantic::BinaryOperator::Multiply: return OpCode::Multiply;
            case semantic::BinaryOperator::Divide: return OpCode::Divide;
            case semantic::BinaryOperator::Modulo: return OpCode::Modulo;
            case semantic::BinaryOperator::Less: return OpCode::CompareLess;
            case semantic::BinaryOperator::LessEqual: return OpCode::CompareLessEqual;
            case semantic::BinaryOperator::Greater: return OpCode::CompareGreater;
            case semantic::BinaryOperator::GreaterEqual: return OpCode::CompareGreaterEqual;
            case semantic::BinaryOperator::Equal: return OpCode::CompareEqual;
            case semantic::BinaryOperator::NotEqual: return OpCode::CompareNotEqual;
            case semantic::BinaryOperator::LogicalAnd: return OpCode::LogicalAnd;
            case semantic::BinaryOperator::LogicalOr: return OpCode::LogicalOr;
        }
        return OpCode::Add;
    }

};

std::vector<Quad> IRBuilder::build(const semantic::ProgramPtr& program,
                                   common::DiagnosticEngine& diagnostics) const {
    std::vector<Quad> quads;

    if (!program) {
        diagnostics.error("IR build received a null program");
        return quads;
    }

    IRBuilderImpl impl(quads, diagnostics);
    impl.build(program);

    return quads;
}

}  // namespace compilerlab::ir
