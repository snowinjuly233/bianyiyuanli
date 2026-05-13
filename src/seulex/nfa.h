#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "seulex/regex_ast.h"

namespace compilerlab::seulex {

using NFAStateId = int;

struct NFAState {
    NFAStateId id {0};
    bool accepting {false};
    std::size_t rule_priority {0};
};

struct NFAEdge {
    NFAStateId from {0};
    NFAStateId to {0};
    std::string symbol;
    bool epsilon {false};
};

struct NFA {
    NFAStateId start_state {0};
    NFAStateId accept_state {0};
    std::vector<NFAState> states;
    std::vector<NFAEdge> edges;
};

class NFABuilder {
public:
    NFA build(const RegexNode& node, std::size_t rule_priority) const;
    NFA combine(const std::vector<NFA>& automata) const;
};

}  // namespace compilerlab::seulex
