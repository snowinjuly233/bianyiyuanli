#include "seuyacc/parser_emitter.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace compilerlab::seuyacc {

namespace {

std::string sanitize_identifier(std::string text) {
    if (text.empty()) {
        return "parser";
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
        return "Parser";
    }
    if (std::isdigit(static_cast<unsigned char>(result.front())) != 0) {
        result.insert(result.begin(), '_');
    }
    return result;
}

std::string escape_cpp_string(std::string_view text) {
    std::string escaped;
    escaped.reserve(text.size());
    for (const char ch : text) {
        switch (ch) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped.push_back(ch); break;
        }
    }
    return escaped;
}

std::string generated_token_kind_expr(common::TokenKind kind) {
    return "TokenKind::" + common::to_string(kind);
}

std::string common_token_kind_expr(common::TokenKind kind) {
    return "compilerlab::common::TokenKind::" + common::to_string(kind);
}

std::string action_kind_expr(runtime::ParserActionKind kind) {
    switch (kind) {
        case runtime::ParserActionKind::Error: return "compilerlab::runtime::ParserActionKind::Error";
        case runtime::ParserActionKind::Shift: return "compilerlab::runtime::ParserActionKind::Shift";
        case runtime::ParserActionKind::Reduce: return "compilerlab::runtime::ParserActionKind::Reduce";
        case runtime::ParserActionKind::Accept: return "compilerlab::runtime::ParserActionKind::Accept";
    }
    return "compilerlab::runtime::ParserActionKind::Error";
}

bool write_parser_header(const std::filesystem::path& path, std::string_view parser_class_name,
                         std::string_view scanner_header_name, std::string_view scanner_class_name,
                         common::DiagnosticEngine& diagnostics) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        diagnostics.error("failed to open generated parser header: " + path.string());
        return false;
    }

    out << "#pragma once\n\n";
    out << "#include <cstddef>\n";
    out << "#include <string>\n";
    out << "#include <string_view>\n";
    out << "#include <vector>\n\n";
    out << "#include \"semantic/ast.h\"\n";
    out << "#include \"" << scanner_header_name << "\"\n\n";
    out << "namespace compilerlab::generated {\n\n";
    out << "struct ParseResult {\n";
    out << "    bool accepted {false};\n";
    out << "    std::vector<std::string> reductions;\n";
    out << "    std::string error_message;\n";
    out << "    SourceSpan error_span {};\n";
    out << "    compilerlab::semantic::ProgramPtr program;\n";
    out << "};\n\n";
    out << "class " << parser_class_name << " {\n";
    out << "public:\n";
    out << "    ParseResult parse(const std::vector<Token>& tokens) const;\n";
    out << "    ParseResult parse_source(std::string_view input, const " << scanner_class_name
        << "& scanner) const;\n\n";
    out << "    static std::size_t lr1_state_count();\n";
    out << "    static std::size_t lalr_state_count();\n";
    out << "};\n\n";
    out << "}  // namespace compilerlab::generated\n";
    return true;
}

