#pragma once

#include <string>
#include <vector>

#include "common/diagnostic.h"
#include "seulex/dfa.h"
#include "seulex/lex_spec.h"

namespace compilerlab::seulex {

class ScannerEmitter {
public:
    bool emit(const DFA& dfa, const std::vector<LexRule>& rules, const std::string& output_dir,
              const std::string& scanner_name, common::DiagnosticEngine& diagnostics) const;
};

}  // namespace compilerlab::seulex
