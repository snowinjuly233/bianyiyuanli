#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "seuyacc/yacc_spec.h"

namespace compilerlab::seuyacc {

using SymbolId = int;

struct GrammarSymbol {
    SymbolId id {0};
    std::string name;
    bool terminal {false};
    int precedence_level {0};
    Assoc assoc {Assoc::None};
};

struct GrammarRule {
    SymbolId lhs {0};
    std::vector<SymbolId> rhs;
    std::size_t production_index {0};
    int precedence_level {0};
    Assoc assoc {Assoc::None};
};

class Grammar {
public:
    SymbolId add_terminal(std::string name);
    SymbolId add_nonterminal(std::string name);
    void add_rule(SymbolId lhs, std::vector<SymbolId> rhs, std::size_t production_index,
                  int precedence_level = 0, Assoc assoc = Assoc::None);
    void set_start_symbol(SymbolId start_symbol);
    void set_symbol_precedence(SymbolId symbol_id, int precedence_level, Assoc assoc);

    SymbolId start_symbol() const;
    const GrammarSymbol* lookup_symbol(std::string_view name) const;
    const std::vector<GrammarSymbol>& symbols() const;
    const std::vector<GrammarRule>& rules() const;

private:
    SymbolId add_symbol(std::string name, bool terminal);

    SymbolId start_symbol_ {-1};
    std::vector<GrammarSymbol> symbols_;
    std::vector<GrammarRule> rules_;
    std::unordered_map<std::string, SymbolId> symbol_ids_;
};

}  // namespace compilerlab::seuyacc