bool write_parser_source(const std::filesystem::path& path, std::string_view header_name,
                         std::string_view parser_class_name, std::string_view scanner_class_name,
                         const ParseTable& table,
                         const std::vector<runtime::ProductionInfo>& productions,
                         const std::unordered_map<int, SymbolId>& token_symbols,
                         std::size_t lr1_state_count, std::size_t lalr_state_count,
                         common::DiagnosticEngine& diagnostics) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        diagnostics.error("failed to open generated parser source: " + path.string());
        return false;
    }

    out << "#include \"" << header_name << "\"\n\n";
    out << "#include \"common/diagnostic.h\"\n";
    out << "#include \"common/source.h\"\n";
    out << "#include \"common/token.h\"\n";
    out << "#include \"runtime/parser_runtime.h\"\n";
    out << "#include \"semantic/semantic_action_executor.h\"\n\n";
    out << "#include <string>\n";
    out << "#include <unordered_map>\n";
    out << "#include <utility>\n";
    out << "#include <vector>\n\n";
    out << "namespace compilerlab::generated {\n\n";
    out << "namespace {\n\n";
    out << "using ActionTable = std::vector<std::unordered_map<int, compilerlab::runtime::ParserAction>>;\n";
    out << "using GotoTable = std::vector<std::unordered_map<int, int>>;\n\n";

    out << "compilerlab::common::SourceLocation to_common_location(const SourceLocation& location) {\n";
    out << "    return compilerlab::common::SourceLocation{location.offset, location.line, location.column};\n";
    out << "}\n\n";

    out << "compilerlab::common::SourceSpan to_common_span(const SourceSpan& span) {\n";
    out << "    compilerlab::common::SourceSpan converted;\n";
    out << "    converted.file_path = span.file_path;\n";
    out << "    converted.begin = to_common_location(span.begin);\n";
    out << "    converted.end = to_common_location(span.end);\n";
    out << "    return converted;\n";
    out << "}\n\n";

    out << "SourceLocation to_generated_location(const compilerlab::common::SourceLocation& location) {\n";
    out << "    return SourceLocation{location.offset, location.line, location.column};\n";
    out << "}\n\n";

    out << "SourceSpan to_generated_span(const compilerlab::common::SourceSpan& span) {\n";
    out << "    SourceSpan converted;\n";
    out << "    converted.file_path = span.file_path;\n";
    out << "    converted.begin = to_generated_location(span.begin);\n";
    out << "    converted.end = to_generated_location(span.end);\n";
    out << "    return converted;\n";
    out << "}\n\n";

    out << "compilerlab::common::TokenKind to_common_token_kind(TokenKind kind) {\n";
    out << "    switch (kind) {\n";
    for (const auto& [token_kind_value, _] : token_symbols) {
        const auto token_kind = static_cast<common::TokenKind>(token_kind_value);
        out << "        case " << generated_token_kind_expr(token_kind) << ": return "
            << common_token_kind_expr(token_kind) << ";\n";
    }
    out << "        default: return compilerlab::common::TokenKind::Invalid;\n";
    out << "    }\n";
    out << "}\n\n";

    out << "compilerlab::common::Token to_common_token(const Token& token) {\n";
    out << "    compilerlab::common::Token converted;\n";
    out << "    converted.kind = to_common_token_kind(token.kind);\n";
    out << "    converted.lexeme = token.lexeme;\n";
    out << "    converted.span = to_common_span(token.span);\n";
    out << "    return converted;\n";
    out << "}\n\n";

    out << "int token_to_symbol(compilerlab::common::TokenKind kind) {\n";
    out << "    switch (kind) {\n";
    for (const auto& [token_kind_value, symbol_id] : token_symbols) {
        const auto token_kind = static_cast<common::TokenKind>(token_kind_value);
        out << "        case " << common_token_kind_expr(token_kind) << ": return " << symbol_id << ";\n";
    }
    out << "        default: return -1;\n";
    out << "    }\n";
    out << "}\n\n";

    out << "const ActionTable& action_table() {\n";
    out << "    static const ActionTable table = [] {\n";
    out << "        ActionTable value(" << table.actions.size() << ");\n";
    for (std::size_t state_id = 0; state_id < table.actions.size(); ++state_id) {
        for (const auto& [symbol, action] : table.actions[state_id]) {
            out << "        value[" << state_id << "].emplace(" << symbol
                << ", compilerlab::runtime::ParserAction{" << action_kind_expr(action.kind)
                << ", " << action.value << "});\n";
        }
    }
    out << "        return value;\n";
    out << "    }();\n";
    out << "    return table;\n";
    out << "}\n\n";

    out << "const GotoTable& goto_table() {\n";
    out << "    static const GotoTable table = [] {\n";
    out << "        GotoTable value(" << table.gotos.size() << ");\n";
    for (std::size_t state_id = 0; state_id < table.gotos.size(); ++state_id) {
        for (const auto& [symbol, target_state] : table.gotos[state_id]) {
            out << "        value[" << state_id << "].emplace(" << symbol << ", " << target_state << ");\n";
        }
    }
    out << "        return value;\n";
    out << "    }();\n";
    out << "    return table;\n";
    out << "}\n\n";

    out << "const std::vector<compilerlab::runtime::ProductionInfo>& productions() {\n";
    out << "    static const std::vector<compilerlab::runtime::ProductionInfo> value = {\n";
    for (const auto& production : productions) {
        out << "        compilerlab::runtime::ProductionInfo{" << production.lhs_symbol << ", "
            << production.rhs_size << ", \"" << escape_cpp_string(production.debug_name)
            << "\", \"" << escape_cpp_string(production.action_text) << "\"},\n";
    }
    out << "    };\n";
    out << "    return value;\n";
    out << "}\n\n";

    out << "void copy_first_error(const compilerlab::common::DiagnosticEngine& diagnostics, ParseResult& result) {\n";
    out << "    for (const auto& item : diagnostics.diagnostics()) {\n";
    out << "        if (item.severity == compilerlab::common::DiagnosticSeverity::Error) {\n";
    out << "            result.error_message = item.message;\n";
    out << "            result.error_span = to_generated_span(item.span);\n";
    out << "            return;\n";
    out << "        }\n";
    out << "    }\n";
    out << "}\n\n";
    out << "}  // namespace\n\n";

    out << "std::size_t " << parser_class_name << "::lr1_state_count() {\n";
    out << "    return " << lr1_state_count << ";\n";
    out << "}\n\n";
    out << "std::size_t " << parser_class_name << "::lalr_state_count() {\n";
    out << "    return " << lalr_state_count << ";\n";
    out << "}\n\n";

    out << "ParseResult " << parser_class_name << "::parse_source(std::string_view input, const "
        << scanner_class_name << "& scanner) const {\n";
    out << "    return parse(scanner.scan_all(input));\n";
    out << "}\n\n";

    out << "ParseResult " << parser_class_name << "::parse(const std::vector<Token>& tokens) const {\n";
    out << "    ParseResult result;\n";
    out << "    if (tokens.empty()) {\n";
    out << "        result.error_message = \"parser received an empty token stream\";\n";
    out << "        return result;\n";
    out << "    }\n\n";
    out << "    std::vector<compilerlab::common::Token> common_tokens;\n";
    out << "    common_tokens.reserve(tokens.size());\n";
    out << "    for (const auto& token : tokens) {\n";
    out << "        common_tokens.push_back(to_common_token(token));\n";
    out << "    }\n\n";
    out << "    compilerlab::common::DiagnosticEngine diagnostics;\n";
    out << "    compilerlab::runtime::ParserRuntime parser_runtime;\n";
    out << "    compilerlab::semantic::SemanticActionExecutor action_executor(diagnostics);\n";
    out << "    const auto& production_list = productions();\n";
    out << "    const auto semantic_result = parser_runtime.parse(\n";
    out << "        common_tokens,\n";
    out << "        action_table(),\n";
    out << "        goto_table(),\n";
    out << "        production_list,\n";
    out << "        [](compilerlab::common::TokenKind kind) {\n";
    out << "            return token_to_symbol(kind);\n";
    out << "        },\n";
    out << "        [&](int production_index, const std::vector<compilerlab::semantic::SemanticValue>& rhs_values) {\n";
    out << "            if (production_index > 0 &&\n";
    out << "                static_cast<std::size_t>(production_index) < production_list.size()) {\n";
    out << "                result.reductions.push_back(\n";
    out << "                    \"reduce \" + std::to_string(result.reductions.size() + 1) + \": \" +\n";
    out << "                    production_list[static_cast<std::size_t>(production_index)].debug_name);\n";
    out << "            }\n";
    out << "            return action_executor.execute(\n";
    out << "                production_list[static_cast<std::size_t>(production_index)], rhs_values);\n";
    out << "        },\n";
    out << "        diagnostics);\n\n";
    out << "    if (diagnostics.has_error()) {\n";
    out << "        copy_first_error(diagnostics, result);\n";
    out << "        return result;\n";
    out << "    }\n\n";
    out << "    if (const auto* program = semantic_result.get_if<compilerlab::semantic::ProgramPtr>();\n";
    out << "        program != nullptr) {\n";
    out << "        result.program = *program;\n";
    out << "    }\n";
    out << "    result.accepted = true;\n";
    out << "    return result;\n";
    out << "}\n\n";

    out << "}  // namespace compilerlab::generated\n";
    return true;
}

