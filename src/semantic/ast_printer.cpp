#include "semantic/ast_printer.h"

#include <sstream>
#include <utility>
#include <vector>

namespace compilerlab::semantic {

namespace {

struct PrintedNode {
    std::string label;
    std::vector<PrintedNode> children;
};

std::string quote_text(std::string text) {
    return "\"" + text + "\"";
}

PrintedNode make_group_node(std::string label) {
    PrintedNode node;
    node.label = std::move(label);
    return node;
}

void append_if_nonempty(PrintedNode& parent, PrintedNode child) {
    if (!child.label.empty()) {
        parent.children.push_back(std::move(child));
    }
}

PrintedNode build_expr_node(const ExprPtr& expr);
PrintedNode build_stmt_node(const StmtPtr& stmt);
PrintedNode build_decl_node(const DeclPtr& decl);

PrintedNode build_expr_list_node(const ExprList& exprs, std::string label) {
    auto group = make_group_node(std::move(label));
    for (const auto& expr : exprs) {
        append_if_nonempty(group, build_expr_node(expr));
    }
    return group;
}

PrintedNode build_param_list_node(const ParameterList& params) {
    auto group = make_group_node("Parameters");
    for (const auto& param : params) {
        PrintedNode node;
        if (param) {
            node.label = "ParameterDecl name=" + quote_text(param->name) +
                         " type=" + to_string(param->type_specifier);
        } else {
            node.label = "<null-parameter>";
        }
        append_if_nonempty(group, std::move(node));
    }
    return group;
}

PrintedNode build_stmt_list_node(const StmtList& stmts) {
    auto group = make_group_node("Statements");
    for (const auto& stmt : stmts) {
        append_if_nonempty(group, build_stmt_node(stmt));
    }
    return group;
}

PrintedNode build_decl_node(const DeclPtr& decl) {
    PrintedNode node;
    if (!decl) {
        node.label = "<null-decl>";
        return node;
    }

    switch (decl->kind) {
        case AstNodeKind::FunctionDecl: {
            const auto* func = static_cast<const FunctionDecl*>(decl.get());
            node.label = "FunctionDecl name=" + quote_text(func->name) +
                         " return=" + to_string(func->return_type);
            append_if_nonempty(node, build_param_list_node(func->parameters));
            PrintedNode body = make_group_node("Body");
            append_if_nonempty(body, build_stmt_node(func->body));
            append_if_nonempty(node, std::move(body));
            return node;
        }
        case AstNodeKind::VarDecl: {
            const auto* var = static_cast<const VarDecl*>(decl.get());
            node.label = "VarDecl name=" + quote_text(var->name) +
                         " type=" + to_string(var->type_specifier);
            if (var->initializer) {
                PrintedNode init = make_group_node("Initializer");
                append_if_nonempty(init, build_expr_node(var->initializer));
                append_if_nonempty(node, std::move(init));
            }
            return node;
        }
        default:
            node.label = to_string(decl->kind);
            return node;
    }
}

PrintedNode build_stmt_node(const StmtPtr& stmt) {
    PrintedNode node;
    if (!stmt) {
        node.label = "<null-stmt>";
        return node;
    }

    switch (stmt->kind) {
        case AstNodeKind::BlockStmt: {
            const auto* block = static_cast<const BlockStmt*>(stmt.get());
            node.label = "BlockStmt";
            append_if_nonempty(node, build_stmt_list_node(block->statements));
            return node;
        }
        case AstNodeKind::DeclStmt: {
            const auto* decl_stmt = static_cast<const DeclStmt*>(stmt.get());
            node.label = "DeclStmt";
            append_if_nonempty(node, build_decl_node(decl_stmt->declaration));
            return node;
        }
        case AstNodeKind::ExprStmt: {
            const auto* expr_stmt = static_cast<const ExprStmt*>(stmt.get());
            node.label = expr_stmt->expression ? "ExprStmt" : "ExprStmt empty";
            if (expr_stmt->expression) {
                append_if_nonempty(node, build_expr_node(expr_stmt->expression));
            }
            return node;
        }
        case AstNodeKind::IfStmt: {
            const auto* if_stmt = static_cast<const IfStmt*>(stmt.get());
            node.label = "IfStmt";
            PrintedNode condition = make_group_node("Condition");
            append_if_nonempty(condition, build_expr_node(if_stmt->condition));
            append_if_nonempty(node, std::move(condition));

            PrintedNode then_branch = make_group_node("Then");
            append_if_nonempty(then_branch, build_stmt_node(if_stmt->then_branch));
            append_if_nonempty(node, std::move(then_branch));

            if (if_stmt->else_branch) {
                PrintedNode else_branch = make_group_node("Else");
                append_if_nonempty(else_branch, build_stmt_node(if_stmt->else_branch));
                append_if_nonempty(node, std::move(else_branch));
            }
            return node;
        }
        case AstNodeKind::WhileStmt: {
            const auto* while_stmt = static_cast<const WhileStmt*>(stmt.get());
            node.label = "WhileStmt";
            PrintedNode condition = make_group_node("Condition");
            append_if_nonempty(condition, build_expr_node(while_stmt->condition));
            append_if_nonempty(node, std::move(condition));

            PrintedNode body = make_group_node("Body");
            append_if_nonempty(body, build_stmt_node(while_stmt->body));
            append_if_nonempty(node, std::move(body));
            return node;
        }
        case AstNodeKind::ReturnStmt: {
            const auto* return_stmt = static_cast<const ReturnStmt*>(stmt.get());
            node.label = return_stmt->value ? "ReturnStmt" : "ReturnStmt void";
            if (return_stmt->value) {
                append_if_nonempty(node, build_expr_node(return_stmt->value));
            }
            return node;
        }
        default:
            node.label = to_string(stmt->kind);
            return node;
    }
}

PrintedNode build_expr_node(const ExprPtr& expr) {
    PrintedNode node;
    if (!expr) {
        node.label = "<null-expr>";
        return node;
    }

    switch (expr->kind) {
        case AstNodeKind::AssignExpr: {
            const auto* assign = static_cast<const AssignExpr*>(expr.get());
            node.label = "AssignExpr target=" + quote_text(assign->target_name);
            append_if_nonempty(node, build_expr_node(assign->value));
            return node;
        }
        case AstNodeKind::BinaryExpr: {
            const auto* binary = static_cast<const BinaryExpr*>(expr.get());
            node.label = "BinaryExpr op=" + quote_text(to_string(binary->op));
            PrintedNode lhs = make_group_node("Left");
            append_if_nonempty(lhs, build_expr_node(binary->lhs));
            append_if_nonempty(node, std::move(lhs));
            PrintedNode rhs = make_group_node("Right");
            append_if_nonempty(rhs, build_expr_node(binary->rhs));
            append_if_nonempty(node, std::move(rhs));
            return node;
        }
        case AstNodeKind::UnaryExpr: {
            const auto* unary = static_cast<const UnaryExpr*>(expr.get());
            node.label = "UnaryExpr op=" + quote_text(to_string(unary->op));
            append_if_nonempty(node, build_expr_node(unary->operand));
            return node;
        }
        case AstNodeKind::CallExpr: {
            const auto* call = static_cast<const CallExpr*>(expr.get());
            node.label = "CallExpr callee=" + quote_text(call->callee);
            append_if_nonempty(node, build_expr_list_node(call->arguments, "Arguments"));
            return node;
        }
        case AstNodeKind::IdentifierExpr: {
            const auto* ident = static_cast<const IdentifierExpr*>(expr.get());
            node.label = "IdentifierExpr name=" + quote_text(ident->name);
            return node;
        }
        case AstNodeKind::IntegerLiteralExpr: {
            const auto* lit = static_cast<const IntegerLiteralExpr*>(expr.get());
            node.label = "IntegerLiteralExpr value=" + std::to_string(lit->value);
            return node;
        }
        case AstNodeKind::FloatLiteralExpr: {
            const auto* lit = static_cast<const FloatLiteralExpr*>(expr.get());
            std::ostringstream text;
            text << lit->value;
            node.label = "FloatLiteralExpr value=" + text.str();
            return node;
        }
        default:
            node.label = to_string(expr->kind);
            return node;
    }
}

PrintedNode build_program_node(const ProgramPtr& program) {
    PrintedNode node;
    if (!program) {
        node.label = "<null-program>";
        return node;
    }

    node.label = "Program";
    PrintedNode decls = make_group_node("Declarations");
    for (const auto& decl : program->declarations) {
        append_if_nonempty(decls, build_decl_node(decl));
    }
    append_if_nonempty(node, std::move(decls));
    return node;
}

void render_text_node(const PrintedNode& node,
                      const std::string& prefix,
                      bool is_last,
                      std::ostringstream& out) {
    out << prefix << (is_last ? "\\- " : "|- ") << node.label << "\n";

    const auto child_prefix = prefix + (is_last ? "   " : "|  ");
    for (std::size_t index = 0; index < node.children.size(); ++index) {
        render_text_node(node.children[index],
                         child_prefix,
                         index + 1 == node.children.size(),
                         out);
    }
}

void render_markdown_node(const PrintedNode& node,
                          std::size_t depth,
                          std::ostringstream& out) {
    out << std::string(depth * 2, ' ') << "- `" << node.label << "`\n";
    for (const auto& child : node.children) {
        render_markdown_node(child, depth + 1, out);
    }
}

}  // namespace

std::string AstPrinter::print_text(const ProgramPtr& program) const {
    std::ostringstream out;
    const auto root = build_program_node(program);
    out << root.label << "\n";
    for (std::size_t index = 0; index < root.children.size(); ++index) {
        render_text_node(root.children[index],
                         "",
                         index + 1 == root.children.size(),
                         out);
    }
    return out.str();
}

std::string AstPrinter::print_markdown(const ProgramPtr& program) const {
    std::ostringstream out;
    out << "# AST\n\n";
    render_markdown_node(build_program_node(program), 0, out);
    return out.str();
}

}  // namespace compilerlab::semantic
