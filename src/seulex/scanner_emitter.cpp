#include "seulex/scanner_emitter.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace compilerlab::seulex {

namespace {

std::string sanitize_identifier(std::string text) {
    if (text.empty()) {
        return "scanner";
    }

    for (char& ch : text) {
        if (std::isalnum(static_cast<unsigned char>(ch)) == 0 && ch != '_') {
            ch = '_';
        }
    }

    if (std::isdigit(static_cast<unsigned char>(text.front())) != 0) {
        text.insert(text.begin(), '_');
    }

    return text;
}

std::string to_pascal_case(std::string_view text) {
    std::string result;
    bool uppercase_next = true;

    for (char ch : text) {
        if (std::isalnum(static_cast<unsigned char>(ch)) == 0) {
            uppercase_next = true;
            continue;
        }

        if (uppercase_next) {
            result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
            uppercase_next = false;
        } else {
            result.push_back(ch);
        }
    }

    if (result.empty()) {
        return "Scanner";
    }

    if (std::isdigit(static_cast<unsigned char>(result.front())) != 0) {
        result.insert(result.begin(), '_');
    }

    return result;
}

const std::vector<std::string>& token_kind_names() {
    static const std::vector<std::string> names {
        "Invalid",
        "EndOfFile",
        "Identifier",
        "IntegerLiteral",
        "FloatLiteral",
        "KwInt",
        "KwFloat",
        "KwVoid",
        "KwIf",
        "KwElse",
        "KwWhile",
        "KwReturn",
        "LParen",
        "RParen",
        "LBrace",
        "RBrace",
        "Comma",
        "Semicolon",
        "Plus",
        "Minus",
        "Star",
        "Slash",
        "Percent",
        "Assign",
        "Equal",
        "NotEqual",
        "Less",
        "LessEqual",
        "Greater",
        "GreaterEqual",
        "LogicalAnd",
        "LogicalOr",
        "LogicalNot",
    };
    return names;
}

std::string escape_char(unsigned char ch) {
    switch (ch) {
        case 0: return "\\x00";
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\t': return "\\t";
        case ' ': return "space";
        case '\\': return "\\\\";
        case '"': return "\\\"";
        default:
            if (ch < 32 || ch > 126) {
                constexpr char hex[] = "0123456789ABCDEF";
                std::string value = "\\x";
                value.push_back(hex[(ch >> 4) & 0xF]);
                value.push_back(hex[ch & 0xF]);
                return value;
            }
            return std::string(1, static_cast<char>(ch));
    }
}

std::string cpp_char_literal(unsigned char ch) {
    switch (ch) {
        case '\n': return "'\\n'";
        case '\r': return "'\\r'";
        case '\t': return "'\\t'";
        case '\\': return "'\\\\'";
        case '\'': return "'\\''";
        default:
            if (ch >= 32 && ch <= 126) {
                return std::string("'") + static_cast<char>(ch) + "'";
            }

            constexpr char hex[] = "0123456789ABCDEF";
            std::string value = "static_cast<char>(0x";
            value.push_back(hex[(ch >> 4) & 0xF]);
            value.push_back(hex[ch & 0xF]);
            value += ")";
            return value;
    }
}

std::string format_char_group(const std::set<unsigned char>& chars) {
    std::ostringstream output;
    bool first = true;
    auto it = chars.begin();
    while (it != chars.end()) {
        auto range_start = *it;
        auto range_end = *it;
        auto next = std::next(it);
        while (next != chars.end() && *next == static_cast<unsigned char>(range_end + 1)) {
            range_end = *next;
            ++next;
        }

        if (!first) {
            output << ",";
        }
        first = false;

        if (range_start == range_end) {
            output << escape_char(range_start);
        } else {
            output << escape_char(range_start) << "-" << escape_char(range_end);
        }

        it = next;
    }
    return output.str();
}

std::string generated_token_kind_expr(common::TokenKind kind) {
    return "TokenKind::" + common::to_string(kind);
}

std::vector<std::pair<unsigned char, int>> sorted_transitions(const DFA& dfa, int state_id) {
    std::vector<std::pair<unsigned char, int>> transitions;
    for (const auto& transition : dfa.transitions) {
        if (transition.from == state_id) {
            transitions.push_back({static_cast<unsigned char>(transition.symbol), transition.to});
        }
    }

    std::sort(transitions.begin(), transitions.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.first < rhs.first;
    });
    return transitions;
}

