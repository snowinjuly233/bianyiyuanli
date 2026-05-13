#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/diagnostic.h"
#include "runtime/parser_runtime.h"
#include "seuyacc/grammar.h"
#include "seuyacc/parse_table.h"

namespace compilerlab::seuyacc {

class ParserEmitter {
public:
    bool emit(const Grammar& grammar, const ParseTable& table,
              const std::vector<runtime::ProductionInfo>& productions,
              const std::unordered_map<int, SymbolId>& token_symbols,
              const std::string& output_dir, const std::string& parser_name,
              const std::string& scanner_name, std::size_t lr1_state_count,
              std::size_t lalr_state_count, const std::string& project_root,
              common::DiagnosticEngine& diagnostics) const;
};

}  // namespace compilerlab::seuyacc
