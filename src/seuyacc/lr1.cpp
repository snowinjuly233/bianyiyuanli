#include "seuyacc/lr1.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "seuyacc/first_set.h"

namespace compilerlab::seuyacc {

namespace {

using ItemKey = std::tuple<std::size_t, std::size_t, SymbolId>;

ItemKey make_key(const LR1Item& item) {
    return {item.rule_index, item.dot_position, item.lookahead};
}

bool item_less(const LR1Item& lhs, const LR1Item& rhs) {
    return make_key(lhs) < make_key(rhs);
}

std::string state_key(std::vector<LR1Item> items) {
    std::sort(items.begin(), items.end(), item_less);

    std::ostringstream key;
    for (const auto& item : items) {
        key << item.rule_index << ':' << item.dot_position << ':' << item.lookahead << ';';
    }
    return key.str();
}

}  // namespace

LR1Automaton LR1Builder::build(const Grammar& grammar, common::DiagnosticEngine& diagnostics) const {
    LR1Automaton automaton;
    if (grammar.rules().empty()) {
        diagnostics.warning("LR(1) build requested on an empty grammar");
        return automaton;
    }

    if (grammar.start_symbol() < 0) {
        diagnostics.error("grammar start symbol has not been set");
        return automaton;
    }

    const auto* eof_symbol = grammar.lookup_symbol("EndOfFile");
    if (eof_symbol == nullptr) {
        diagnostics.error("grammar is missing the EndOfFile terminal");
        return automaton;
    }

    const FirstSetComputer first_computer;
    const auto first_result = first_computer.compute(grammar);
    const auto& rules = grammar.rules();

    auto closure = [&](const std::vector<LR1Item>& seed_items) {
        std::set<ItemKey> seen;
        std::vector<LR1Item> items;
        std::vector<LR1Item> worklist;

        auto push_item = [&](const LR1Item& item) {
            if (seen.insert(make_key(item)).second) {
                items.push_back(item);
                worklist.push_back(item);
            }
        };

        for (const auto& item : seed_items) {
            push_item(item);
        }

        while (!worklist.empty()) {
            const auto item = worklist.back();
            worklist.pop_back();

            const auto& rule = rules[item.rule_index];
            if (item.dot_position >= rule.rhs.size()) {
                continue;
            }

            const auto next_symbol = rule.rhs[item.dot_position];
            const auto& symbol = grammar.symbols()[static_cast<std::size_t>(next_symbol)];
            if (symbol.terminal) {
                continue;
            }

            std::vector<SymbolId> lookahead_sequence;
            for (std::size_t index = item.dot_position + 1; index < rule.rhs.size(); ++index) {
                lookahead_sequence.push_back(rule.rhs[index]);
            }
            lookahead_sequence.push_back(item.lookahead);

            const auto lookaheads = first_computer.first_of_sequence(lookahead_sequence, first_result);
            for (std::size_t rule_index = 0; rule_index < rules.size(); ++rule_index) {
                if (rules[rule_index].lhs != next_symbol) {
                    continue;
                }

                for (const auto lookahead : lookaheads) {
                    push_item({rule_index, 0, lookahead});
                }
            }
        }

        std::sort(items.begin(), items.end(), item_less);
        return items;
    };

    auto goto_items = [&](const std::vector<LR1Item>& items, SymbolId symbol) {
        std::vector<LR1Item> advanced;
        for (const auto& item : items) {
            const auto& rule = rules[item.rule_index];
            if (item.dot_position < rule.rhs.size() && rule.rhs[item.dot_position] == symbol) {
                advanced.push_back({item.rule_index, item.dot_position + 1, item.lookahead});
            }
        }
        return closure(advanced);
    };

    const auto initial_items = closure({LR1Item{0, 0, eof_symbol->id}});
    automaton.states.push_back({0, initial_items});
    automaton.transitions.emplace_back();

    std::unordered_map<std::string, int> state_ids;
    state_ids.emplace(state_key(initial_items), 0);

    for (std::size_t state_index = 0; state_index < automaton.states.size(); ++state_index) {
        const auto items = automaton.states[state_index].items;
        std::set<SymbolId> next_symbols;
        for (const auto& item : items) {
            const auto& rule = rules[item.rule_index];
            if (item.dot_position < rule.rhs.size()) {
                next_symbols.insert(rule.rhs[item.dot_position]);
            }
        }

        for (const auto symbol : next_symbols) {
            auto next_state_items = goto_items(items, symbol);
            if (next_state_items.empty()) {
                continue;
            }

            const auto key = state_key(next_state_items);
            int target_state = -1;
            if (const auto it = state_ids.find(key); it != state_ids.end()) {
                target_state = it->second;
            } else {
                target_state = static_cast<int>(automaton.states.size());
                state_ids.emplace(key, target_state);
                automaton.states.push_back({target_state, std::move(next_state_items)});
                automaton.transitions.emplace_back();
            }

            automaton.transitions[state_index][symbol] = target_state;
        }
    }

    return automaton;
}

}  // namespace compilerlab::seuyacc
