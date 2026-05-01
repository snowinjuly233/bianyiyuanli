#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace compilerlab::generated {

struct SourceLocation {
    std::size_t offset {0};
    std::size_t line {1};
    std::size_t column {1};
};

struct SourceSpan {
    std::string file_path;
    SourceLocation begin {};
    SourceLocation end {};
};

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
    LogicalNot
};

struct Token {
    TokenKind kind {TokenKind::Invalid};
    std::string lexeme;
    SourceSpan span {};

    bool is(TokenKind expected) const {
        return kind == expected;
    }
};

std::string to_string(TokenKind kind);
std::string escape_text(std::string_view text);

class MinicScanner {
public:
    explicit MinicScanner(std::string file_path = {});

    Token next_token(std::string_view input, std::size_t& offset) const;
    std::vector<Token> scan_all(std::string_view input) const;

private:
    struct ScannerAcceptAction {
        TokenKind kind {TokenKind::Invalid};
        std::size_t rule_priority {0};
        bool skip {false};
    };

    struct ScannerState {
        int id {0};
        std::optional<ScannerAcceptAction> accept;
        std::vector<std::pair<char, int>> transitions;
    };

    static std::vector<ScannerState> build_states();
    static SourceLocation locate(std::string_view input, std::size_t offset);

    std::vector<ScannerState> states_;
    std::string file_path_;
};

}  // namespace compilerlab::generated
