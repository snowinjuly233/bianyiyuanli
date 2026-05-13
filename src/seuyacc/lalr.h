#pragma once

#include "seuyacc/lr1.h"

namespace compilerlab::seuyacc {

class LALRMerger {
public:
    LR1Automaton merge(const LR1Automaton& lr1_automaton) const;
};

}  // namespace compilerlab::seuyacc
