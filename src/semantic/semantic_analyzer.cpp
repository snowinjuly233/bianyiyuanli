#include "semantic/semantic_analyzer.h"

#include <algorithm>
#include <string_view>

namespace compilerlab::semantic {

namespace {

bool is_numeric(TypeKind kind) {
    return kind == TypeKind::Int || kind == TypeKind::Float;
}

bool is_scalar(TypeKind kind) {
    return is_numeric(kind);
}

bool types_compatible(TypeKind lhs, TypeKind rhs) {
    if (lhs == rhs) return true;
    if (lhs == TypeKind::Error || rhs == TypeKind::Error) return true;
    return false;
}

TypeKind promote_types(TypeKind lhs, TypeKind rhs) {
    if (lhs == TypeKind::Float || rhs == TypeKind::Float) return TypeKind::Float;
    if (lhs == TypeKind::Int && rhs == TypeKind::Int) return TypeKind::Int;
    return TypeKind::Error;
}

}  // namespace

SemanticAnalyzer::SemanticAnalyzer(common::DiagnosticEngine& diagnostics)
    : diagnostics_(diagnostics) {
}

bool SemanticAnalyzer::analyze(const ProgramPtr& program) {
    symbols_.reset();
    current_function_return_kind_ = TypeKind::Void;
    in_function_ = false;

    if (!program) {
        diagnostics_.error("semantic analysis received a null program");
        return false;
    }

    bool success = true;

    symbols_.enter_scope(ScopeKind::Global);

    for (const auto& decl : program->declarations) {
        if (!analyze_decl(decl.get())) {
            success = false;
        }
    }

    symbols_.leave_scope();

    return success && !diagnostics_.has_error();
}

bool SemanticAnalyzer::analyze_decl(const Decl* decl) {
    if (!decl) return true;

    switch (decl->kind) {
        case AstNodeKind::FunctionDecl:
            return analyze_function_decl(static_cast<const FunctionDecl*>(decl));
        case AstNodeKind::VarDecl:
            return analyze_var_decl(static_cast<const VarDecl*>(decl));
        default:
            return true;
    }
}

bool SemanticAnalyzer::analyze_function_decl(const FunctionDecl* func) {
    if (!func) return true;

    if (!check_local_redefinition("function", func->name, func->span)) {
        diagnostics_.error("redefinition of function: " + func->name, func->span);
        return false;
    }

    TypePtr func_type = std::make_shared<FunctionType>(
        make_builtin_type(type_kind_from_specifier(func->return_type)),
        collect_param_types(func->parameters));

    Symbol sym;
    sym.name = func->name;
    sym.type = func_type;
    sym.span = func->span;
    symbols_.insert(std::move(sym));

    const auto saved_return_kind = current_function_return_kind_;
    const auto saved_in_function = in_function_;
    current_function_return_kind_ = type_kind_from_specifier(func->return_type);
    in_function_ = true;

    symbols_.enter_scope(ScopeKind::Function);

    bool success = true;
    for (const auto& param : func->parameters) {
        if (!analyze_parameter_decl(param.get())) {
            success = false;
        }
    }

    if (func->body) {
        if (!analyze_stmt(func->body.get())) {
            success = false;
        }
    }

    symbols_.leave_scope();
    current_function_return_kind_ = saved_return_kind;
    in_function_ = saved_in_function;
    return success;
}

bool SemanticAnalyzer::analyze_var_decl(const VarDecl* var) {
    if (!var) return true;

    if (!check_local_redefinition("variable", var->name, var->span)) {
        diagnostics_.error("redefinition of variable: " + var->name, var->span);
        return false;
    }

    TypeKind kind = type_kind_from_specifier(var->type_specifier);
    Symbol sym;
    sym.name = var->name;
    sym.type = make_builtin_type(kind);
    sym.span = var->span;
    symbols_.insert(std::move(sym));

    if (var->initializer) {
        TypePtr init_type = analyze_expr(var->initializer.get());
        if (init_type && !types_compatible(kind, init_type->kind)) {
            diagnostics_.error("initialization type mismatch for variable: " + var->name, var->span);
            return false;
        }
    }

    return true;
}

bool SemanticAnalyzer::analyze_parameter_decl(const ParameterDecl* param) {
    if (!param) return true;

    if (!check_local_redefinition("parameter", param->name, param->span)) {
        diagnostics_.error("redefinition of parameter: " + param->name, param->span);
        return false;
    }

    TypeKind kind = type_kind_from_specifier(param->type_specifier);
    Symbol sym;
    sym.name = param->name;
    sym.type = make_builtin_type(kind);
    sym.span = param->span;
    symbols_.insert(std::move(sym));

    return true;
}

bool SemanticAnalyzer::analyze_stmt(const Stmt* stmt) {
    if (!stmt) return true;

    switch (stmt->kind) {
        case AstNodeKind::BlockStmt:
            return analyze_block_stmt(static_cast<const BlockStmt*>(stmt));
        case AstNodeKind::DeclStmt:
            return analyze_decl_stmt(static_cast<const DeclStmt*>(stmt));
        case AstNodeKind::ExprStmt:
            return analyze_expr_stmt(static_cast<const ExprStmt*>(stmt));
        case AstNodeKind::IfStmt:
            return analyze_if_stmt(static_cast<const IfStmt*>(stmt));
        case AstNodeKind::WhileStmt:
            return analyze_while_stmt(static_cast<const WhileStmt*>(stmt));
        case AstNodeKind::ReturnStmt:
            return analyze_return_stmt(static_cast<const ReturnStmt*>(stmt));
        default:
            return true;
    }
}

bool SemanticAnalyzer::analyze_block_stmt(const BlockStmt* block) {
    if (!block) return true;

    symbols_.enter_scope(ScopeKind::Block);

    bool success = true;
    for (const auto& stmt : block->statements) {
        if (!analyze_stmt(stmt.get())) {
            success = false;
        }
    }

    symbols_.leave_scope();
    return success;
}

bool SemanticAnalyzer::analyze_decl_stmt(const DeclStmt* decl_stmt) {
    if (!decl_stmt) return true;
    return analyze_decl(decl_stmt->declaration.get());
}

bool SemanticAnalyzer::analyze_expr_stmt(const ExprStmt* expr_stmt) {
    if (!expr_stmt) return true;
    if (expr_stmt->expression) {
        analyze_expr(expr_stmt->expression.get());
    }
    return true;
}

bool SemanticAnalyzer::analyze_if_stmt(const IfStmt* if_stmt) {
    if (!if_stmt) return true;

    bool success = true;
    if (if_stmt->condition) {
        TypePtr cond_type = analyze_expr(if_stmt->condition.get());
        if (cond_type && !is_scalar(cond_type->kind)) {
            diagnostics_.error("if condition must be scalar type", if_stmt->span);
            success = false;
        }
    }

    if (if_stmt->then_branch) {
        if (!analyze_stmt(if_stmt->then_branch.get())) {
            success = false;
        }
    }
    if (if_stmt->else_branch) {
        if (!analyze_stmt(if_stmt->else_branch.get())) {
            success = false;
        }
    }

    return success;
}

bool SemanticAnalyzer::analyze_while_stmt(const WhileStmt* while_stmt) {
    if (!while_stmt) return true;

    bool success = true;
    if (while_stmt->condition) {
        TypePtr cond_type = analyze_expr(while_stmt->condition.get());
        if (cond_type && !is_scalar(cond_type->kind)) {
            diagnostics_.error("while condition must be scalar type", while_stmt->span);
            success = false;
        }
    }

    if (while_stmt->body) {
        if (!analyze_stmt(while_stmt->body.get())) {
            success = false;
        }
    }

    return success;
}

bool SemanticAnalyzer::analyze_return_stmt(const ReturnStmt* return_stmt) {
    if (!return_stmt) return true;

    if (!in_function_) {
        diagnostics_.error("return statement is only allowed inside a function", return_stmt->span);
        return false;
    }

    if (current_function_return_kind_ == TypeKind::Void) {
        if (return_stmt->value) {
            analyze_expr(return_stmt->value.get());
            diagnostics_.error("void function must not return a value", return_stmt->span);
            return false;
        }
        return true;
    }

    if (!return_stmt->value) {
        diagnostics_.error("non-void function must return a value", return_stmt->span);
        return false;
    }

    if (return_stmt->value) {
        TypePtr value_type = analyze_expr(return_stmt->value.get());
        if (value_type && !types_compatible(current_function_return_kind_, value_type->kind)) {
            diagnostics_.error("return type mismatch: expected " +
                                   to_string(current_function_return_kind_) +
                                   " but got " + to_string(value_type->kind),
                               return_stmt->span);
            return false;
        }
    }

    return true;
}

TypePtr SemanticAnalyzer::analyze_expr(const Expr* expr) {
    if (!expr) return nullptr;

    switch (expr->kind) {
        case AstNodeKind::IntegerLiteralExpr:
            return make_builtin_type(TypeKind::Int);
        case AstNodeKind::FloatLiteralExpr:
            return make_builtin_type(TypeKind::Float);
        case AstNodeKind::IdentifierExpr:
            return analyze_identifier_expr(static_cast<const IdentifierExpr*>(expr));
        case AstNodeKind::BinaryExpr:
            return analyze_binary_expr(static_cast<const BinaryExpr*>(expr));
        case AstNodeKind::UnaryExpr:
            return analyze_unary_expr(static_cast<const UnaryExpr*>(expr));
        case AstNodeKind::AssignExpr:
            return analyze_assign_expr(static_cast<const AssignExpr*>(expr));
        case AstNodeKind::CallExpr:
            return analyze_call_expr(static_cast<const CallExpr*>(expr));
        default:
            return nullptr;
    }
}

TypePtr SemanticAnalyzer::analyze_identifier_expr(const IdentifierExpr* ident) {
    if (!ident) return nullptr;

    const Symbol* sym = symbols_.lookup(ident->name);
    if (!sym) {
        diagnostics_.error("use of undeclared identifier: " + ident->name, ident->span);
        return make_builtin_type(TypeKind::Error);
    }

    return sym->type;
}

TypePtr SemanticAnalyzer::analyze_binary_expr(const BinaryExpr* bin_expr) {
    if (!bin_expr) return nullptr;

    TypePtr lhs_type = analyze_expr(bin_expr->lhs.get());
    TypePtr rhs_type = analyze_expr(bin_expr->rhs.get());

    if (!lhs_type || !rhs_type) return nullptr;

    if (lhs_type->kind == TypeKind::Error || rhs_type->kind == TypeKind::Error) {
        return make_builtin_type(TypeKind::Error);
    }

    switch (bin_expr->op) {
        case BinaryOperator::Add:
        case BinaryOperator::Subtract:
        case BinaryOperator::Multiply:
        case BinaryOperator::Divide:
        case BinaryOperator::Modulo: {
            if (!is_numeric(lhs_type->kind) || !is_numeric(rhs_type->kind)) {
                diagnostics_.error("binary operator requires numeric operands", bin_expr->span);
                return make_builtin_type(TypeKind::Error);
            }
            return make_builtin_type(promote_types(lhs_type->kind, rhs_type->kind));
        }
        case BinaryOperator::Less:
        case BinaryOperator::LessEqual:
        case BinaryOperator::Greater:
        case BinaryOperator::GreaterEqual:
        case BinaryOperator::Equal:
        case BinaryOperator::NotEqual: {
            if (!is_numeric(lhs_type->kind) || !is_numeric(rhs_type->kind)) {
                diagnostics_.error("comparison requires numeric operands", bin_expr->span);
                return make_builtin_type(TypeKind::Error);
            }
            return make_builtin_type(TypeKind::Int);
        }
        case BinaryOperator::LogicalAnd:
        case BinaryOperator::LogicalOr: {
            if (!is_scalar(lhs_type->kind) || !is_scalar(rhs_type->kind)) {
                diagnostics_.error("logical operator requires scalar operands", bin_expr->span);
                return make_builtin_type(TypeKind::Error);
            }
            return make_builtin_type(TypeKind::Int);
        }
    }

    return nullptr;
}

TypePtr SemanticAnalyzer::analyze_unary_expr(const UnaryExpr* unary_expr) {
    if (!unary_expr) return nullptr;

    TypePtr operand_type = analyze_expr(unary_expr->operand.get());

    if (!operand_type) return nullptr;

    if (operand_type->kind == TypeKind::Error) {
        return make_builtin_type(TypeKind::Error);
    }

    switch (unary_expr->op) {
        case UnaryOperator::Negate: {
            if (!is_numeric(operand_type->kind)) {
                diagnostics_.error("unary minus requires numeric operand", unary_expr->span);
                return make_builtin_type(TypeKind::Error);
            }
            return operand_type;
        }
        case UnaryOperator::LogicalNot: {
            if (!is_scalar(operand_type->kind)) {
                diagnostics_.error("logical not requires scalar operand", unary_expr->span);
                return make_builtin_type(TypeKind::Error);
            }
            return make_builtin_type(TypeKind::Int);
        }
    }

    return nullptr;
}

TypePtr SemanticAnalyzer::analyze_assign_expr(const AssignExpr* assign_expr) {
    if (!assign_expr) return nullptr;

    const Symbol* sym = symbols_.lookup(assign_expr->target_name);
    if (!sym) {
        diagnostics_.error("assignment to undeclared identifier: " + assign_expr->target_name,
                          assign_expr->span);
        return make_builtin_type(TypeKind::Error);
    }

    TypePtr value_type = analyze_expr(assign_expr->value.get());

    if (!value_type) return nullptr;

    if (value_type->kind == TypeKind::Error) {
        return make_builtin_type(TypeKind::Error);
    }

    if (!types_compatible(sym->type->kind, value_type->kind)) {
        diagnostics_.error("assignment type mismatch", assign_expr->span);
        return make_builtin_type(TypeKind::Error);
    }

    return sym->type;
}

TypePtr SemanticAnalyzer::analyze_call_expr(const CallExpr* call_expr) {
    if (!call_expr) return nullptr;

    const Symbol* sym = symbols_.lookup(call_expr->callee);
    if (!sym) {
        diagnostics_.error("call to undeclared function: " + call_expr->callee, call_expr->span);
        for (const auto& arg : call_expr->arguments) {
            analyze_expr(arg.get());
        }
        return make_builtin_type(TypeKind::Error);
    }

    const auto* func_type = dynamic_cast<const FunctionType*>(sym->type.get());
    if (!func_type) {
        diagnostics_.error("call of non-function: " + call_expr->callee, call_expr->span);
        return make_builtin_type(TypeKind::Error);
    }

    if (call_expr->arguments.size() != func_type->parameter_types.size()) {
        diagnostics_.error("function call argument count mismatch", call_expr->span);
    }

    for (std::size_t i = 0; i < call_expr->arguments.size(); ++i) {
        TypePtr arg_type = analyze_expr(call_expr->arguments[i].get());
        if (i < func_type->parameter_types.size() && arg_type && func_type->parameter_types[i]) {
            if (!types_compatible(func_type->parameter_types[i]->kind, arg_type->kind)) {
                diagnostics_.error("function argument type mismatch", call_expr->span);
            }
        }
    }

    return func_type->return_type;
}

std::vector<TypePtr> SemanticAnalyzer::collect_param_types(const ParameterList& params) {
    std::vector<TypePtr> types;
    types.reserve(params.size());
    for (const auto& param : params) {
        types.push_back(make_builtin_type(type_kind_from_specifier(param->type_specifier)));
    }
    return types;
}

SymbolTable& SemanticAnalyzer::symbols() {
    return symbols_;
}

const SymbolTable& SemanticAnalyzer::symbols() const {
    return symbols_;
}

bool SemanticAnalyzer::check_local_redefinition(std::string_view,
                                                const std::string& name,
                                                const common::SourceSpan&) {
    return symbols_.lookup_current_scope(name) == nullptr;
}

}  // namespace compilerlab::semantic
