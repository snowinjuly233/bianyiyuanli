#pragma once

#include "semantic/ast.h"

namespace compilerlab::semantic {

class AstFactory {
public:
    ProgramPtr make_program(DeclList declarations, common::SourceSpan span = {}) const;
    FunctionDeclPtr make_function(TypeSpecifier return_type, std::string name, ParameterList parameters,
                                  BlockStmtPtr body, common::SourceSpan span = {}) const;
    VarDeclPtr make_var_decl(TypeSpecifier type_specifier, std::string name, ExprPtr initializer = nullptr,
                             common::SourceSpan span = {}) const;
    ParameterDeclPtr make_parameter(TypeSpecifier type_specifier, std::string name,
                                    common::SourceSpan span = {}) const;
    BlockStmtPtr make_block(StmtList statements, common::SourceSpan span = {}) const;
    DeclStmtPtr make_decl_stmt(VarDeclPtr declaration, common::SourceSpan span = {}) const;
    ExprStmtPtr make_expr_stmt(ExprPtr expression, common::SourceSpan span = {}) const;
    IfStmtPtr make_if(ExprPtr condition, StmtPtr then_branch, StmtPtr else_branch = nullptr,
                      common::SourceSpan span = {}) const;
    WhileStmtPtr make_while(ExprPtr condition, StmtPtr body, common::SourceSpan span = {}) const;
    ReturnStmtPtr make_return(ExprPtr value = nullptr, common::SourceSpan span = {}) const;
    AssignExprPtr make_assign(std::string target_name, ExprPtr value, common::SourceSpan span = {}) const;
    BinaryExprPtr make_binary(BinaryOperator op, ExprPtr lhs, ExprPtr rhs, common::SourceSpan span = {}) const;
    UnaryExprPtr make_unary(UnaryOperator op, ExprPtr operand, common::SourceSpan span = {}) const;
    CallExprPtr make_call(std::string callee, ExprList arguments, common::SourceSpan span = {}) const;
    IdentifierExprPtr make_identifier(std::string name, common::SourceSpan span = {}) const;
    IntegerLiteralExprPtr make_integer_literal(std::int64_t value, common::SourceSpan span = {}) const;
    FloatLiteralExprPtr make_float_literal(double value, common::SourceSpan span = {}) const;

    common::SourceSpan merge(const common::SourceSpan& lhs, const common::SourceSpan& rhs) const;
};

}  // namespace compilerlab::semantic
