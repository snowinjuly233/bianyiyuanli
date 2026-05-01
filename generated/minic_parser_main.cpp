#include "minic_parser.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

std::string read_file_or_die(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to open input file: " + path);
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "usage: minic_parser <input-file>\n";
            return 1;
        }

        const std::string input_path = argv[1];
        const auto input = read_file_or_die(input_path);
        compilerlab::generated::MinicScanner scanner(input_path);
        compilerlab::generated::MinicParser parser;
        std::cout << "LR(1) states: " << compilerlab::generated::MinicParser::lr1_state_count() << '\n';
        std::cout << "LALR(1) states: " << compilerlab::generated::MinicParser::lalr_state_count() << '\n';
        const auto result = parser.parse_source(input, scanner);
        for (const auto& reduction : result.reductions) {
            std::cout << reduction << '\n';
        }
        if (result.accepted) {
            if (result.program) {
                std::cout << "AST root: "
                          << compilerlab::semantic::to_string(result.program->kind)
                          << '\n';
            }
            std::cout << "ACCEPT\n";
            return 0;
        }
        std::cout << "REJECT\n";
        std::cerr << "error: " << result.error_message;
        if (!result.error_span.file_path.empty()) {
            std::cerr << " (" << result.error_span.file_path << ":"
                      << result.error_span.begin.line << ":"
                      << result.error_span.begin.column << ")";
        }
        std::cerr << '\n';
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "fatal: " << ex.what() << '\n';
        return 1;
    }
}
