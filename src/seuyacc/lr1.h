#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "common/diagnostic.h"
#include "seuyacc/grammar.h"

namespace compilerlab::seuyacc {

struct LR1Item {
    std::size_t rule_index {0};
    std::size_t dot_position {0};
    SymbolId lookahead {-1};
};

struct LR1State {
    int id {0};
    std::vector<LR1Item> items;
};

struct LR1Automaton {
    std::vector<LR1State> states;
    std::vector<std::unordered_map<SymbolId, int>> transitions;
};

class LR1Builder {
public:
    LR1Automaton build(const Grammar& grammar, common::DiagnosticEngine& diagnostics) const;
};

}  // namespace compilerlab::seuyacc
