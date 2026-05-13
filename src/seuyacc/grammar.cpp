#include "seuyacc/grammar.h"

#include <utility>

namespace compilerlab::seuyacc {

SymbolId Grammar::add_terminal(std::string name) {
    return add_symbol(std::move(name), true);
}

SymbolId Grammar::add_nonterminal(std::string name) {
    return add_symbol(std::move(name), false);
}

void Grammar::add_rule(SymbolId lhs, std::vector<SymbolId> rhs, std::size_t production_index,
                       int precedence_level, Assoc assoc) {
    rules_.push_back({lhs, std::move(rhs), production_index, precedence_level, assoc});
}

void Grammar::set_start_symbol(SymbolId start_symbol) {
    start_symbol_ = start_symbol;
}

void Grammar::set_symbol_precedence(SymbolId symbol_id, int precedence_level, Assoc assoc) {
    if (symbol_id < 0 || static_cast<std::size_t>(symbol_id) >= symbols_.size()) {
        return;
    }
    auto& symbol = symbols_[static_cast<std::size_t>(symbol_id)];
    symbol.precedence_level = precedence_level;
    symbol.assoc = assoc;
}

SymbolId Grammar::start_symbol() const {
    return start_symbol_;
}

const GrammarSymbol* Grammar::lookup_symbol(std::string_view name) const {
    if (auto it = symbol_ids_.find(std::string(name)); it != symbol_ids_.end()) {
        return &symbols_[static_cast<std::size_t>(it->second)];
    }
    return nullptr;
}

const std::vector<GrammarSymbol>& Grammar::symbols() const {
    return symbols_;
}

const std::vector<GrammarRule>& Grammar::rules() const {
    return rules_;
}

SymbolId Grammar::add_symbol(std::string name, bool terminal) {
    if (auto it = symbol_ids_.find(name); it != symbol_ids_.end()) {
        return it->second;
    }

    const auto id = static_cast<SymbolId>(symbols_.size());
    symbol_ids_[name] = id;
    symbols_.push_back({id, std::move(name), terminal, 0, Assoc::None});
    return id;
}

}  // namespace compilerlab::seuyacc
