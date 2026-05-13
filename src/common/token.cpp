#include "common/token.h"

namespace compilerlab::common {

bool Token::is(TokenKind expected) const {
    return kind == expected;
}

std::string to_string(TokenKind kind) {
    switch (kind) {
        case TokenKind::Invalid: return "Invalid";
        case TokenKind::EndOfFile: return "EndOfFile";
        case TokenKind::Identifier: return "Identifier";
        case TokenKind::IntegerLiteral: return "IntegerLiteral";
        case TokenKind::FloatLiteral: return "FloatLiteral";
        case TokenKind::KwInt: return "KwInt";
        case TokenKind::KwFloat: return "KwFloat";
        case TokenKind::KwVoid: return "KwVoid";
        case TokenKind::KwIf: return "KwIf";
        case TokenKind::KwElse: return "KwElse";
        case TokenKind::KwWhile: return "KwWhile";
        case TokenKind::KwReturn: return "KwReturn";
        case TokenKind::LParen: return "LParen";
        case TokenKind::RParen: return "RParen";
        case TokenKind::LBrace: return "LBrace";
        case TokenKind::RBrace: return "RBrace";
        case TokenKind::Comma: return "Comma";
        case TokenKind::Semicolon: return "Semicolon";
        case TokenKind::Plus: return "Plus";
        case TokenKind::Minus: return "Minus";
        case TokenKind::Star: return "Star";
        case TokenKind::Slash: return "Slash";
        case TokenKind::Percent: return "Percent";
        case TokenKind::Assign: return "Assign";
        case TokenKind::Equal: return "Equal";
        case TokenKind::NotEqual: return "NotEqual";
        case TokenKind::Less: return "Less";
        case TokenKind::LessEqual: return "LessEqual";
        case TokenKind::Greater: return "Greater";
        case TokenKind::GreaterEqual: return "GreaterEqual";
        case TokenKind::LogicalAnd: return "LogicalAnd";
        case TokenKind::LogicalOr: return "LogicalOr";
        case TokenKind::LogicalNot: return "LogicalNot";
    }

    return "UnknownTokenKind";
}

bool is_keyword(TokenKind kind) {
    switch (kind) {
        case TokenKind::KwInt:
        case TokenKind::KwFloat:
        case TokenKind::KwVoid:
        case TokenKind::KwIf:
        case TokenKind::KwElse:
        case TokenKind::KwWhile:
        case TokenKind::KwReturn:
            return true;
        default:
            return false;
    }
}

std::optional<TokenKind> keyword_token_kind(std::string_view lexeme) {
    if (lexeme == "int") {
        return TokenKind::KwInt;
    }
    if (lexeme == "float") {
        return TokenKind::KwFloat;
    }
    if (lexeme == "void") {
        return TokenKind::KwVoid;
    }
    if (lexeme == "if") {
        return TokenKind::KwIf;
    }
    if (lexeme == "else") {
        return TokenKind::KwElse;
    }
    if (lexeme == "while") {
        return TokenKind::KwWhile;
    }
    if (lexeme == "return") {
        return TokenKind::KwReturn;
    }
    return std::nullopt;
}

std::optional<TokenKind> token_kind_from_name(std::string_view name) {
    if (name == "Invalid") return TokenKind::Invalid;
    if (name == "EndOfFile") return TokenKind::EndOfFile;
    if (name == "Identifier") return TokenKind::Identifier;
    if (name == "IntegerLiteral") return TokenKind::IntegerLiteral;
    if (name == "FloatLiteral") return TokenKind::FloatLiteral;
    if (name == "KwInt") return TokenKind::KwInt;
    if (name == "KwFloat") return TokenKind::KwFloat;
    if (name == "KwVoid") return TokenKind::KwVoid;
    if (name == "KwIf") return TokenKind::KwIf;
    if (name == "KwElse") return TokenKind::KwElse;
    if (name == "KwWhile") return TokenKind::KwWhile;
    if (name == "KwReturn") return TokenKind::KwReturn;
    if (name == "LParen") return TokenKind::LParen;
    if (name == "RParen") return TokenKind::RParen;
    if (name == "LBrace") return TokenKind::LBrace;
    if (name == "RBrace") return TokenKind::RBrace;
    if (name == "Comma") return TokenKind::Comma;
    if (name == "Semicolon") return TokenKind::Semicolon;
    if (name == "Plus") return TokenKind::Plus;
    if (name == "Minus") return TokenKind::Minus;
    if (name == "Star") return TokenKind::Star;
    if (name == "Slash") return TokenKind::Slash;
    if (name == "Percent") return TokenKind::Percent;
    if (name == "Assign") return TokenKind::Assign;
    if (name == "Equal") return TokenKind::Equal;
    if (name == "NotEqual") return TokenKind::NotEqual;
    if (name == "Less") return TokenKind::Less;
    if (name == "LessEqual") return TokenKind::LessEqual;
    if (name == "Greater") return TokenKind::Greater;
    if (name == "GreaterEqual") return TokenKind::GreaterEqual;
    if (name == "LogicalAnd") return TokenKind::LogicalAnd;
    if (name == "LogicalOr") return TokenKind::LogicalOr;
    if (name == "LogicalNot") return TokenKind::LogicalNot;
    return std::nullopt;
}

}  // namespace compilerlab::common
