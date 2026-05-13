#include "semantic/ast_factory.h"

#include <utility>

namespace compilerlab::semantic {

ProgramPtr AstFactory::make_program(DeclList declarations, common::SourceSpan span) const {
    return std::make_shared<Program>(std::move(declarations), std::move(span));
}

FunctionDeclPtr AstFactory::make_function(TypeSpecifier return_type, std::string name,
                                          ParameterList parameters, BlockStmtPtr body,
                                          common::SourceSpan span) const {
    return std::make_shared<FunctionDecl>(return_type, std::move(name), std::move(parameters),
                                          std::move(body), std::move(span));
}

VarDeclPtr AstFactory::make_var_decl(TypeSpecifier type_specifier, std::string name,
                                     ExprPtr initializer, common::SourceSpan span) const {
    return std::make_shared<VarDecl>(type_specifier, std::move(name), std::move(initializer),
                                     std::move(span));
}

ParameterDeclPtr AstFactory::make_parameter(TypeSpecifier type_specifier, std::string name,
                                            common::SourceSpan span) const {
    return std::make_shared<ParameterDecl>(type_specifier, std::move(name), std::move(span));
}

BlockStmtPtr AstFactory::make_block(StmtList statements, common::SourceSpan span) const {
    return std::make_shared<BlockStmt>(std::move(statements), std::move(span));
}

DeclStmtPtr AstFactory::make_decl_stmt(VarDeclPtr declaration, common::SourceSpan span) const {
    return std::make_shared<DeclStmt>(std::move(declaration), std::move(span));
}

ExprStmtPtr AstFactory::make_expr_stmt(ExprPtr expression, common::SourceSpan span) const {
    return std::make_shared<ExprStmt>(std::move(expression), std::move(span));
}

IfStmtPtr AstFactory::make_if(ExprPtr condition, StmtPtr then_branch, StmtPtr else_branch,
                              common::SourceSpan span) const {
    return std::make_shared<IfStmt>(std::move(condition), std::move(then_branch),
                                    std::move(else_branch), std::move(span));
}

WhileStmtPtr AstFactory::make_while(ExprPtr condition, StmtPtr body, common::SourceSpan span) const {
    return std::make_shared<WhileStmt>(std::move(condition), std::move(body), std::move(span));
}

ReturnStmtPtr AstFactory::make_return(ExprPtr value, common::SourceSpan span) const {
    return std::make_shared<ReturnStmt>(std::move(value), std::move(span));
}

AssignExprPtr AstFactory::make_assign(std::string target_name, ExprPtr value, common::SourceSpan span) const {
    return std::make_shared<AssignExpr>(std::move(target_name), std::move(value), std::move(span));
}

BinaryExprPtr AstFactory::make_binary(BinaryOperator op, ExprPtr lhs, ExprPtr rhs,
                                      common::SourceSpan span) const {
    return std::make_shared<BinaryExpr>(op, std::move(lhs), std::move(rhs), std::move(span));
}

UnaryExprPtr AstFactory::make_unary(UnaryOperator op, ExprPtr operand, common::SourceSpan span) const {
    return std::make_shared<UnaryExpr>(op, std::move(operand), std::move(span));
}

CallExprPtr AstFactory::make_call(std::string callee, ExprList arguments, common::SourceSpan span) const {
    return std::make_shared<CallExpr>(std::move(callee), std::move(arguments), std::move(span));
}

IdentifierExprPtr AstFactory::make_identifier(std::string name, common::SourceSpan span) const {
    return std::make_shared<IdentifierExpr>(std::move(name), std::move(span));
}

IntegerLiteralExprPtr AstFactory::make_integer_literal(std::int64_t value, common::SourceSpan span) const {
    return std::make_shared<IntegerLiteralExpr>(value, std::move(span));
}

FloatLiteralExprPtr AstFactory::make_float_literal(double value, common::SourceSpan span) const {
    return std::make_shared<FloatLiteralExpr>(value, std::move(span));
}

common::SourceSpan AstFactory::merge(const common::SourceSpan& lhs, const common::SourceSpan& rhs) const {
    if (!lhs.is_valid()) {
        return rhs;
    }
    if (!rhs.is_valid()) {
        return lhs;
    }

    common::SourceSpan merged;
    merged.file_path = lhs.file_path.empty() ? rhs.file_path : lhs.file_path;
    merged.begin = lhs.begin;
    merged.end = rhs.end;
    return merged;
}

}  // namespace compilerlab::semantic
