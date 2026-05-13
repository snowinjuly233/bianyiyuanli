#pragma once

#include <string>
#include <vector>

#include "ir/quad.h"

namespace compilerlab::ir {

struct BasicBlock {
    std::string name;
    std::vector<Quad> quads;
    std::vector<std::string> successors;
};

std::vector<BasicBlock> build_basic_blocks(const std::vector<Quad>& quads);

class BasicBlockPrinter {
public:
    std::string print(const std::vector<BasicBlock>& blocks) const;
};

}  // namespace compilerlab::ir
