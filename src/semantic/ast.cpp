#include "semantic/ast.h"

#include <utility>

namespace compilerlab::semantic {

AstNode::AstNode(AstNodeKind kind, common::SourceSpan span)
    : kind(kind), span(std::move(span)) {
}

Decl::Decl(AstNodeKind kind, common::SourceSpan span)
    : AstNode(kind, std::move(span)) {
}

Stmt::Stmt(AstNodeKind kind, common::SourceSpan span)
    : AstNode(kind, std::move(span)) {
}

Expr::Expr(AstNodeKind kind, common::SourceSpan span)
    : AstNode(kind, std::move(span)) {
}

Program::Program(DeclList declarations, common::SourceSpan span)
    : AstNode(AstNodeKind::Program, std::move(span)), declarations(std::move(declarations)) {
}

ParameterDecl::ParameterDecl(TypeSpecifier type_specifier, std::string name, common::SourceSpan span)
    : AstNode(AstNodeKind::ParameterDecl, std::move(span)),
      type_specifier(type_specifier),
      name(std::move(name)) {
}

VarDecl::VarDecl(TypeSpecifier type_specifier, std::string name, ExprPtr initializer, common::SourceSpan span)
    : Decl(AstNodeKind::VarDecl, std::move(span)),
      type_specifier(type_specifier),
      name(std::move(name)),
      initializer(std::move(initializer)) {
}

FunctionDecl::FunctionDecl(TypeSpecifier return_type, std::string name, ParameterList parameters,
                           BlockStmtPtr body, common::SourceSpan span)
    : Decl(AstNodeKind::FunctionDecl, std::move(span)),
      return_type(return_type),
      name(std::move(name)),
      parameters(std::move(parameters)),
      body(std::move(body)) {
}

BlockStmt::BlockStmt(StmtList statements, common::SourceSpan span)
    : Stmt(AstNodeKind::BlockStmt, std::move(span)),
      statements(std::move(statements)) {
}

DeclStmt::DeclStmt(VarDeclPtr declaration, common::SourceSpan span)
    : Stmt(AstNodeKind::DeclStmt, std::move(span)),
      declaration(std::move(declaration)) {
}

ExprStmt::ExprStmt(ExprPtr expression, common::SourceSpan span)
    : Stmt(AstNodeKind::ExprStmt, std::move(span)),
      expression(std::move(expression)) {
}

IfStmt::IfStmt(ExprPtr condition, StmtPtr then_branch, StmtPtr else_branch, common::SourceSpan span)
    : Stmt(AstNodeKind::IfStmt, std::move(span)),
      condition(std::move(condition)),
      then_branch(std::move(then_branch)),
      else_branch(std::move(else_branch)) {
}

WhileStmt::WhileStmt(ExprPtr condition, StmtPtr body, common::SourceSpan span)
    : Stmt(AstNodeKind::WhileStmt, std::move(span)),
      condition(std::move(condition)),
      body(std::move(body)) {
}

ReturnStmt::ReturnStmt(ExprPtr value, common::SourceSpan span)
    : Stmt(AstNodeKind::ReturnStmt, std::move(span)),
      value(std::move(value)) {
}

AssignExpr::AssignExpr(std::string target_name, ExprPtr value, common::SourceSpan span)
    : Expr(AstNodeKind::AssignExpr, std::move(span)),
      target_name(std::move(target_name)),
      value(std::move(value)) {
}

BinaryExpr::BinaryExpr(BinaryOperator op, ExprPtr lhs, ExprPtr rhs, common::SourceSpan span)
    : Expr(AstNodeKind::BinaryExpr, std::move(span)),
      op(op),
      lhs(std::move(lhs)),
      rhs(std::move(rhs)) {
}

UnaryExpr::UnaryExpr(UnaryOperator op, ExprPtr operand, common::SourceSpan span)
    : Expr(AstNodeKind::UnaryExpr, std::move(span)),
      op(op),
      operand(std::move(operand)) {
}

CallExpr::CallExpr(std::string callee, ExprList arguments, common::SourceSpan span)
    : Expr(AstNodeKind::CallExpr, std::move(span)),
      callee(std::move(callee)),
      arguments(std::move(arguments)) {
}

IdentifierExpr::IdentifierExpr(std::string name, common::SourceSpan span)
    : Expr(AstNodeKind::IdentifierExpr, std::move(span)),
      name(std::move(name)) {
}

IntegerLiteralExpr::IntegerLiteralExpr(std::int64_t value, common::SourceSpan span)
    : Expr(AstNodeKind::IntegerLiteralExpr, std::move(span)),
      value(value) {
}

FloatLiteralExpr::FloatLiteralExpr(double value, common::SourceSpan span)
    : Expr(AstNodeKind::FloatLiteralExpr, std::move(span)),
      value(value) {
}

std::string to_string(TypeSpecifier specifier) {
    switch (specifier) {
        case TypeSpecifier::Int: return "int";
        case TypeSpecifier::Float: return "float";
        case TypeSpecifier::Void: return "void";
    }
    return "unknown-type-specifier";
}

std::string to_string(AstNodeKind kind) {
    switch (kind) {
        case AstNodeKind::Program: return "Program";
        case AstNodeKind::FunctionDecl: return "FunctionDecl";
        case AstNodeKind::VarDecl: return "VarDecl";
        case AstNodeKind::ParameterDecl: return "ParameterDecl";
        case AstNodeKind::BlockStmt: return "BlockStmt";
        case AstNodeKind::DeclStmt: return "DeclStmt";
        case AstNodeKind::ExprStmt: return "ExprStmt";
        case AstNodeKind::IfStmt: return "IfStmt";
        case AstNodeKind::WhileStmt: return "WhileStmt";
        case AstNodeKind::ReturnStmt: return "ReturnStmt";
        case AstNodeKind::AssignExpr: return "AssignExpr";
        case AstNodeKind::BinaryExpr: return "BinaryExpr";
        case AstNodeKind::UnaryExpr: return "UnaryExpr";
        case AstNodeKind::CallExpr: return "CallExpr";
        case AstNodeKind::IdentifierExpr: return "IdentifierExpr";
        case AstNodeKind::IntegerLiteralExpr: return "IntegerLiteralExpr";
        case AstNodeKind::FloatLiteralExpr: return "FloatLiteralExpr";
    }
    return "UnknownAstNodeKind";
}

std::string to_string(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add: return "+";
        case BinaryOperator::Subtract: return "-";
        case BinaryOperator::Multiply: return "*";
        case BinaryOperator::Divide: return "/";
        case BinaryOperator::Modulo: return "%";
        case BinaryOperator::Less: return "<";
        case BinaryOperator::LessEqual: return "<=";
        case BinaryOperator::Greater: return ">";
        case BinaryOperator::GreaterEqual: return ">=";
        case BinaryOperator::Equal: return "==";
        case BinaryOperator::NotEqual: return "!=";
        case BinaryOperator::LogicalAnd: return "&&";
        case BinaryOperator::LogicalOr: return "||";
    }
    return "unknown-binary-op";
}

std::string to_string(UnaryOperator op) {
    switch (op) {
        case UnaryOperator::Negate: return "-";
        case UnaryOperator::LogicalNot: return "!";
    }
    return "unknown-unary-op";
}

}  // namespace compilerlab::semantic
