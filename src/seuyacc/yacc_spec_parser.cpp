#include "seuyacc/yacc_spec_parser.h"

#include <cctype>
#include <exception>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

#include "common/string_utils.h"

namespace compilerlab::seuyacc {

namespace {

struct SpecSections {
    std::string declarations;
    std::string rules;
    std::string epilogue;
};

bool starts_with_at(std::string_view text, std::size_t offset, std::string_view prefix) {
    return offset + prefix.size() <= text.size() && text.substr(offset, prefix.size()) == prefix;
}

std::string replace_all(std::string text, std::string_view from, std::string_view to) {
    std::size_t position = 0;
    while ((position = text.find(from, position)) != std::string::npos) {
        text.replace(position, from.size(), to);
        position += to.size();
    }
    return text;
}

SpecSections split_sections(const std::string& content, common::DiagnosticEngine& diagnostics) {
    const auto first = content.find("%%");
    if (first == std::string::npos) {
        diagnostics.error("yacc specification is missing the first %% separator");
        return {};
    }

    const auto second = content.find("%%", first + 2);
    if (second == std::string::npos) {
        diagnostics.error("yacc specification is missing the second %% separator");
        return {};
    }

    return {
        content.substr(0, first),
        content.substr(first + 2, second - first - 2),
        content.substr(second + 2),
    };
}

void skip_quoted(std::string_view text, std::size_t& position, char quote) {
    ++position;
    while (position < text.size()) {
        const auto ch = text[position++];
        if (ch == '\\' && position < text.size()) {
            ++position;
            continue;
        }
        if (ch == quote) {
            break;
        }
    }
}

void skip_space_and_comments(std::string_view text, std::size_t& position, bool include_newlines) {
    while (position < text.size()) {
        const auto ch = text[position];
        if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
            if (!include_newlines && (ch == '\n' || ch == '\r')) {
                return;
            }
            ++position;
            continue;
        }

        if (starts_with_at(text, position, "//")) {
            position += 2;
            while (position < text.size() && text[position] != '\n') {
                ++position;
            }
            continue;
        }

        if (starts_with_at(text, position, "/*")) {
            position += 2;
            while (position + 1 < text.size() && !starts_with_at(text, position, "*/")) {
                ++position;
            }
            if (position + 1 < text.size()) {
                position += 2;
            } else {
                position = text.size();
            }
            continue;
        }

        return;
    }
}

void consume_line_end(std::string_view text, std::size_t& position) {
    if (position < text.size() && text[position] == '\r') {
        ++position;
    }
    if (position < text.size() && text[position] == '\n') {
        ++position;
    }
}

std::string read_identifier(std::string_view text, std::size_t& position) {
    if (position >= text.size()) {
        return {};
    }

    const auto start = position;
    if (text[position] == '%') {
        ++position;
        while (position < text.size() && common::is_identifier_continue(text[position])) {
            ++position;
        }
        return std::string(text.substr(start, position - start));
    }

    if (!common::is_identifier_start(text[position])) {
        return {};
    }

    ++position;
    while (position < text.size() && common::is_identifier_continue(text[position])) {
        ++position;
    }
    return std::string(text.substr(start, position - start));
}

std::string read_quoted_symbol(std::string_view text, std::size_t& position) {
    if (position >= text.size() || (text[position] != '\'' && text[position] != '"')) {
        return {};
    }

    const auto quote = text[position];
    const auto start = position;
    skip_quoted(text, position, quote);
    return std::string(text.substr(start, position - start));
}

std::string read_symbol(std::string_view text, std::size_t& position) {
    if (position >= text.size()) {
        return {};
    }

    if (text[position] == '\'' || text[position] == '"') {
        return read_quoted_symbol(text, position);
    }
    return read_identifier(text, position);
}

std::string read_angle_block(std::string_view text, std::size_t& position,
                             common::DiagnosticEngine& diagnostics, std::string_view context) {
    skip_space_and_comments(text, position, false);
    if (position >= text.size() || text[position] != '<') {
        diagnostics.error(std::string(context) + " requires a <type-tag>");
        return {};
    }

    const auto start = ++position;
    while (position < text.size() && text[position] != '>') {
        ++position;
    }
    if (position >= text.size()) {
        diagnostics.error("unterminated type tag in yacc declarations");
        return {};
    }

    const auto tag = common::trim(text.substr(start, position - start));
    ++position;
    return tag;
}

std::string read_balanced_block(std::string_view text, std::size_t& position,
                                common::DiagnosticEngine& diagnostics, std::string_view context) {
    if (position >= text.size() || text[position] != '{') {
        diagnostics.error(std::string(context) + " is missing an opening '{'");
        return {};
    }

    const auto content_start = position + 1;
    int depth = 1;
    ++position;

    while (position < text.size()) {
        const auto ch = text[position];
        if (ch == '\'' || ch == '"') {
            skip_quoted(text, position, ch);
            continue;
        }
        if (starts_with_at(text, position, "//")) {
            position += 2;
            while (position < text.size() && text[position] != '\n') {
                ++position;
            }
            continue;
        }
        if (starts_with_at(text, position, "/*")) {
            position += 2;
            while (position + 1 < text.size() && !starts_with_at(text, position, "*/")) {
                ++position;
            }
            if (position + 1 < text.size()) {
                position += 2;
            } else {
                diagnostics.error("unterminated block comment while parsing " + std::string(context));
                return {};
            }
            continue;
        }
        if (ch == '{') {
            ++depth;
            ++position;
            continue;
        }
        if (ch == '}') {
            --depth;
            if (depth == 0) {
                const auto content_end = position;
                ++position;
                return std::string(text.substr(content_start, content_end - content_start));
            }
            ++position;
            continue;
        }
        ++position;
    }

    diagnostics.error("unterminated brace block while parsing " + std::string(context));
    return {};
}

std::vector<std::string> read_symbols_until_eol(std::string_view text, std::size_t& position,
                                                common::DiagnosticEngine& diagnostics) {
    std::vector<std::string> symbols;
    while (true) {
        skip_space_and_comments(text, position, false);
        if (position >= text.size() || text[position] == '\n' || text[position] == '\r') {
            break;
        }

        auto symbol = read_symbol(text, position);
        if (symbol.empty()) {
            diagnostics.error("failed to parse symbol in yacc declaration");
            break;
        }
        symbols.push_back(std::move(symbol));
    }
    consume_line_end(text, position);
    return symbols;
}

void skip_to_next_line(std::string_view text, std::size_t& position) {
    while (position < text.size() && text[position] != '\n' && text[position] != '\r') {
        ++position;
    }
    consume_line_end(text, position);
}

void parse_declarations(std::string_view declarations_text, YaccSpec& spec,
                        common::DiagnosticEngine& diagnostics) {
    std::size_t position = 0;
    while (position < declarations_text.size()) {
        skip_space_and_comments(declarations_text, position, true);
        if (position >= declarations_text.size()) {
            break;
        }

        if (declarations_text[position] != '%') {
            diagnostics.warning("unsupported text in yacc declarations ignored");
            skip_to_next_line(declarations_text, position);
            continue;
        }

        ++position;
        const auto keyword = read_identifier(declarations_text, position);
        if (keyword.empty()) {
            diagnostics.error("expected yacc declaration keyword after '%'");
            skip_to_next_line(declarations_text, position);
            continue;
        }

        if (keyword == "token" || keyword == "type") {
            std::string type_tag;
            skip_space_and_comments(declarations_text, position, false);
            if (position < declarations_text.size() && declarations_text[position] == '<') {
                type_tag = read_angle_block(declarations_text, position, diagnostics, "%" + keyword);
            }

            const auto symbols = read_symbols_until_eol(declarations_text, position, diagnostics);
            if (keyword == "token") {
                for (const auto& symbol : symbols) {
                    spec.tokens.push_back({symbol, type_tag});
                }
            } else if (!symbols.empty()) {
                spec.typed_symbols.push_back({type_tag, symbols});
            }
            continue;
        }

        if (keyword == "start") {
            const auto symbols = read_symbols_until_eol(declarations_text, position, diagnostics);
            if (symbols.size() != 1) {
                diagnostics.error("%start requires exactly one symbol name");
            } else {
                spec.start_symbol = symbols.front();
            }
            continue;
        }

        if (keyword == "left" || keyword == "right" || keyword == "nonassoc") {
            PrecedenceDecl precedence;
            if (keyword == "left") {
                precedence.assoc = Assoc::Left;
            } else if (keyword == "right") {
                precedence.assoc = Assoc::Right;
            } else {
                precedence.assoc = Assoc::None;
            }
            precedence.tokens = read_symbols_until_eol(declarations_text, position, diagnostics);
            spec.precedences.push_back(std::move(precedence));
            continue;
        }

        if (keyword == "union") {
            skip_space_and_comments(declarations_text, position, true);
            const auto union_text = read_balanced_block(declarations_text, position, diagnostics, "%union");
            if (!union_text.empty()) {
                spec.union_text = common::trim(union_text);
            }
            skip_space_and_comments(declarations_text, position, true);
            continue;
        }

        diagnostics.warning("unsupported yacc declaration ignored: %" + keyword);
        skip_to_next_line(declarations_text, position);
    }
}

void push_production(const std::string& lhs, std::vector<std::string> rhs, std::string action_text,
                     std::string precedence_token, YaccSpec& spec,
                     common::DiagnosticEngine& diagnostics) {
    if (rhs.size() > 1) {
        for (const auto& symbol : rhs) {
            if (symbol == "%empty") {
                diagnostics.error("%empty cannot appear alongside other right-hand-side symbols in production: " +
                                  lhs);
                rhs.clear();
                break;
            }
        }
    }

    if (rhs.size() == 1 && rhs.front() == "%empty") {
        rhs.clear();
    }

    Production production;
    production.lhs = lhs;
    production.rhs = std::move(rhs);
    production.action_text = common::trim(action_text);
    production.precedence_token = std::move(precedence_token);
    spec.productions.push_back(std::move(production));
}

void parse_rules(std::string rules_text, YaccSpec& spec, common::DiagnosticEngine& diagnostics) {
    rules_text = replace_all(std::move(rules_text), "/* empty */", "%empty");

    std::size_t position = 0;
    while (position < rules_text.size()) {
        skip_space_and_comments(rules_text, position, true);
        if (position >= rules_text.size()) {
            break;
        }

        auto lhs = read_symbol(rules_text, position);
        if (lhs.empty()) {
            diagnostics.error("expected nonterminal on the left-hand side of a production");
            return;
        }

        skip_space_and_comments(rules_text, position, true);
        if (position >= rules_text.size() || rules_text[position] != ':') {
            diagnostics.error("expected ':' after production head `" + lhs + "`");
            return;
        }
        ++position;

        bool done_with_rule = false;
        while (!done_with_rule) {
            std::vector<std::string> rhs;
            std::string action_text;
            std::string precedence_token;
            bool action_seen = false;

            while (true) {
                skip_space_and_comments(rules_text, position, true);
                if (position >= rules_text.size()) {
                    diagnostics.error("unterminated production list for nonterminal: " + lhs);
                    return;
                }

                const auto ch = rules_text[position];
                if (ch == '|' || ch == ';') {
                    push_production(lhs, std::move(rhs), std::move(action_text),
                                    std::move(precedence_token), spec, diagnostics);
                    ++position;
                    done_with_rule = (ch == ';');
                    break;
                }

                if (action_seen) {
                    diagnostics.error("mid-rule semantic actions are not supported in the current SeuYacc milestone");
                    return;
                }

                if (starts_with_at(rules_text, position, "%prec") &&
                    (position + 5 == rules_text.size() ||
                     !common::is_identifier_continue(rules_text[position + 5]))) {
                    position += 5;
                    skip_space_and_comments(rules_text, position, true);
                    precedence_token = read_symbol(rules_text, position);
                    if (precedence_token.empty()) {
                        diagnostics.error("missing precedence token after %prec in production: " + lhs);
                        return;
                    }
                    continue;
                }

                if (ch == '{') {
                    action_text = read_balanced_block(rules_text, position, diagnostics, "semantic action");
                    action_seen = true;
                    continue;
                }

                auto symbol = read_symbol(rules_text, position);
                if (symbol.empty()) {
                    diagnostics.error("failed to parse production body for `" + lhs + "`");
                    return;
                }
                rhs.push_back(std::move(symbol));
            }
        }
    }
}

}  // namespace

YaccSpec YaccSpecParser::parse_file(const std::string& path, common::SourceManager& sources,
                                    common::DiagnosticEngine& diagnostics) const {
    YaccSpec spec;

    try {
        const auto file = sources.load_file(path);
        const auto sections = split_sections(file->content(), diagnostics);
        if (diagnostics.has_error()) {
            return spec;
        }

        auto declarations_text = sections.declarations;
        const auto prologue_start = declarations_text.find("%{");
        const auto prologue_end = declarations_text.find("%}");
        if (prologue_start != std::string::npos && prologue_end != std::string::npos &&
            prologue_start < prologue_end) {
            spec.prologue = declarations_text.substr(prologue_start + 2, prologue_end - prologue_start - 2);
            declarations_text.erase(prologue_start, prologue_end - prologue_start + 2);
        }

        parse_declarations(declarations_text, spec, diagnostics);
        parse_rules(sections.rules, spec, diagnostics);
        spec.epilogue = sections.epilogue;

        if (spec.start_symbol.empty() && !spec.productions.empty()) {
            spec.start_symbol = spec.productions.front().lhs;
        }
    } catch (const std::exception& ex) {
        diagnostics.error(ex.what());
    }

    return spec;
}

}  // namespace compilerlab::seuyacc
