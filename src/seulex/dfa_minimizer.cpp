#include "seulex/dfa_minimizer.h"

#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace compilerlab::seulex {

DFA DFAMinimizer::minimize(const DFA& dfa) const {
    if (dfa.states.empty()) {
        return dfa;
    }

    std::unordered_map<DFAStateId, const DFAState*> states_by_id;
    std::unordered_map<DFAStateId, std::unordered_map<unsigned char, DFAStateId>> transitions;
    std::set<unsigned char> alphabet;

    for (const auto& state : dfa.states) {
        states_by_id.emplace(state.id, &state);
    }
    for (const auto& transition : dfa.transitions) {
        const auto symbol = static_cast<unsigned char>(transition.symbol);
        transitions[transition.from][symbol] = transition.to;
        alphabet.insert(symbol);
    }

    std::map<std::pair<bool, std::size_t>, std::vector<DFAStateId>> initial_groups;
    for (const auto& state : dfa.states) {
        initial_groups[{state.accepting, state.accepting ? state.rule_priority : 0}].push_back(state.id);
    }

    std::vector<std::vector<DFAStateId>> partitions;
    partitions.reserve(initial_groups.size());
    for (auto& [_, group] : initial_groups) {
        std::sort(group.begin(), group.end());
        partitions.push_back(std::move(group));
    }

    bool changed = true;
    while (changed) {
        changed = false;

        std::unordered_map<DFAStateId, int> block_of_state;
        for (int block_id = 0; block_id < static_cast<int>(partitions.size()); ++block_id) {
            for (auto state_id : partitions[static_cast<std::size_t>(block_id)]) {
                block_of_state[state_id] = block_id;
            }
        }

        std::vector<std::vector<DFAStateId>> refined;
        for (const auto& block : partitions) {
            std::map<std::vector<int>, std::vector<DFAStateId>> split_groups;

            for (auto state_id : block) {
                std::vector<int> signature;
                signature.reserve(alphabet.size());
                for (auto symbol : alphabet) {
                    const auto state_it = transitions.find(state_id);
                    if (state_it == transitions.end()) {
                        signature.push_back(-1);
                        continue;
                    }
                    const auto target_it = state_it->second.find(symbol);
                    if (target_it == state_it->second.end()) {
                        signature.push_back(-1);
                        continue;
                    }
                    signature.push_back(block_of_state[target_it->second]);
                }
                split_groups[signature].push_back(state_id);
            }

            if (split_groups.size() > 1) {
                changed = true;
            }

            for (auto& [_, group] : split_groups) {
                std::sort(group.begin(), group.end());
                refined.push_back(std::move(group));
            }
        }

        partitions = std::move(refined);
    }

    auto group_sort_key = [&](const std::vector<DFAStateId>& group) {
        const bool contains_start = std::find(group.begin(), group.end(), dfa.start_state) != group.end();
        return std::pair<int, DFAStateId>(contains_start ? 0 : 1, group.front());
    };
    std::sort(partitions.begin(), partitions.end(), [&](const auto& lhs, const auto& rhs) {
        return group_sort_key(lhs) < group_sort_key(rhs);
    });

    std::unordered_map<DFAStateId, DFAStateId> new_id_of_old;
    for (DFAStateId new_id = 0; new_id < static_cast<DFAStateId>(partitions.size()); ++new_id) {
        for (auto old_id : partitions[static_cast<std::size_t>(new_id)]) {
            new_id_of_old[old_id] = new_id;
        }
    }

    DFA minimized;
    minimized.start_state = new_id_of_old[dfa.start_state];
    minimized.states.reserve(partitions.size());

    for (DFAStateId new_id = 0; new_id < static_cast<DFAStateId>(partitions.size()); ++new_id) {
        const auto& group = partitions[static_cast<std::size_t>(new_id)];
        const auto* representative = states_by_id[group.front()];

        DFAState merged;
        merged.id = new_id;
        merged.accepting = representative->accepting;
        merged.rule_priority = representative->rule_priority;
        for (auto old_id : group) {
            const auto* old_state = states_by_id[old_id];
            merged.nfa_states.insert(old_state->nfa_states.begin(), old_state->nfa_states.end());
        }
        minimized.states.push_back(std::move(merged));
    }

    for (DFAStateId new_id = 0; new_id < static_cast<DFAStateId>(partitions.size()); ++new_id) {
        const auto& group = partitions[static_cast<std::size_t>(new_id)];
        const auto representative = group.front();

        for (auto symbol : alphabet) {
            const auto state_it = transitions.find(representative);
            if (state_it == transitions.end()) {
                continue;
            }
            const auto target_it = state_it->second.find(symbol);
            if (target_it == state_it->second.end()) {
                continue;
            }
            minimized.transitions.push_back({
                new_id,
                new_id_of_old[target_it->second],
                static_cast<char>(symbol),
            });
        }
    }

    return minimized;
}

}  // namespace compilerlab::seulex
