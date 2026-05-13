#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace compilerlab::seulex {

enum class RegexKind {
    Empty,
    Literal,
    Concat,
    Alternate,
    Star,
    Plus,
    Optional,
    CharacterClass,
};

struct RegexNode {
    explicit RegexNode(RegexKind kind, std::string text = {})
        : kind(kind), text(std::move(text)) {
    }

    RegexKind kind {RegexKind::Empty};
    std::string text;
    std::vector<std::unique_ptr<RegexNode>> children;
};

using RegexNodePtr = std::unique_ptr<RegexNode>;

}  // namespace compilerlab::seulex
