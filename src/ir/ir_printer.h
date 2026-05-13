#pragma once

#include <string>
#include <vector>

#include "ir/quad.h"

namespace compilerlab::ir {

class IRPrinter {
public:
    std::string print(const std::vector<Quad>& quads) const;
};

}  // namespace compilerlab::ir
