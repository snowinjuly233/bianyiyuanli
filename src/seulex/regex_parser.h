#pragma once

#include <string>

#include "common/diagnostic.h"
#include "seulex/regex_ast.h"

namespace compilerlab::seulex {

class RegexParser {
public:
    RegexNodePtr parse(const std::string& pattern, common::DiagnosticEngine& diagnostics) const;
};

}  // namespace compilerlab::seulex
