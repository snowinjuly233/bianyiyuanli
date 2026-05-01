#include "minic_scanner.h"

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
            std::cerr << "usage: minic_scanner <input-file>\n";
            return 1;
        }

        const std::string input_path = argv[1];
        const auto input = read_file_or_die(input_path);
        compilerlab::generated::MinicScanner scanner(input_path);
        std::size_t offset = 0;
        while (true) {
            auto token = scanner.next_token(input, offset);
            if (token.kind == compilerlab::generated::TokenKind::Invalid) {
                std::cerr << "error: invalid token starting at "
                          << token.span.begin.line << ":" << token.span.begin.column
                          << " -> `" << compilerlab::generated::escape_text(token.lexeme) << "`\n";
                return 1;
            }
            if (token.kind == compilerlab::generated::TokenKind::EndOfFile) {
                std::cout << "EndOfFile\n";
                break;
            }
            std::cout << compilerlab::generated::to_string(token.kind)
                      << " \"" << compilerlab::generated::escape_text(token.lexeme) << "\""
                      << " @" << token.span.begin.line << ":" << token.span.begin.column
                      << '\n';
        }
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "fatal: " << ex.what() << '\n';
        return 1;
    }
}