bool write_parser_main(const std::filesystem::path& path, std::string_view header_name,
                       std::string_view parser_class_name, std::string_view scanner_class_name,
                       std::string_view parser_name, common::DiagnosticEngine& diagnostics) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        diagnostics.error("failed to open generated parser main: " + path.string());
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
    out << "            std::cerr << \"usage: " << parser_name << "_parser <input-file>\\n\";\n";
    out << "            return 1;\n";
    out << "        }\n\n";
    out << "        const std::string input_path = argv[1];\n";
    out << "        const auto input = read_file_or_die(input_path);\n";
    out << "        compilerlab::generated::" << scanner_class_name << " scanner(input_path);\n";
    out << "        compilerlab::generated::" << parser_class_name << " parser;\n";
    out << "        std::cout << \"LR(1) states: \" << compilerlab::generated::" << parser_class_name
        << "::lr1_state_count() << '\\n';\n";
    out << "        std::cout << \"LALR(1) states: \" << compilerlab::generated::" << parser_class_name
        << "::lalr_state_count() << '\\n';\n";
    out << "        const auto result = parser.parse_source(input, scanner);\n";
    out << "        for (const auto& reduction : result.reductions) {\n";
    out << "            std::cout << reduction << '\\n';\n";
    out << "        }\n";
    out << "        if (result.accepted) {\n";
    out << "            if (result.program) {\n";
    out << "                std::cout << \"AST root: \"\n";
    out << "                          << compilerlab::semantic::to_string(result.program->kind)\n";
    out << "                          << '\\n';\n";
    out << "            }\n";
    out << "            std::cout << \"ACCEPT\\n\";\n";
    out << "            return 0;\n";
    out << "        }\n";
    out << "        std::cout << \"REJECT\\n\";\n";
    out << "        std::cerr << \"error: \" << result.error_message;\n";
    out << "        if (!result.error_span.file_path.empty()) {\n";
    out << "            std::cerr << \" (\" << result.error_span.file_path << \":\"\n";
    out << "                      << result.error_span.begin.line << \":\"\n";
    out << "                      << result.error_span.begin.column << \")\";\n";
    out << "        }\n";
    out << "        std::cerr << '\\n';\n";
    out << "        return 1;\n";
    out << "    } catch (const std::exception& ex) {\n";
    out << "        std::cerr << \"fatal: \" << ex.what() << '\\n';\n";
    out << "        return 1;\n";
    out << "    }\n";
    out << "}\n";
    return true;
}

