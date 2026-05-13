#pragma once

#include <string>

#include "common/diagnostic.h"
#include "common/source.h"
#include "seuyacc/yacc_spec.h"

namespace compilerlab::seuyacc {

class YaccSpecParser {
public:
    YaccSpec parse_file(const std::string& path, common::SourceManager& sources,
                        common::DiagnosticEngine& diagnostics) const;
};

}  // namespace compilerlab::seuyacc
