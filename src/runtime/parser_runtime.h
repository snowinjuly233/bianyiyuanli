#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/diagnostic.h"
#include "common/token.h"
#include "semantic/semantic_value.h"

namespace compilerlab::runtime {

enum class ParserActionKind {
    Error,
    Shift,
    Reduce,
    Accept,
};

struct ParserAction {
    ParserActionKind kind {ParserActionKind::Error};
    int value {-1};
};

struct ProductionInfo {
    int lhs_symbol {-1};
    int rhs_size {0};
    std::string debug_name;
    std::string action_text;
};

using ReduceCallback = std::function<semantic::SemanticValue(
    int production_index,
    const std::vector<semantic::SemanticValue>& rhs_values)>;

class ParserRuntime {
public:
    semantic::SemanticValue parse(
        const std::vector<common::Token>& tokens,
        const std::vector<std::unordered_map<int, ParserAction>>& action_table,
        const std::vector<std::unordered_map<int, int>>& goto_table,
        const std::vector<ProductionInfo>& productions,
        const std::function<int(common::TokenKind)>& token_to_symbol,
        const ReduceCallback& on_reduce,
        common::DiagnosticEngine& diagnostics) const;
};

}  // namespace compilerlab::runtime