bool write_generated_cmake(const std::filesystem::path& path, std::string_view parser_name,
                           std::string_view scanner_name, const std::filesystem::path& project_root,
                           common::DiagnosticEngine& diagnostics) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        diagnostics.error("failed to open generated CMakeLists.txt for parser output: " + path.string());
        return false;
    }

    auto relative_root = std::filesystem::relative(project_root, path.parent_path());
    if (relative_root.empty()) {
        relative_root = ".";
    }

    out << "cmake_minimum_required(VERSION 3.20)\n\n";
    out << "project(" << parser_name << "_generated_frontend LANGUAGES CXX)\n\n";
    out << "set(CMAKE_CXX_STANDARD 17)\n";
    out << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    out << "set(CMAKE_CXX_EXTENSIONS OFF)\n\n";
    out << "set(COMPILERLAB_ROOT \"" << escape_cpp_string(relative_root.generic_string()) << "\")\n\n";
    out << "add_library(compilerlab_common STATIC\n";
    out << "    ${COMPILERLAB_ROOT}/src/common/source.cpp\n";
    out << "    ${COMPILERLAB_ROOT}/src/common/token.cpp\n";
    out << "    ${COMPILERLAB_ROOT}/src/common/diagnostic.cpp\n";
    out << "    ${COMPILERLAB_ROOT}/src/common/string_utils.cpp\n";
    out << ")\n";
    out << "target_include_directories(compilerlab_common PUBLIC ${COMPILERLAB_ROOT}/src)\n\n";
    out << "add_library(compilerlab_runtime STATIC\n";
    out << "    ${COMPILERLAB_ROOT}/src/runtime/parser_runtime.cpp\n";
    out << ")\n";
    out << "target_include_directories(compilerlab_runtime PUBLIC ${COMPILERLAB_ROOT}/src)\n";
    out << "target_link_libraries(compilerlab_runtime PUBLIC compilerlab_common)\n\n";
    out << "add_library(compilerlab_semantic STATIC\n";
    out << "    ${COMPILERLAB_ROOT}/src/semantic/ast.cpp\n";
    out << "    ${COMPILERLAB_ROOT}/src/semantic/ast_factory.cpp\n";
    out << "    ${COMPILERLAB_ROOT}/src/semantic/types.cpp\n";
    out << "    ${COMPILERLAB_ROOT}/src/semantic/semantic_action_executor.cpp\n";
    out << ")\n";
    out << "target_include_directories(compilerlab_semantic PUBLIC ${COMPILERLAB_ROOT}/src)\n";
    out << "target_link_libraries(compilerlab_semantic PUBLIC compilerlab_common compilerlab_runtime)\n\n";
    out << "add_executable(" << scanner_name << "_scanner\n";
    out << "    " << scanner_name << "_scanner.cpp\n";
    out << "    " << scanner_name << "_scanner_main.cpp\n";
    out << ")\n";
    out << "target_include_directories(" << scanner_name << "_scanner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})\n\n";
    out << "add_executable(" << parser_name << "_parser\n";
    out << "    " << scanner_name << "_scanner.cpp\n";
    out << "    " << parser_name << "_parser.cpp\n";
    out << "    " << parser_name << "_parser_main.cpp\n";
    out << ")\n";
    out << "target_include_directories(" << parser_name
        << "_parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${COMPILERLAB_ROOT}/src)\n";
    out << "target_link_libraries(" << parser_name
        << "_parser PRIVATE compilerlab_common compilerlab_runtime compilerlab_semantic)\n";
    return true;
}

}  // namespace

