#pragma once

#include <string>
#include <unordered_map>

#include "common/diagnostic.h"

namespace compilerlab::seulex {

class RegexNormalizer {
public:
    std::string normalize(const std::string& pattern,
                          const std::unordered_map<std::string, std::string>& definitions,
                          common::DiagnosticEngine& diagnostics) const;
};

}  // namespace compilerlab::seulex
