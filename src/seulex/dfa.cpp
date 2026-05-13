#include "seulex/dfa.h"

#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <unordered_map>
#include <vector>

namespace compilerlab::seulex {

namespace {

using StateSet = std::set<NFAStateId>;

struct StateSetLess {
    bool operator()(const StateSet& lhs, const StateSet& rhs) const {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
};

}  // namespace

DFA DFABuilder::build(const NFA& nfa) const {
    DFA dfa;
    if (nfa.states.empty()) {
        return dfa;
    }

    std::unordered_map<NFAStateId, std::vector<const NFAEdge*>> outgoing;
    std::unordered_map<NFAStateId, NFAState> state_info;
    std::set<char> alphabet;

    for (const auto& state : nfa.states) {
        state_info.emplace(state.id, state);
    }
    for (const auto& edge : nfa.edges) {
        outgoing[edge.from].push_back(&edge);
        if (!edge.epsilon) {
            for (char ch : edge.symbol) {
                alphabet.insert(ch);
            }
        }
    }

    const auto epsilon_closure = [&](StateSet input) {
        std::queue<NFAStateId> work;
        for (auto state_id : input) {
            work.push(state_id);
        }

        while (!work.empty()) {
            const auto current = work.front();
            work.pop();
            for (const auto* edge : outgoing[current]) {
                if (!edge->epsilon) {
                    continue;
                }
                if (input.insert(edge->to).second) {
                    work.push(edge->to);
                }
            }
        }

        return input;
    };

    const auto move_on_char = [&](const StateSet& states, char ch) {
        StateSet moved;
        for (auto state_id : states) {
            for (const auto* edge : outgoing[state_id]) {
                if (!edge->epsilon && edge->symbol.find(ch) != std::string::npos) {
                    moved.insert(edge->to);
                }
            }
        }
        return moved;
    };

    const auto make_dfa_state = [&](DFAStateId id, const StateSet& states) {
        DFAState state;
        state.id = id;
        state.nfa_states = states;
        bool found_accept = false;
        for (auto nfa_state_id : states) {
            const auto it = state_info.find(nfa_state_id);
            if (it == state_info.end() || !it->second.accepting) {
                continue;
            }
            if (!found_accept || it->second.rule_priority < state.rule_priority) {
                state.accepting = true;
                state.rule_priority = it->second.rule_priority;
                found_accept = true;
            }
        }
        return state;
    };

    StateSet initial;
    initial.insert(nfa.start_state);
    initial = epsilon_closure(std::move(initial));

    std::map<StateSet, DFAStateId, StateSetLess> known_states;
    std::queue<StateSet> pending;

    dfa.start_state = 0;
    known_states.emplace(initial, 0);
    dfa.states.push_back(make_dfa_state(0, initial));
    pending.push(initial);

    while (!pending.empty()) {
        const auto current = pending.front();
        pending.pop();
        const auto from_id = known_states[current];

        for (char ch : alphabet) {
            auto moved = move_on_char(current, ch);
            if (moved.empty()) {
                continue;
            }

            auto target = epsilon_closure(std::move(moved));
            auto found = known_states.find(target);
            if (found == known_states.end()) {
                const auto new_id = static_cast<DFAStateId>(known_states.size());
                found = known_states.emplace(target, new_id).first;
                dfa.states.push_back(make_dfa_state(new_id, found->first));
                pending.push(found->first);
            }

            dfa.transitions.push_back({from_id, found->second, ch});
        }
    }

    return dfa;
}

}  // namespace compilerlab::seulex
