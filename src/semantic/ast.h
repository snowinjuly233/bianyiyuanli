#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "common/source.h"

namespace compilerlab::semantic {

enum class TypeSpecifier {
    Int,
    Float,
    Void,
};

enum class AstNodeKind {
    Program,
    FunctionDecl,
    VarDecl,
    ParameterDecl,
    BlockStmt,
    DeclStmt,
    ExprStmt,
    IfStmt,
    WhileStmt,
    ReturnStmt,
    AssignExpr,
    BinaryExpr,
    UnaryExpr,
    CallExpr,
    IdentifierExpr,
    IntegerLiteralExpr,
    FloatLiteralExpr,
};

enum class BinaryOperator {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,
    LogicalAnd,
    LogicalOr,
};

enum class UnaryOperator {
    Negate,
    LogicalNot,
};

class AstNode;
class Decl;
class Stmt;
class Expr;
class Program;
class FunctionDecl;
class VarDecl;
class ParameterDecl;
class BlockStmt;
class DeclStmt;
class ExprStmt;
class IfStmt;
class WhileStmt;
class ReturnStmt;
class AssignExpr;
class BinaryExpr;
class UnaryExpr;
class CallExpr;
class IdentifierExpr;
class IntegerLiteralExpr;
class FloatLiteralExpr;

using AstNodePtr = std::shared_ptr<AstNode>;
using DeclPtr = std::shared_ptr<Decl>;
using StmtPtr = std::shared_ptr<Stmt>;
using ExprPtr = std::shared_ptr<Expr>;
using ProgramPtr = std::shared_ptr<Program>;
using FunctionDeclPtr = std::shared_ptr<FunctionDecl>;
using VarDeclPtr = std::shared_ptr<VarDecl>;
using ParameterDeclPtr = std::shared_ptr<ParameterDecl>;
using BlockStmtPtr = std::shared_ptr<BlockStmt>;
using DeclStmtPtr = std::shared_ptr<DeclStmt>;
using ExprStmtPtr = std::shared_ptr<ExprStmt>;
using IfStmtPtr = std::shared_ptr<IfStmt>;
using WhileStmtPtr = std::shared_ptr<WhileStmt>;
using ReturnStmtPtr = std::shared_ptr<ReturnStmt>;
using AssignExprPtr = std::shared_ptr<AssignExpr>;
using BinaryExprPtr = std::shared_ptr<BinaryExpr>;
using UnaryExprPtr = std::shared_ptr<UnaryExpr>;
using CallExprPtr = std::shared_ptr<CallExpr>;
using IdentifierExprPtr = std::shared_ptr<IdentifierExpr>;
using IntegerLiteralExprPtr = std::shared_ptr<IntegerLiteralExpr>;
using FloatLiteralExprPtr = std::shared_ptr<FloatLiteralExpr>;

using DeclList = std::vector<DeclPtr>;
using StmtList = std::vector<StmtPtr>;
using ExprList = std::vector<ExprPtr>;
using ParameterList = std::vector<ParameterDeclPtr>;

class AstNode {
public:
    AstNode(AstNodeKind kind, common::SourceSpan span = {});
    virtual ~AstNode() = default;

    AstNodeKind kind;
    common::SourceSpan span;
};

class Decl : public AstNode {
protected:
    Decl(AstNodeKind kind, common::SourceSpan span = {});
};

class Stmt : public AstNode {
protected:
    Stmt(AstNodeKind kind, common::SourceSpan span = {});
};

class Expr : public AstNode {
protected:
    Expr(AstNodeKind kind, common::SourceSpan span = {});
};

class Program final : public AstNode {
public:
    explicit Program(DeclList declarations = {}, common::SourceSpan span = {});

    DeclList declarations;
};

class ParameterDecl final : public AstNode {
public:
    ParameterDecl(TypeSpecifier type_specifier, std::string name, common::SourceSpan span = {});

    TypeSpecifier type_specifier;
    std::string name;
};

class VarDecl final : public Decl {
public:
    VarDecl(TypeSpecifier type_specifier, std::string name, ExprPtr initializer = nullptr,
            common::SourceSpan span = {});

    TypeSpecifier type_specifier;
    std::string name;
    ExprPtr initializer;
};

class FunctionDecl final : public Decl {
public:
    FunctionDecl(TypeSpecifier return_type, std::string name, ParameterList parameters = {},
                 BlockStmtPtr body = nullptr, common::SourceSpan span = {});

    TypeSpecifier return_type;
    std::string name;
    ParameterList parameters;
    BlockStmtPtr body;
};

class BlockStmt final : public Stmt {
public:
    explicit BlockStmt(StmtList statements = {}, common::SourceSpan span = {});

    StmtList statements;
};

class DeclStmt final : public Stmt {
public:
    explicit DeclStmt(VarDeclPtr declaration, common::SourceSpan span = {});

    VarDeclPtr declaration;
};

class ExprStmt final : public Stmt {
public:
    explicit ExprStmt(ExprPtr expression = nullptr, common::SourceSpan span = {});

    ExprPtr expression;
};

class IfStmt final : public Stmt {
public:
    IfStmt(ExprPtr condition, StmtPtr then_branch, StmtPtr else_branch = nullptr,
           common::SourceSpan span = {});

    ExprPtr condition;
    StmtPtr then_branch;
    StmtPtr else_branch;
};

class WhileStmt final : public Stmt {
public:
    WhileStmt(ExprPtr condition, StmtPtr body, common::SourceSpan span = {});

    ExprPtr condition;
    StmtPtr body;
};

class ReturnStmt final : public Stmt {
public:
    explicit ReturnStmt(ExprPtr value = nullptr, common::SourceSpan span = {});

    ExprPtr value;
};

class AssignExpr final : public Expr {
public:
    AssignExpr(std::string target_name, ExprPtr value, common::SourceSpan span = {});

    std::string target_name;
    ExprPtr value;
};

class BinaryExpr final : public Expr {
public:
    BinaryExpr(BinaryOperator op, ExprPtr lhs, ExprPtr rhs, common::SourceSpan span = {});

    BinaryOperator op;
    ExprPtr lhs;
    ExprPtr rhs;
};

class UnaryExpr final : public Expr {
public:
    UnaryExpr(UnaryOperator op, ExprPtr operand, common::SourceSpan span = {});

    UnaryOperator op;
    ExprPtr operand;
};

class CallExpr final : public Expr {
public:
    CallExpr(std::string callee, ExprList arguments = {}, common::SourceSpan span = {});

    std::string callee;
    ExprList arguments;
};

class IdentifierExpr final : public Expr {
public:
    explicit IdentifierExpr(std::string name, common::SourceSpan span = {});

    std::string name;
};

class IntegerLiteralExpr final : public Expr {
public:
    explicit IntegerLiteralExpr(std::int64_t value, common::SourceSpan span = {});

    std::int64_t value;
};

class FloatLiteralExpr final : public Expr {
public:
    explicit FloatLiteralExpr(double value, common::SourceSpan span = {});

    double value;
};

std::string to_string(TypeSpecifier specifier);
std::string to_string(AstNodeKind kind);
std::string to_string(BinaryOperator op);
std::string to_string(UnaryOperator op);

}  // namespace compilerlab::semantic
