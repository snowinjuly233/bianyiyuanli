#include "seuyacc/parse_table.h"

#include <optional>
#include <sstream>

namespace compilerlab::seuyacc {

namespace {

bool same_action(const runtime::ParserAction& lhs, const runtime::ParserAction& rhs) {
    return lhs.kind == rhs.kind && lhs.value == rhs.value;
}

std::string action_name(runtime::ParserActionKind kind) {
    switch (kind) {
        case runtime::ParserActionKind::Error: return "error";
        case runtime::ParserActionKind::Shift: return "shift";
        case runtime::ParserActionKind::Reduce: return "reduce";
        case runtime::ParserActionKind::Accept: return "accept";
    }
    return "unknown";
}

std::string assoc_name(Assoc assoc) {
    switch (assoc) {
        case Assoc::Left: return "left";
        case Assoc::Right: return "right";
        case Assoc::None: return "nonassoc";
    }
    return "unknown";
}

}  // namespace

ParseTable ParseTableBuilder::build(const Grammar& grammar, const LR1Automaton& automaton,
                                    common::DiagnosticEngine& diagnostics) const {
    ParseTable table;
    table.actions.resize(automaton.states.size());
    table.gotos.resize(automaton.states.size());
    if (automaton.states.empty()) {
        diagnostics.warning("parse table build requested on an empty LR automaton");
        return table;
    }

    const auto* eof_symbol = grammar.lookup_symbol("EndOfFile");
    if (eof_symbol == nullptr) {
        diagnostics.error("grammar is missing the EndOfFile terminal");
        return table;
    }

    const auto resolve_shift_reduce =
        [&](int terminal, const runtime::ParserAction& shift_action,
            const runtime::ParserAction& reduce_action) -> std::optional<runtime::ParserAction> {
        const auto& terminal_symbol = grammar.symbols()[static_cast<std::size_t>(terminal)];
        if (reduce_action.value < 0 ||
            static_cast<std::size_t>(reduce_action.value) >= grammar.rules().size()) {
            return std::nullopt;
        }

        const auto& reduce_rule = grammar.rules()[static_cast<std::size_t>(reduce_action.value)];
        const auto shift_precedence = terminal_symbol.precedence_level;
        const auto reduce_precedence = reduce_rule.precedence_level;

        if (shift_precedence <= 0 || reduce_precedence <= 0) {
            return std::nullopt;
        }

        if (shift_precedence > reduce_precedence) {
            return shift_action;
        }
        if (shift_precedence < reduce_precedence) {
            return reduce_action;
        }

        switch (terminal_symbol.assoc) {
            case Assoc::Left:
                return reduce_action;
            case Assoc::Right:
                return shift_action;
            case Assoc::None:
                return runtime::ParserAction{runtime::ParserActionKind::Error, -1};
        }
        return std::nullopt;
    };

    const auto set_action = [&](int state_id, int terminal, runtime::ParserAction action) {
        auto& row = table.actions[static_cast<std::size_t>(state_id)];
        if (const auto it = row.find(terminal); it != row.end()) {
            if (!same_action(it->second, action)) {
                const auto existing_is_shift = it->second.kind == runtime::ParserActionKind::Shift;
                const auto existing_is_reduce = it->second.kind == runtime::ParserActionKind::Reduce;
                const auto incoming_is_shift = action.kind == runtime::ParserActionKind::Shift;
                const auto incoming_is_reduce = action.kind == runtime::ParserActionKind::Reduce;

                if ((existing_is_shift && incoming_is_reduce) || (existing_is_reduce && incoming_is_shift)) {
                    const auto resolved = existing_is_shift
                        ? resolve_shift_reduce(terminal, it->second, action)
                        : resolve_shift_reduce(terminal, action, it->second);
                    if (resolved.has_value()) {
                        if (resolved->kind == runtime::ParserActionKind::Error) {
                            row.erase(terminal);
                        } else {
                            row[terminal] = *resolved;
                        }
                        return;
                    }
                }

                std::ostringstream message;
                message << "LR conflict in state " << state_id << " on symbol "
                        << grammar.symbols()[static_cast<std::size_t>(terminal)].name
                        << ": existing " << action_name(it->second.kind)
                        << ", new " << action_name(action.kind);
                if ((existing_is_shift && incoming_is_reduce) || (existing_is_reduce && incoming_is_shift)) {
                    const runtime::ParserAction reduce_action = existing_is_reduce ? it->second : action;
                    if (reduce_action.value >= 0 &&
                        static_cast<std::size_t>(reduce_action.value) < grammar.rules().size()) {
                        const auto& reduce_rule =
                            grammar.rules()[static_cast<std::size_t>(reduce_action.value)];
                        const auto& terminal_symbol =
                            grammar.symbols()[static_cast<std::size_t>(terminal)];
                        message << " (shift precedence=" << terminal_symbol.precedence_level
                                << ", reduce precedence=" << reduce_rule.precedence_level
                                << ", assoc=" << assoc_name(terminal_symbol.assoc) << ")";
                    }
                }
                diagnostics.error(message.str());
            }
            return;
        }
        row.emplace(terminal, action);
    };

    for (std::size_t state_index = 0; state_index < automaton.states.size(); ++state_index) {
        const auto& state = automaton.states[state_index];
        const auto& transitions = automaton.transitions[state_index];

        for (const auto& [symbol, target_state] : transitions) {
            const auto& grammar_symbol = grammar.symbols()[static_cast<std::size_t>(symbol)];
            if (grammar_symbol.terminal) {
                set_action(state.id, symbol, {runtime::ParserActionKind::Shift, target_state});
            } else {
                table.gotos[state_index][symbol] = target_state;
            }
        }

        for (const auto& item : state.items) {
            const auto& rule = grammar.rules()[item.rule_index];
            if (item.dot_position != rule.rhs.size()) {
                continue;
            }

            if (item.rule_index == 0 && item.lookahead == eof_symbol->id) {
                set_action(state.id, eof_symbol->id, {runtime::ParserActionKind::Accept, 0});
                continue;
            }

            set_action(state.id, item.lookahead,
                       {runtime::ParserActionKind::Reduce, static_cast<int>(item.rule_index)});
        }
    }

    return table;
}

}  // namespace compilerlab::seuyacc
