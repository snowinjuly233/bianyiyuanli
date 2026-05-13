#pragma once

#include "seulex/dfa.h"

namespace compilerlab::seulex {

class DFAMinimizer {
public:
    DFA minimize(const DFA& dfa) const;
};

}  // namespace compilerlab::seulex
