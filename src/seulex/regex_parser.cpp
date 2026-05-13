#include "seulex/regex_parser.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace compilerlab::seulex {

namespace {

char decode_escape(char ch) {
    switch (ch) {
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        default: return ch;
    }
}

std::string any_chars_except(bool exclude_newline) {
    std::string chars;
    chars.reserve(255);
    for (int value = 1; value <= 255; ++value) {
        if (exclude_newline && value == '\n') {
            continue;
        }
        chars.push_back(static_cast<char>(value));
    }
    return chars;
}

RegexNodePtr make_empty_node() {
    return std::make_unique<RegexNode>(RegexKind::Empty);
}

RegexNodePtr make_literal_node(char ch) {
    return std::make_unique<RegexNode>(RegexKind::Literal, std::string(1, ch));
}

RegexNodePtr make_character_class_node(std::string chars) {
    std::sort(chars.begin(), chars.end());
    chars.erase(std::unique(chars.begin(), chars.end()), chars.end());
    return std::make_unique<RegexNode>(RegexKind::CharacterClass, std::move(chars));
}

RegexNodePtr make_concat_node(std::vector<RegexNodePtr> children) {
    if (children.empty()) {
        return make_empty_node();
    }
    if (children.size() == 1) {
        return std::move(children.front());
    }

    auto node = std::make_unique<RegexNode>(RegexKind::Concat);
    node->children = std::move(children);
    return node;
}

RegexNodePtr make_alternate_node(std::vector<RegexNodePtr> children) {
    if (children.empty()) {
        return make_empty_node();
    }
    if (children.size() == 1) {
        return std::move(children.front());
    }

    auto node = std::make_unique<RegexNode>(RegexKind::Alternate);
    node->children = std::move(children);
    return node;
}

RegexNodePtr make_unary_node(RegexKind kind, RegexNodePtr child) {
    auto node = std::make_unique<RegexNode>(kind);
    node->children.push_back(std::move(child));
    return node;
}

RegexNodePtr clone_node(const RegexNode& node) {
    auto copy = std::make_unique<RegexNode>(node.kind, node.text);
    for (const auto& child : node.children) {
        copy->children.push_back(clone_node(*child));
    }
    return copy;
}

RegexNodePtr make_repetition_node(const RegexNode& base, int min_repeat, std::optional<int> max_repeat,
                                  common::DiagnosticEngine& diagnostics) {
    if (min_repeat < 0) {
        diagnostics.error("regex repetition lower bound cannot be negative");
        return make_empty_node();
    }
    if (max_repeat.has_value() && *max_repeat < min_repeat) {
        diagnostics.error("regex repetition upper bound is smaller than lower bound");
        return make_empty_node();
    }

    std::vector<RegexNodePtr> pieces;
    for (int index = 0; index < min_repeat; ++index) {
        pieces.push_back(clone_node(base));
    }

    if (!max_repeat.has_value()) {
        if (min_repeat == 0) {
            return make_unary_node(RegexKind::Star, clone_node(base));
        }
        pieces.push_back(make_unary_node(RegexKind::Star, clone_node(base)));
        return make_concat_node(std::move(pieces));
    }

    for (int index = min_repeat; index < *max_repeat; ++index) {
        pieces.push_back(make_unary_node(RegexKind::Optional, clone_node(base)));
    }

    return make_concat_node(std::move(pieces));
}

RegexNodePtr make_literal_sequence(const std::string& text) {
    std::vector<RegexNodePtr> children;
    children.reserve(text.size());
    for (char ch : text) {
        children.push_back(make_literal_node(ch));
    }
    return make_concat_node(std::move(children));
}

class Parser {
public:
    Parser(const std::string& pattern, common::DiagnosticEngine& diagnostics)
        : pattern_(pattern), diagnostics_(diagnostics) {
    }

    RegexNodePtr parse() {
        if (pattern_.empty()) {
            diagnostics_.warning("empty regex pattern");
            return make_empty_node();
        }

        auto result = parse_expression();
        if (!at_end()) {
            diagnostics_.error("unexpected trailing regex content near: " + pattern_.substr(position_));
        }
        return result;
    }

private:
    RegexNodePtr parse_expression() {
        std::vector<RegexNodePtr> branches;
        branches.push_back(parse_concatenation());
        while (match('|')) {
            branches.push_back(parse_concatenation());
        }
        return make_alternate_node(std::move(branches));
    }

    RegexNodePtr parse_concatenation() {
        std::vector<RegexNodePtr> pieces;
        while (!at_end() && peek() != ')' && peek() != '|') {
            pieces.push_back(parse_postfix());
        }
        return make_concat_node(std::move(pieces));
    }

    RegexNodePtr parse_postfix() {
        auto node = parse_atom();
        while (!at_end()) {
            if (match('*')) {
                node = make_unary_node(RegexKind::Star, std::move(node));
                continue;
            }
            if (match('+')) {
                node = make_unary_node(RegexKind::Plus, std::move(node));
                continue;
            }
            if (match('?')) {
                node = make_unary_node(RegexKind::Optional, std::move(node));
                continue;
            }
            if (peek() == '{') {
                node = parse_repetition(std::move(node));
                continue;
            }
            break;
        }
        return node;
    }

