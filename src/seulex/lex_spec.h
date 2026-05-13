#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "common/token.h"

namespace compilerlab::seulex {

enum class LexActionKind {
    Emit,
    Skip,
};

struct LexDefinition {
    std::string name;
    std::string pattern;
};

struct LexRule {
    std::string regex_text;
    std::string normalized_regex;
    std::string action_text;
    std::size_t priority {0};
    LexActionKind action_kind {LexActionKind::Emit};
    common::TokenKind token_kind {common::TokenKind::Invalid};
};

struct LexSpec {
    std::string prologue;
    std::string epilogue;
    std::vector<LexDefinition> definitions;
    std::vector<LexRule> rules;
};

}  // namespace compilerlab::seulex
