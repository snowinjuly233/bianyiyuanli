#pragma once

#include <set>
#include <unordered_map>

#include "seuyacc/grammar.h"

namespace compilerlab::seuyacc {

class FirstSetComputer {
public:
    using FirstSetTable = std::unordered_map<SymbolId, std::set<SymbolId>>;

    struct Result {
        FirstSetTable table;
        std::set<SymbolId> nullable_symbols;
    };

    Result compute(const Grammar& grammar) const;
    std::set<SymbolId> first_of_sequence(const std::vector<SymbolId>& sequence,
                                         const Result& result) const;
};

}  // namespace compilerlab::seuyacc
