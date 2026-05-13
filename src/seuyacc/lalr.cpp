#include "seuyacc/lalr.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace compilerlab::seuyacc {

namespace {

using FullItemKey = std::tuple<std::size_t, std::size_t, SymbolId>;
using CoreItemKey = std::pair<std::size_t, std::size_t>;

bool item_less(const LR1Item& lhs, const LR1Item& rhs) {
    return FullItemKey{lhs.rule_index, lhs.dot_position, lhs.lookahead} <
           FullItemKey{rhs.rule_index, rhs.dot_position, rhs.lookahead};
}

std::string core_key(const LR1State& state) {
    std::vector<CoreItemKey> core;
    core.reserve(state.items.size());
    for (const auto& item : state.items) {
        core.emplace_back(item.rule_index, item.dot_position);
    }

    std::sort(core.begin(), core.end());
    core.erase(std::unique(core.begin(), core.end()), core.end());

    std::ostringstream key;
    for (const auto& [rule_index, dot_position] : core) {
        key << rule_index << ':' << dot_position << ';';
    }
    return key.str();
}

}  // namespace

LR1Automaton LALRMerger::merge(const LR1Automaton& lr1_automaton) const {
    if (lr1_automaton.states.empty()) {
        return lr1_automaton;
    }

    LR1Automaton lalr_automaton;

    std::unordered_map<std::string, int> group_by_core;
    std::vector<std::vector<int>> grouped_state_ids;
    grouped_state_ids.reserve(lr1_automaton.states.size());

    for (const auto& state : lr1_automaton.states) {
        const auto key = core_key(state);
        const auto [it, inserted] =
            group_by_core.emplace(key, static_cast<int>(grouped_state_ids.size()));
        if (inserted) {
            grouped_state_ids.push_back({});
        }
        grouped_state_ids[static_cast<std::size_t>(it->second)].push_back(state.id);
    }

    std::vector<int> merged_of_old(lr1_automaton.states.size(), -1);
    for (std::size_t merged_id = 0; merged_id < grouped_state_ids.size(); ++merged_id) {
        for (const auto old_state_id : grouped_state_ids[merged_id]) {
            if (old_state_id >= 0 && static_cast<std::size_t>(old_state_id) < merged_of_old.size()) {
                merged_of_old[static_cast<std::size_t>(old_state_id)] = static_cast<int>(merged_id);
            }
        }
    }

    lalr_automaton.states.reserve(grouped_state_ids.size());
    lalr_automaton.transitions.resize(grouped_state_ids.size());

    for (std::size_t merged_id = 0; merged_id < grouped_state_ids.size(); ++merged_id) {
        std::set<FullItemKey> seen_items;
        std::vector<LR1Item> merged_items;

        for (const auto old_state_id : grouped_state_ids[merged_id]) {
            const auto& old_state = lr1_automaton.states[static_cast<std::size_t>(old_state_id)];
            for (const auto& item : old_state.items) {
                const auto key = FullItemKey{item.rule_index, item.dot_position, item.lookahead};
                if (seen_items.insert(key).second) {
                    merged_items.push_back(item);
                }
            }
        }

        std::sort(merged_items.begin(), merged_items.end(), item_less);
        lalr_automaton.states.push_back({static_cast<int>(merged_id), std::move(merged_items)});
    }

    for (std::size_t merged_id = 0; merged_id < grouped_state_ids.size(); ++merged_id) {
        auto& merged_transitions = lalr_automaton.transitions[merged_id];
        for (const auto old_state_id : grouped_state_ids[merged_id]) {
            const auto& old_transitions = lr1_automaton.transitions[static_cast<std::size_t>(old_state_id)];
            for (const auto& [symbol, old_target] : old_transitions) {
                if (old_target < 0 || static_cast<std::size_t>(old_target) >= merged_of_old.size()) {
                    continue;
                }
                merged_transitions[symbol] = merged_of_old[static_cast<std::size_t>(old_target)];
            }
        }
    }

    return lalr_automaton;
}

}  // namespace compilerlab::seuyacc
