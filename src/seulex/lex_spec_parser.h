#pragma once

#include <string>

#include "common/diagnostic.h"
#include "common/source.h"
#include "seulex/lex_spec.h"

namespace compilerlab::seulex {

class LexSpecParser {
public:
    LexSpec parse_file(const std::string& path, common::SourceManager& sources,
                       common::DiagnosticEngine& diagnostics) const;
};

}  // namespace compilerlab::seulex