bool write_scanner_header(const std::filesystem::path& path, std::string_view class_name,
                          common::DiagnosticEngine& diagnostics) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        diagnostics.error("failed to open generated scanner header: " + path.string());
        return false;
    }

    out << "#pragma once\n\n";
    out << "#include <cstddef>\n";
    out << "#include <optional>\n";
    out << "#include <string>\n";
    out << "#include <string_view>\n";
    out << "#include <utility>\n";
    out << "#include <vector>\n\n";
    out << "namespace compilerlab::generated {\n\n";
    out << "struct SourceLocation {\n";
    out << "    std::size_t offset {0};\n";
    out << "    std::size_t line {1};\n";
    out << "    std::size_t column {1};\n";
    out << "};\n\n";
    out << "struct SourceSpan {\n";
    out << "    std::string file_path;\n";
    out << "    SourceLocation begin {};\n";
    out << "    SourceLocation end {};\n";
    out << "};\n\n";
    out << "enum class TokenKind {\n";
    const auto& names = token_kind_names();
    for (std::size_t index = 0; index < names.size(); ++index) {
        out << "    " << names[index];
        if (index + 1 != names.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "};\n\n";
    out << "struct Token {\n";
    out << "    TokenKind kind {TokenKind::Invalid};\n";
    out << "    std::string lexeme;\n";
    out << "    SourceSpan span {};\n\n";
    out << "    bool is(TokenKind expected) const {\n";
    out << "        return kind == expected;\n";
    out << "    }\n";
    out << "};\n\n";
    out << "std::string to_string(TokenKind kind);\n";
    out << "std::string escape_text(std::string_view text);\n\n";
    out << "class " << class_name << " {\n";
    out << "public:\n";
    out << "    explicit " << class_name << "(std::string file_path = {});\n\n";
    out << "    Token next_token(std::string_view input, std::size_t& offset) const;\n";
    out << "    std::vector<Token> scan_all(std::string_view input) const;\n\n";
    out << "private:\n";
    out << "    struct ScannerAcceptAction {\n";
    out << "        TokenKind kind {TokenKind::Invalid};\n";
    out << "        std::size_t rule_priority {0};\n";
    out << "        bool skip {false};\n";
    out << "    };\n\n";
    out << "    struct ScannerState {\n";
    out << "        int id {0};\n";
    out << "        std::optional<ScannerAcceptAction> accept;\n";
    out << "        std::vector<std::pair<char, int>> transitions;\n";
    out << "    };\n\n";
    out << "    static std::vector<ScannerState> build_states();\n";
    out << "    static SourceLocation locate(std::string_view input, std::size_t offset);\n\n";
    out << "    std::vector<ScannerState> states_;\n";
    out << "    std::string file_path_;\n";
    out << "};\n\n";
    out << "}  // namespace compilerlab::generated\n";
    return true;
}

bool write_scanner_source(const std::filesystem::path& path, std::string_view header_name,
                          std::string_view class_name, const DFA& dfa,
                          const std::vector<LexRule>& rules, common::DiagnosticEngine& diagnostics) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        diagnostics.error("failed to open generated scanner source: " + path.string());
        return false;
    }

    out << "#include \"" << header_name << "\"\n\n";
    out << "#include <algorithm>\n";
    out << "#include <optional>\n";
    out << "#include <utility>\n";
    out << "#include <vector>\n\n";
    out << "namespace compilerlab::generated {\n\n";
    out << "std::string to_string(TokenKind kind) {\n";
    out << "    switch (kind) {\n";
    for (const auto& name : token_kind_names()) {
        out << "        case TokenKind::" << name << ": return \"" << name << "\";\n";
    }
    out << "    }\n";
    out << "    return \"UnknownTokenKind\";\n";
    out << "}\n\n";
    out << "std::string escape_text(std::string_view text) {\n";
    out << "    std::string escaped;\n";
    out << "    escaped.reserve(text.size());\n";
    out << "    for (char ch : text) {\n";
    out << "        switch (ch) {\n";
    out << "            case '\\n': escaped += \"\\\\n\"; break;\n";
    out << "            case '\\r': escaped += \"\\\\r\"; break;\n";
    out << "            case '\\t': escaped += \"\\\\t\"; break;\n";
    out << "            case '\\\\': escaped += \"\\\\\\\\\"; break;\n";
    out << "            default: escaped.push_back(ch); break;\n";
    out << "        }\n";
    out << "    }\n";
    out << "    return escaped;\n";
    out << "}\n\n";
    out << "SourceLocation " << class_name << "::locate(std::string_view input, std::size_t offset) {\n";
    out << "    SourceLocation location;\n";
    out << "    location.offset = offset;\n";
    out << "    location.line = 1;\n";
    out << "    location.column = 1;\n\n";
    out << "    std::size_t start = 0;\n";
    out << "    if (input.size() >= 3 &&\n";
    out << "        static_cast<unsigned char>(input[0]) == 0xEF &&\n";
    out << "        static_cast<unsigned char>(input[1]) == 0xBB &&\n";
    out << "        static_cast<unsigned char>(input[2]) == 0xBF) {\n";
    out << "        start = std::min<std::size_t>(3, offset);\n";
    out << "    }\n\n";
    out << "    for (std::size_t index = start; index < offset && index < input.size(); ++index) {\n";
    out << "        if (input[index] == '\\n') {\n";
    out << "            ++location.line;\n";
    out << "            location.column = 1;\n";
    out << "        } else {\n";
    out << "            ++location.column;\n";
    out << "        }\n";
    out << "    }\n";
    out << "    return location;\n";
    out << "}\n\n";
    out << "std::vector<" << class_name << "::ScannerState> " << class_name << "::build_states() {\n";
    out << "    std::vector<ScannerState> states;\n";
    out << "    states.reserve(" << dfa.states.size() << ");\n\n";

    for (const auto& state : dfa.states) {
        out << "    states.push_back(ScannerState{";
        out << state.id << ", ";
        if (state.accepting) {
            if (state.rule_priority >= rules.size()) {
                diagnostics.error("accepting DFA state references an out-of-range lex rule");
                return false;
            }
            const auto& rule = rules[state.rule_priority];
            out << "ScannerAcceptAction{"
                << generated_token_kind_expr(rule.token_kind) << ", "
                << rule.priority << ", "
                << (rule.action_kind == LexActionKind::Skip ? "true" : "false")
                << "}";
        } else {
            out << "std::nullopt";
        }
        out << ", {";

        const auto transitions = sorted_transitions(dfa, state.id);
        for (std::size_t index = 0; index < transitions.size(); ++index) {
            if (index > 0) {
                out << ", ";
            }
            out << "{" << cpp_char_literal(transitions[index].first)
                << ", " << transitions[index].second << "}";
        }
        out << "}});\n";
    }

    out << "\n    return states;\n";
    out << "}\n\n";
    out << class_name << "::" << class_name << "(std::string file_path)\n";
    out << "    : states_(build_states()), file_path_(std::move(file_path)) {\n";
    out << "}\n\n";
    out << "Token " << class_name << "::next_token(std::string_view input, std::size_t& offset) const {\n";
    out << "    while (true) {\n";
    out << "        if (offset == 0 && input.size() >= 3 &&\n";
    out << "            static_cast<unsigned char>(input[0]) == 0xEF &&\n";
    out << "            static_cast<unsigned char>(input[1]) == 0xBB &&\n";
    out << "            static_cast<unsigned char>(input[2]) == 0xBF) {\n";
    out << "            offset = 3;\n";
    out << "        }\n\n";
    out << "        Token token;\n";
    out << "        token.span.file_path = file_path_;\n";
    out << "        token.span.begin = locate(input, offset);\n";
    out << "        token.span.end = token.span.begin;\n\n";
    out << "        if (offset >= input.size()) {\n";
    out << "            token.kind = TokenKind::EndOfFile;\n";
    out << "            return token;\n";
    out << "        }\n\n";
    out << "        if (states_.empty()) {\n";
    out << "            token.kind = TokenKind::Invalid;\n";
    out << "            token.lexeme.assign(1, input[offset]);\n";
    out << "            ++offset;\n";
    out << "            token.span.end = locate(input, offset);\n";
    out << "            return token;\n";
    out << "        }\n\n";
    out << "        struct AcceptSnapshot {\n";
    out << "            ScannerAcceptAction action;\n";
    out << "            std::size_t end_offset {0};\n";
    out << "        };\n\n";
    out << "        int current_state = 0;\n";
    out << "        std::size_t cursor = offset;\n";
    out << "        std::optional<AcceptSnapshot> last_accept;\n\n";
    out << "        if (states_[0].accept.has_value()) {\n";
    out << "            last_accept = AcceptSnapshot{*states_[0].accept, offset};\n";
    out << "        }\n\n";
    out << "        while (cursor < input.size()) {\n";
    out << "            const auto& transitions = states_[static_cast<std::size_t>(current_state)].transitions;\n";
    out << "            const auto transition = std::find_if(transitions.begin(), transitions.end(),\n";
    out << "                [&](const auto& item) { return item.first == input[cursor]; });\n";
    out << "            if (transition == transitions.end()) {\n";
    out << "                break;\n";
    out << "            }\n\n";
    out << "            current_state = transition->second;\n";
    out << "            ++cursor;\n\n";
    out << "            const auto& state = states_[static_cast<std::size_t>(current_state)];\n";
    out << "            if (state.accept.has_value()) {\n";
    out << "                last_accept = AcceptSnapshot{*state.accept, cursor};\n";
    out << "            }\n";
    out << "        }\n\n";
    out << "        if (!last_accept.has_value()) {\n";
    out << "            token.kind = TokenKind::Invalid;\n";
    out << "            token.lexeme.assign(1, input[offset]);\n";
    out << "            ++offset;\n";
    out << "            token.span.end = locate(input, offset);\n";
    out << "            return token;\n";
    out << "        }\n\n";
    out << "        token.kind = last_accept->action.kind;\n";
    out << "        token.lexeme = std::string(input.substr(offset, last_accept->end_offset - offset));\n";
    out << "        offset = last_accept->end_offset;\n";
    out << "        token.span.end = locate(input, offset);\n\n";
    out << "        if (last_accept->action.skip) {\n";
    out << "            continue;\n";
    out << "        }\n\n";
    out << "        return token;\n";
    out << "    }\n";
    out << "}\n\n";
    out << "std::vector<Token> " << class_name << "::scan_all(std::string_view input) const {\n";
    out << "    std::vector<Token> tokens;\n";
    out << "    std::size_t offset = 0;\n";
    out << "    while (true) {\n";
    out << "        auto token = next_token(input, offset);\n";
    out << "        tokens.push_back(token);\n";
    out << "        if (token.kind == TokenKind::EndOfFile || token.kind == TokenKind::Invalid) {\n";
    out << "            break;\n";
    out << "        }\n";
    out << "    }\n";
    out << "    return tokens;\n";
    out << "}\n\n";
    out << "}  // namespace compilerlab::generated\n";
    return true;
}

