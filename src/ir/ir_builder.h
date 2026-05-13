#pragma once

#include <vector>

#include "common/diagnostic.h"
#include "ir/quad.h"
#include "semantic/ast.h"

namespace compilerlab::ir {

class IRBuilder {
public:
    std::vector<Quad> build(const semantic::ProgramPtr& program,
                            common::DiagnosticEngine& diagnostics) const;
};

}  // namespace compilerlab::ir
