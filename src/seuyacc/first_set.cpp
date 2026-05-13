#include "seuyacc/first_set.h"

#include <algorithm>

namespace compilerlab::seuyacc {

namespace {

bool is_nullable(SymbolId symbol, const FirstSetComputer::Result& result) {
    return result.nullable_symbols.find(symbol) != result.nullable_symbols.end();
}

}  // namespace

FirstSetComputer::Result FirstSetComputer::compute(const Grammar& grammar) const {
    Result result;
    for (const auto& symbol : grammar.symbols()) {
        if (symbol.terminal) {
            result.table[symbol.id].insert(symbol.id);
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& rule : grammar.rules()) {
            auto& lhs_first = result.table[rule.lhs];

            if (rule.rhs.empty()) {
                changed = result.nullable_symbols.insert(rule.lhs).second || changed;
                continue;
            }

            bool rhs_nullable = true;
            for (const auto symbol : rule.rhs) {
                const auto& first = result.table[symbol];
                const auto old_size = lhs_first.size();
                lhs_first.insert(first.begin(), first.end());
                if (lhs_first.size() != old_size) {
                    changed = true;
                }

                if (!is_nullable(symbol, result)) {
                    rhs_nullable = false;
                    break;
                }
            }

            if (rhs_nullable) {
                changed = result.nullable_symbols.insert(rule.lhs).second || changed;
            }
        }
    }

    return result;
}

std::set<SymbolId> FirstSetComputer::first_of_sequence(const std::vector<SymbolId>& sequence,
                                                        const Result& result) const {
    std::set<SymbolId> first;
    for (const auto symbol : sequence) {
        if (const auto it = result.table.find(symbol); it != result.table.end()) {
            first.insert(it->second.begin(), it->second.end());
        }
        if (!is_nullable(symbol, result)) {
            break;
        }
    }
    return first;
}

}  // namespace compilerlab::seuyacc