bool write_scanner_main(const std::filesystem::path& path, std::string_view header_name,
                        std::string_view class_name, std::string_view scanner_name,
                        common::DiagnosticEngine& diagnostics) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        diagnostics.error("failed to open generated scanner main: " + path.string());
        return false;
    }

    out << "#include \"" << header_name << "\"\n\n";
    out << "#include <exception>\n";
    out << "#include <fstream>\n";
    out << "#include <iostream>\n";
    out << "#include <sstream>\n";
    out << "#include <stdexcept>\n";
    out << "#include <string>\n\n";
    out << "namespace {\n\n";
    out << "std::string read_file_or_die(const std::string& path) {\n";
    out << "    std::ifstream input(path, std::ios::binary);\n";
    out << "    if (!input) {\n";
    out << "        throw std::runtime_error(\"failed to open input file: \" + path);\n";
    out << "    }\n";
    out << "    std::ostringstream buffer;\n";
    out << "    buffer << input.rdbuf();\n";
    out << "    return buffer.str();\n";
    out << "}\n\n";
    out << "}  // namespace\n\n";
    out << "int main(int argc, char** argv) {\n";
    out << "    try {\n";
    out << "        if (argc < 2) {\n";
    out << "            std::cerr << \"usage: " << scanner_name << "_scanner <input-file>\\n\";\n";
    out << "            return 1;\n";
    out << "        }\n\n";
    out << "        const std::string input_path = argv[1];\n";
    out << "        const auto input = read_file_or_die(input_path);\n";
    out << "        compilerlab::generated::" << class_name << " scanner(input_path);\n";
    out << "        std::size_t offset = 0;\n";
    out << "        while (true) {\n";
    out << "            auto token = scanner.next_token(input, offset);\n";
    out << "            if (token.kind == compilerlab::generated::TokenKind::Invalid) {\n";
    out << "                std::cerr << \"error: invalid token starting at \"\n";
    out << "                          << token.span.begin.line << \":\" << token.span.begin.column\n";
    out << "                          << \" -> `\" << compilerlab::generated::escape_text(token.lexeme) << \"`\\n\";\n";
    out << "                return 1;\n";
    out << "            }\n";
    out << "            if (token.kind == compilerlab::generated::TokenKind::EndOfFile) {\n";
    out << "                std::cout << \"EndOfFile\\n\";\n";
    out << "                break;\n";
    out << "            }\n";
    out << "            std::cout << compilerlab::generated::to_string(token.kind)\n";
    out << "                      << \" \\\"\" << compilerlab::generated::escape_text(token.lexeme) << \"\\\"\"\n";
    out << "                      << \" @\" << token.span.begin.line << \":\" << token.span.begin.column\n";
    out << "                      << '\\n';\n";
    out << "        }\n";
    out << "        return 0;\n";
    out << "    } catch (const std::exception& ex) {\n";
    out << "        std::cerr << \"fatal: \" << ex.what() << '\\n';\n";
    out << "        return 1;\n";
    out << "    }\n";
    out << "}\n";
    return true;
}

