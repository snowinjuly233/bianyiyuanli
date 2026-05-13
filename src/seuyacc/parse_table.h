#pragma once

#include <unordered_map>
#include <vector>

#include "common/diagnostic.h"
#include "runtime/parser_runtime.h"
#include "seuyacc/grammar.h"
#include "seuyacc/lr1.h"

namespace compilerlab::seuyacc {

struct ParseTable {
    std::vector<std::unordered_map<int, runtime::ParserAction>> actions;
    std::vector<std::unordered_map<int, int>> gotos;
};

class ParseTableBuilder {
public:
    ParseTable build(const Grammar& grammar, const LR1Automaton& automaton,
                     common::DiagnosticEngine& diagnostics) const;
};

}  // namespace compilerlab::seuyacc
