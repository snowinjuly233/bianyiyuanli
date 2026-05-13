#pragma once

#include <cstddef>
#include <set>
#include <vector>

#include "seulex/nfa.h"

namespace compilerlab::seulex {

using DFAStateId = int;

struct DFAState {
    DFAStateId id {0};
    bool accepting {false};
    std::size_t rule_priority {0};
    std::set<NFAStateId> nfa_states;
};

struct DFATransition {
    DFAStateId from {0};
    DFAStateId to {0};
    char symbol {'\0'};
};

struct DFA {
    DFAStateId start_state {0};
    std::vector<DFAState> states;
    std::vector<DFATransition> transitions;
};

class DFABuilder {
public:
    DFA build(const NFA& nfa) const;
};

}  // namespace compilerlab::seulex
