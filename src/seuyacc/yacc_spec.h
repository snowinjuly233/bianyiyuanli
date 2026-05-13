#pragma once

#include <string>
#include <vector>

namespace compilerlab::seuyacc {

enum class Assoc {
    None,
    Left,
    Right,
};

struct TokenDecl {
    std::string name;
    std::string type_tag;
};

struct TypeDecl {
    std::string type_tag;
    std::vector<std::string> symbols;
};

struct PrecedenceDecl {
    Assoc assoc {Assoc::None};
    std::vector<std::string> tokens;
};

struct Production {
    std::string lhs;
    std::vector<std::string> rhs;
    std::string action_text;
    int precedence_level {0};
    std::string precedence_token;
};

struct YaccSpec {
    std::string prologue;
    std::string epilogue;
    std::string union_text;
    std::string start_symbol;
    std::vector<TokenDecl> tokens;
    std::vector<TypeDecl> typed_symbols;
    std::vector<PrecedenceDecl> precedences;
    std::vector<Production> productions;
};

}  // namespace compilerlab::seuyacc
