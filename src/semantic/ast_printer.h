#pragma once

#include <string>

#include "semantic/ast.h"

namespace compilerlab::semantic {

class AstPrinter {
public:
    std::string print_text(const ProgramPtr& program) const;
    std::string print_markdown(const ProgramPtr& program) const;
};

}  // namespace compilerlab::semantic
