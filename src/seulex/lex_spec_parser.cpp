#include "seulex/lex_spec_parser.h"

#include <cctype>
#include <exception>
#include <string_view>

#include "common/string_utils.h"

namespace compilerlab::seulex {

namespace {

struct SpecSections {
    std::string definitions;
    std::string rules;
    std::string epilogue;
};

struct RawRule {
    std::string regex_text;
    std::string action_text;
};

bool starts_with(std::string_view text, std::string_view prefix) {
    return text.substr(0, prefix.size()) == prefix;
}

std::size_t find_action_start(const std::string& line) {
    bool in_quote = false;
    bool in_class = false;
    bool escaped = false;

    for (std::size_t index = 0; index < line.size(); ++index) {
        const char ch = line[index];
        if (escaped) {
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        if (ch == '"' && !in_class) {
            in_quote = !in_quote;
            continue;
        }
        if (!in_quote) {
            if (ch == '[') {
                in_class = true;
                continue;
            }
            if (ch == ']' && in_class) {
                in_class = false;
                continue;
            }
            if (ch == '{' && !in_class && index > 0 &&
                std::isspace(static_cast<unsigned char>(line[index - 1])) != 0) {
                return index;
            }
        }
    }

    return std::string::npos;
}

std::size_t find_last_action_separator(const std::string& line) {
    bool in_quote = false;
    bool in_class = false;
    bool escaped = false;
    std::size_t last_whitespace = std::string::npos;

    for (std::size_t index = 0; index < line.size(); ++index) {
        const char ch = line[index];
        if (escaped) {
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        if (ch == '"' && !in_class) {
            in_quote = !in_quote;
            continue;
        }
        if (!in_quote) {
            if (ch == '[') {
                in_class = true;
                continue;
            }
            if (ch == ']' && in_class) {
                in_class = false;
                continue;
            }
            if (!in_class && std::isspace(static_cast<unsigned char>(ch)) != 0) {
                last_whitespace = index;
            }
        }
    }

    return last_whitespace;
}

SpecSections split_sections(const std::string& content, common::DiagnosticEngine& diagnostics) {
    const auto first = content.find("%%");
    if (first == std::string::npos) {
        diagnostics.error("lex specification is missing the first %% separator");
        return {};
    }

    const auto second = content.find("%%", first + 2);
    if (second == std::string::npos) {
        diagnostics.error("lex specification is missing the second %% separator");
        return {};
    }

    return {
        content.substr(0, first),
        content.substr(first + 2, second - first - 2),
        content.substr(second + 2),
    };
}

void parse_action(LexRule& rule, common::DiagnosticEngine& diagnostics) {
    auto action = common::trim(rule.action_text);
    if (action == "skip;" || action == "skip" || action == ";") {
        rule.action_kind = LexActionKind::Skip;
        rule.token_kind = common::TokenKind::Invalid;
        return;
    }

    constexpr std::string_view prefix = "return";
    if (starts_with(action, prefix)) {
        action = common::trim(action.substr(prefix.size()));
        if (!action.empty() && action.back() == ';') {
            action.pop_back();
            action = common::trim(action);
        }
        if (const auto token = common::token_kind_from_name(action)) {
            rule.action_kind = LexActionKind::Emit;
            rule.token_kind = *token;
            return;
        }
    }

    diagnostics.error("unsupported lex action: " + rule.action_text);
}

std::optional<RawRule> parse_raw_rule(const std::string& line, common::DiagnosticEngine& diagnostics) {
    const auto action_start = find_action_start(line);
    if (action_start != std::string::npos) {
        const auto action_end = line.rfind('}');
        if (action_end == std::string::npos || action_end <= action_start) {
            diagnostics.error("malformed action block in lex rule: " + line);
            return std::nullopt;
        }
        return RawRule{
            common::trim(line.substr(0, action_start)),
            common::trim(line.substr(action_start + 1, action_end - action_start - 1)),
        };
    }

    if (!line.empty() && (line.back() == ';' || line.back() == '|')) {
        const auto separator = find_last_action_separator(line);
        if (separator == std::string::npos) {
            diagnostics.error("failed to split short action in lex rule: " + line);
            return std::nullopt;
        }
        return RawRule{
            common::trim(line.substr(0, separator)),
            common::trim(line.substr(separator)),
        };
    }

    diagnostics.error("failed to find an action block in lex rule: " + line);
    return std::nullopt;
}

}  // namespace

LexSpec LexSpecParser::parse_file(const std::string& path, common::SourceManager& sources,
                                  common::DiagnosticEngine& diagnostics) const {
    LexSpec spec;

    try {
        const auto file = sources.load_file(path);
        const auto sections = split_sections(file->content(), diagnostics);
        if (diagnostics.has_error()) {
            return spec;
        }

        auto definitions_text = sections.definitions;
        const auto prologue_start = definitions_text.find("%{");
        const auto prologue_end = definitions_text.find("%}");
        if (prologue_start != std::string::npos && prologue_end != std::string::npos &&
            prologue_start < prologue_end) {
            spec.prologue = definitions_text.substr(prologue_start + 2, prologue_end - prologue_start - 2);
            definitions_text.erase(prologue_start, prologue_end - prologue_start + 2);
        }

        for (const auto& line : common::split_lines(definitions_text)) {
            auto trimmed = common::trim(line);
            if (trimmed.empty() || starts_with(trimmed, "//") || starts_with(trimmed, "/*")) {
                continue;
            }

            std::size_t index = 0;
            while (index < trimmed.size() &&
                   std::isspace(static_cast<unsigned char>(trimmed[index])) == 0) {
                ++index;
            }

            const auto name = trimmed.substr(0, index);
            const auto pattern = common::trim(trimmed.substr(index));
            if (name.empty() || pattern.empty()) {
                diagnostics.error("malformed lex definition line: " + trimmed);
                continue;
            }

            spec.definitions.push_back({name, pattern});
        }

        std::vector<std::size_t> continued_rule_indices;
        std::size_t priority = 0;
        for (const auto& line : common::split_lines(sections.rules)) {
            auto trimmed = common::trim(line);
            if (trimmed.empty() || starts_with(trimmed, "//") || starts_with(trimmed, "/*")) {
                continue;
            }

            const auto raw_rule = parse_raw_rule(trimmed, diagnostics);
            if (!raw_rule.has_value()) {
                continue;
            }

            LexRule rule;
            rule.regex_text = raw_rule->regex_text;
            rule.action_text = raw_rule->action_text;
            rule.priority = priority++;
            spec.rules.push_back(std::move(rule));

            const auto current_index = spec.rules.size() - 1;
            if (raw_rule->action_text == "|") {
                continued_rule_indices.push_back(current_index);
                continue;
            }

            parse_action(spec.rules[current_index], diagnostics);
            for (const auto continuation_index : continued_rule_indices) {
                spec.rules[continuation_index].action_text = spec.rules[current_index].action_text;
                spec.rules[continuation_index].action_kind = spec.rules[current_index].action_kind;
                spec.rules[continuation_index].token_kind = spec.rules[current_index].token_kind;
            }
            continued_rule_indices.clear();
        }

        if (!continued_rule_indices.empty()) {
            diagnostics.error("lex rule continuation '|' cannot appear on the last rule line");
        }

        spec.epilogue = sections.epilogue;
    } catch (const std::exception& ex) {
        diagnostics.error(ex.what());
    }

    return spec;
}

}  // namespace compilerlab::seulex
