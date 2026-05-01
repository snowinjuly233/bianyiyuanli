#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "semantic/ast.h"
#include "minic_scanner.h"

namespace compilerlab::generated {

struct ParseResult {
    bool accepted {false};
    std::vector<std::string> reductions;
    std::string error_message;
    SourceSpan error_span {};
    compilerlab::semantic::ProgramPtr program;
};

class MinicParser {
public:
    ParseResult parse(const std::vector<Token>& tokens) const;
    ParseResult parse_source(std::string_view input, const MinicScanner& scanner) const;

    static std::size_t lr1_state_count();
    static std::size_t lalr_state_count();
};

}  // namespace compilerlab::generated
