#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "common/source.h"

namespace compilerlab::common {

enum class TokenKind {
    Invalid,
    EndOfFile,

    Identifier,
    IntegerLiteral,
    FloatLiteral,

    KwInt,
    KwFloat,
    KwVoid,
    KwIf,
    KwElse,
    KwWhile,
    KwReturn,

    LParen,
    RParen,
    LBrace,
    RBrace,
    Comma,
    Semicolon,

    Plus,
    Minus,
    Star,
    Slash,
    Percent,

    Assign,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    LogicalAnd,
    LogicalOr,
    LogicalNot,
};

struct Token {
    TokenKind kind {TokenKind::Invalid};
    std::string lexeme;
    SourceSpan span {};

    bool is(TokenKind expected) const;
};

std::string to_string(TokenKind kind);
bool is_keyword(TokenKind kind);
std::optional<TokenKind> keyword_token_kind(std::string_view lexeme);
std::optional<TokenKind> token_kind_from_name(std::string_view name);

}  // namespace compilerlab::common
