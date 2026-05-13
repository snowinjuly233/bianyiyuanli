#pragma once

#include <string_view>

#include "common/diagnostic.h"
#include "semantic/ast.h"
#include "semantic/symbol_table.h"

namespace compilerlab::semantic {

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(common::DiagnosticEngine& diagnostics);

    bool analyze(const ProgramPtr& program);

    SymbolTable& symbols();
    const SymbolTable& symbols() const;

private:
    bool analyze_decl(const Decl* decl);
    bool analyze_function_decl(const FunctionDecl* func);
    bool analyze_var_decl(const VarDecl* var);
    bool analyze_parameter_decl(const ParameterDecl* param);
    bool analyze_stmt(const Stmt* stmt);
    bool analyze_block_stmt(const BlockStmt* block);
    bool analyze_decl_stmt(const DeclStmt* decl_stmt);
    bool analyze_expr_stmt(const ExprStmt* expr_stmt);
    bool analyze_if_stmt(const IfStmt* if_stmt);
    bool analyze_while_stmt(const WhileStmt* while_stmt);
    bool analyze_return_stmt(const ReturnStmt* return_stmt);
    TypePtr analyze_expr(const Expr* expr);
    TypePtr analyze_identifier_expr(const IdentifierExpr* ident);
    TypePtr analyze_binary_expr(const BinaryExpr* bin_expr);
    TypePtr analyze_unary_expr(const UnaryExpr* unary_expr);
    TypePtr analyze_assign_expr(const AssignExpr* assign_expr);
    TypePtr analyze_call_expr(const CallExpr* call_expr);
    std::vector<TypePtr> collect_param_types(const ParameterList& params);
    bool check_local_redefinition(std::string_view symbol_kind, const std::string& name,
                                  const common::SourceSpan& span);

    common::DiagnosticEngine& diagnostics_;
    SymbolTable symbols_;
    TypeKind current_function_return_kind_ {TypeKind::Void};
    bool in_function_ {false};
};

}  // namespace compilerlab::semantic
