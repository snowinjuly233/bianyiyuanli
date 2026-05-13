#include "runtime/parser_runtime.h"

#include <sstream>

namespace compilerlab::runtime {

semantic::SemanticValue ParserRuntime::parse(
    const std::vector<common::Token>& tokens,
    const std::vector<std::unordered_map<int, ParserAction>>& action_table,
    const std::vector<std::unordered_map<int, int>>& goto_table,
    const std::vector<ProductionInfo>& productions,
    const std::function<int(common::TokenKind)>& token_to_symbol,
    const ReduceCallback& on_reduce,
    common::DiagnosticEngine& diagnostics) const {
    if (tokens.empty()) {
        diagnostics.error("parser received an empty token stream");
        return {};
    }
    if (action_table.empty() || goto_table.empty()) {
        diagnostics.error("parser tables have not been initialized");
        return {};
    }

    std::vector<int> state_stack {0};
    std::vector<semantic::SemanticValue> value_stack;
    std::size_t token_index = 0;

    while (true) {
        if (token_index >= tokens.size()) {
            diagnostics.error("parser ran past the end of the token stream");
            return {};
        }

        const auto& token = tokens[token_index];
        const auto symbol = token_to_symbol(token.kind);
        if (symbol < 0) {
            diagnostics.error("no grammar symbol matches token kind " + common::to_string(token.kind),
                              token.span);
            return {};
        }

        const auto state = state_stack.back();
        if (state < 0 || static_cast<std::size_t>(state) >= action_table.size()) {
            diagnostics.error("parser state stack contains an invalid state id");
            return {};
        }

        const auto action_it = action_table[static_cast<std::size_t>(state)].find(symbol);
        if (action_it == action_table[static_cast<std::size_t>(state)].end()) {
            diagnostics.error("unexpected token " + common::to_string(token.kind), token.span);
            return {};
        }

        const auto action = action_it->second;
        switch (action.kind) {
            case ParserActionKind::Shift:
                state_stack.push_back(action.value);
                value_stack.emplace_back(token);
                ++token_index;
                break;

            case ParserActionKind::Reduce: {
                if (action.value < 0 || static_cast<std::size_t>(action.value) >= productions.size()) {
                    diagnostics.error("reduce action references an invalid production index");
                    return {};
                }

                const auto& production = productions[static_cast<std::size_t>(action.value)];
                if (production.rhs_size < 0 ||
                    static_cast<std::size_t>(production.rhs_size) > value_stack.size() ||
                    static_cast<std::size_t>(production.rhs_size) >= state_stack.size()) {
                    diagnostics.error("parser stack underflow during reduction");
                    return {};
                }

                std::vector<semantic::SemanticValue> rhs_values;
                rhs_values.reserve(static_cast<std::size_t>(production.rhs_size));
                if (production.rhs_size > 0) {
                    const auto value_begin = value_stack.end() - production.rhs_size;
                    rhs_values.assign(value_begin, value_stack.end());
                    value_stack.erase(value_begin, value_stack.end());
                    state_stack.erase(state_stack.end() - production.rhs_size, state_stack.end());
                }

                semantic::SemanticValue reduced_value;
                if (on_reduce) {
                    reduced_value = on_reduce(action.value, rhs_values);
                }
                value_stack.push_back(std::move(reduced_value));

                const auto goto_state_it =
                    goto_table[static_cast<std::size_t>(state_stack.back())].find(production.lhs_symbol);
                if (goto_state_it == goto_table[static_cast<std::size_t>(state_stack.back())].end()) {
                    diagnostics.error("goto table entry is missing after reduction");
                    return {};
                }
                state_stack.push_back(goto_state_it->second);
                break;
            }

            case ParserActionKind::Accept:
                if (value_stack.empty()) {
                    return {};
                }
                return value_stack.back();

            case ParserActionKind::Error:
            default:
                diagnostics.error("parser encountered an error action", token.span);
                return {};
        }
    }
}

}  // namespace compilerlab::runtime