bool ParserEmitter::emit(const Grammar&, const ParseTable& table,
                         const std::vector<runtime::ProductionInfo>& productions,
                         const std::unordered_map<int, SymbolId>& token_symbols,
                         const std::string& output_dir, const std::string& parser_name,
                         const std::string& scanner_name, std::size_t lr1_state_count,
                         std::size_t lalr_state_count, const std::string& project_root,
                         common::DiagnosticEngine& diagnostics) const {
    if (output_dir.empty()) {
        diagnostics.error("parser output directory is empty");
        return false;
    }
    if (parser_name.empty()) {
        diagnostics.error("parser output name is empty");
        return false;
    }
    if (scanner_name.empty()) {
        diagnostics.error("scanner dependency name is empty for parser emission");
        return false;
    }
    if (project_root.empty()) {
        diagnostics.error("project root is empty for parser emission");
        return false;
    }

    std::error_code error;
    std::filesystem::create_directories(output_dir, error);
    if (error) {
        diagnostics.error("failed to create parser output directory: " + output_dir);
        return false;
    }

    const auto safe_parser_name = sanitize_identifier(parser_name);
    const auto safe_scanner_name = sanitize_identifier(scanner_name);
    const auto parser_class_name = to_pascal_case(safe_parser_name) + "Parser";
    const auto scanner_class_name = to_pascal_case(safe_scanner_name) + "Scanner";
    const auto parser_header_path = std::filesystem::path(output_dir) / (safe_parser_name + "_parser.h");
    const auto parser_source_path = std::filesystem::path(output_dir) / (safe_parser_name + "_parser.cpp");
    const auto parser_main_path = std::filesystem::path(output_dir) / (safe_parser_name + "_parser_main.cpp");
    const auto cmake_path = std::filesystem::path(output_dir) / "CMakeLists.txt";
    const auto scanner_header_name = safe_scanner_name + "_scanner.h";

    if (!write_parser_header(parser_header_path, parser_class_name, scanner_header_name,
                             scanner_class_name, diagnostics)) {
        return false;
    }
    if (!write_parser_source(parser_source_path, parser_header_path.filename().string(),
                             parser_class_name, scanner_class_name, table, productions, token_symbols,
                             lr1_state_count, lalr_state_count, diagnostics)) {
        return false;
    }
    if (!write_parser_main(parser_main_path, parser_header_path.filename().string(),
                           parser_class_name, scanner_class_name, safe_parser_name, diagnostics)) {
        return false;
    }
    if (!write_generated_cmake(cmake_path, safe_parser_name, safe_scanner_name,
                               std::filesystem::path(project_root), diagnostics)) {
        return false;
    }

    return true;
}

}  // namespace compilerlab::seuyacc
