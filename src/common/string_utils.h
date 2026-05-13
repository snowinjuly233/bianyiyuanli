#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace compilerlab::common {

std::string trim(std::string_view text);
std::vector<std::string> split_lines(std::string_view text);
std::string escape_text(std::string_view text);
bool is_identifier_start(char ch);
bool is_identifier_continue(char ch);

}  // namespace compilerlab::common