    RegexNodePtr parse_atom() {
        if (at_end()) {
            diagnostics_.error("unexpected end of regex");
            return make_empty_node();
        }

        if (match('(')) {
            auto inside = parse_expression();
            if (!match(')')) {
                diagnostics_.error("missing ')' in regex: " + pattern_);
            }
            return inside;
        }

        if (match('"')) {
            std::string literal;
            while (!at_end() && peek() != '"') {
                if (match('\\')) {
                    if (at_end()) {
                        diagnostics_.error("dangling escape inside quoted literal");
                        break;
                    }
                    literal.push_back(decode_escape(advance()));
                    continue;
                }
                literal.push_back(advance());
            }
            if (!match('"')) {
                diagnostics_.error("missing closing quote in regex: " + pattern_);
            }
            return make_literal_sequence(literal);
        }

        if (match('[')) {
            return parse_character_class();
        }

        if (match('.')) {
            return make_character_class_node(any_chars_except(true));
        }

        if (match('\\')) {
            if (at_end()) {
                diagnostics_.error("dangling escape at end of regex");
                return make_empty_node();
            }
            return make_literal_node(decode_escape(advance()));
        }

        const char ch = advance();
        if (ch == ')' || ch == '|' || ch == '*' || ch == '+' || ch == '?') {
            diagnostics_.error(std::string("unexpected regex operator: ") + ch);
            return make_empty_node();
        }
        return make_literal_node(ch);
    }

    RegexNodePtr parse_character_class() {
        std::set<char> chars;
        bool negated = match('^');
        bool closed = false;

        while (!at_end()) {
            if (match(']')) {
                closed = true;
                break;
            }

            auto first = parse_class_char();
            if (!first.has_value()) {
                break;
            }

            if (*first != '-' && !at_end() && peek() == '-' && position_ + 1 < pattern_.size() &&
                pattern_[position_ + 1] != ']') {
                advance();
                auto second = parse_class_char();
                if (!second.has_value()) {
                    break;
                }

                const auto from = static_cast<unsigned char>(*first);
                const auto to = static_cast<unsigned char>(*second);
                if (from > to) {
                    diagnostics_.error("invalid character range in regex class");
                    chars.insert(*first);
                    chars.insert(*second);
                } else {
                    for (unsigned int value = from; value <= to; ++value) {
                        chars.insert(static_cast<char>(value));
                    }
                }
                continue;
            }

            chars.insert(*first);
        }

        if (!closed) {
            diagnostics_.error("missing closing ']' in regex: " + pattern_);
        }

        if (negated) {
            std::set<char> complemented;
            for (int value = 1; value <= 255; ++value) {
                const auto ch = static_cast<char>(value);
                if (chars.find(ch) == chars.end()) {
                    complemented.insert(ch);
                }
            }
            chars = std::move(complemented);
        }

        std::string class_chars;
        class_chars.reserve(chars.size());
        for (char ch : chars) {
            class_chars.push_back(ch);
        }
        return make_character_class_node(std::move(class_chars));
    }

    RegexNodePtr parse_repetition(RegexNodePtr base) {
        if (!match('{')) {
            return base;
        }

        const auto min_value = parse_integer();
        if (!min_value.has_value()) {
            diagnostics_.error("expected integer repetition lower bound in regex");
            skip_until_closing_brace();
            return make_empty_node();
        }

        std::optional<int> max_value = *min_value;
        if (match(',')) {
            max_value.reset();
            if (std::isdigit(static_cast<unsigned char>(peek_or('\0'))) != 0) {
                max_value = parse_integer();
            }
        }

        if (!match('}')) {
            diagnostics_.error("missing closing '}' in regex repetition: " + pattern_);
            skip_until_closing_brace();
        }

        return make_repetition_node(*base, *min_value, max_value, diagnostics_);
    }

    std::optional<char> parse_class_char() {
        if (at_end()) {
            diagnostics_.error("unexpected end of character class");
            return std::nullopt;
        }

        if (match('\\')) {
            if (at_end()) {
                diagnostics_.error("dangling escape in character class");
                return std::nullopt;
            }
            return decode_escape(advance());
        }

        return advance();
    }

    std::optional<int> parse_integer() {
        if (at_end() || std::isdigit(static_cast<unsigned char>(peek())) == 0) {
            return std::nullopt;
        }

        int value = 0;
        while (!at_end() && std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            value = value * 10 + (advance() - '0');
        }
        return value;
    }

    void skip_until_closing_brace() {
        while (!at_end() && !match('}')) {
            advance();
        }
    }

    char peek_or(char fallback) const {
        return at_end() ? fallback : pattern_[position_];
    }

    bool at_end() const {
        return position_ >= pattern_.size();
    }

    char peek() const {
        return pattern_[position_];
    }

    char advance() {
        return pattern_[position_++];
    }

    bool match(char expected) {
        if (at_end() || pattern_[position_] != expected) {
            return false;
        }
        ++position_;
        return true;
    }

    const std::string& pattern_;
    common::DiagnosticEngine& diagnostics_;
    std::size_t position_ {0};
};

}  // namespace

RegexNodePtr RegexParser::parse(const std::string& pattern, common::DiagnosticEngine& diagnostics) const {
    Parser parser(pattern, diagnostics);
    return parser.parse();
}

}  // namespace compilerlab::seulex
