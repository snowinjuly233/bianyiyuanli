#include "common/string_utils.h"

#include <cctype>

namespace compilerlab::common {

std::string trim(std::string_view text) {
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start])) != 0) {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
        --end;
    }
    return std::string(text.substr(start, end - start));
}

std::vector<std::string> split_lines(std::string_view text) {
    std::vector<std::string> lines;
    std::string current;
    for (char ch : text) {
        if (ch == '\n') {
            lines.push_back(current);
            current.clear();
            continue;
        }
        if (ch != '\r') {
            current.push_back(ch);
        }
    }
    lines.push_back(current);
    return lines;
}

std::string escape_text(std::string_view text) {
    std::string escaped;
    escaped.reserve(text.size());
    for (char ch : text) {
        switch (ch) {
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            case '\\': escaped += "\\\\"; break;
            default: escaped.push_back(ch); break;
        }
    }
    return escaped;
}

bool is_identifier_start(char ch) {
    return std::isalpha(static_cast<unsigned char>(ch)) != 0 || ch == '_';
}

bool is_identifier_continue(char ch) {
    return is_identifier_start(ch) || std::isdigit(static_cast<unsigned char>(ch)) != 0;
}

}  // namespace compilerlab::common