bool write_generated_cmake(const std::filesystem::path& path, std::string_view scanner_name,
                           common::DiagnosticEngine& diagnostics) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        diagnostics.error("failed to open generated CMakeLists.txt: " + path.string());
        return false;
    }

    out << "cmake_minimum_required(VERSION 3.20)\n\n";
    out << "project(" << scanner_name << "_generated_scanner LANGUAGES CXX)\n\n";
    out << "set(CMAKE_CXX_STANDARD 17)\n";
    out << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    out << "set(CMAKE_CXX_EXTENSIONS OFF)\n\n";
    out << "add_executable(" << scanner_name << "_scanner\n";
    out << "    " << scanner_name << "_scanner.cpp\n";
    out << "    " << scanner_name << "_scanner_main.cpp\n";
    out << ")\n";
    return true;
}

}  // namespace

bool ScannerEmitter::emit(const DFA& dfa, const std::vector<LexRule>& rules, const std::string& output_dir,
                          const std::string& scanner_name, common::DiagnosticEngine& diagnostics) const {
    if (output_dir.empty()) {
        diagnostics.error("scanner output directory is empty");
        return false;
    }
    if (scanner_name.empty()) {
        diagnostics.error("scanner output name is empty");
        return false;
    }

    std::error_code error;
    std::filesystem::create_directories(output_dir, error);
    if (error) {
        diagnostics.error("failed to create scanner output directory: " + output_dir);
        return false;
    }

    const auto safe_name = sanitize_identifier(scanner_name);
    const auto class_name = to_pascal_case(safe_name) + "Scanner";
    const auto header_path = std::filesystem::path(output_dir) / (safe_name + "_scanner.h");
    const auto source_path = std::filesystem::path(output_dir) / (safe_name + "_scanner.cpp");
    const auto main_path = std::filesystem::path(output_dir) / (safe_name + "_scanner_main.cpp");
    const auto cmake_path = std::filesystem::path(output_dir) / "CMakeLists.txt";
    const auto dot_path = std::filesystem::path(output_dir) / "dfa.dot";

    std::ofstream dot_out(dot_path, std::ios::binary);
    if (!dot_out) {
        diagnostics.error("failed to open DFA dot output file: " + dot_path.string());
        return false;
    }

    std::map<std::pair<int, int>, std::set<unsigned char>> grouped_transitions;
    for (const auto& transition : dfa.transitions) {
        grouped_transitions[{transition.from, transition.to}].insert(
            static_cast<unsigned char>(transition.symbol));
    }

    dot_out << "digraph DFA {\n";
    dot_out << "  rankdir=LR;\n";
    dot_out << "  node [shape=circle];\n";
    dot_out << "  start [shape=point];\n";
    dot_out << "  start -> q" << dfa.start_state << ";\n";

    for (const auto& state : dfa.states) {
        dot_out << "  q" << state.id << " [shape=" << (state.accepting ? "doublecircle" : "circle")
                << ",label=\"q" << state.id;
        if (state.accepting) {
            dot_out << "\\nrule=" << state.rule_priority;
        }
        dot_out << "\"];\n";
    }

    for (const auto& [edge, chars] : grouped_transitions) {
        dot_out << "  q" << edge.first << " -> q" << edge.second
                << " [label=\"" << format_char_group(chars) << "\"];\n";
    }

    dot_out << "}\n";

    if (!write_scanner_header(header_path, class_name, diagnostics)) {
        return false;
    }
    if (!write_scanner_source(source_path, header_path.filename().string(), class_name, dfa, rules, diagnostics)) {
        return false;
    }
    if (!write_scanner_main(main_path, header_path.filename().string(), class_name, safe_name, diagnostics)) {
        return false;
    }
    if (!write_generated_cmake(cmake_path, safe_name, diagnostics)) {
        return false;
    }

    return true;
}

}  // namespace compilerlab::seulex
