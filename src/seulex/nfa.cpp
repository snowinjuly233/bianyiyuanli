#include "seulex/nfa.h"

#include <utility>
#include <vector>

namespace compilerlab::seulex {

namespace {

struct Fragment {
    NFAStateId start {0};
    NFAStateId accept {0};
};

class BuilderContext {
public:
    Fragment build_node(const RegexNode& node) {
        switch (node.kind) {
            case RegexKind::Empty:
                return make_epsilon_fragment();
            case RegexKind::Literal:
                return make_symbol_fragment(node.text);
            case RegexKind::CharacterClass:
                return make_symbol_fragment(node.text);
            case RegexKind::Concat:
                return build_concat(node);
            case RegexKind::Alternate:
                return build_alternate(node);
            case RegexKind::Star:
                return build_star(node);
            case RegexKind::Plus:
                return build_plus(node);
            case RegexKind::Optional:
                return build_optional(node);
        }

        return make_epsilon_fragment();
    }

    NFA finalize(Fragment fragment, std::size_t rule_priority) {
        automaton_.start_state = fragment.start;
        automaton_.accept_state = fragment.accept;
        automaton_.states[static_cast<std::size_t>(fragment.accept)].accepting = true;
        automaton_.states[static_cast<std::size_t>(fragment.accept)].rule_priority = rule_priority;
        return std::move(automaton_);
    }

private:
    NFAStateId add_state() {
        const auto id = static_cast<NFAStateId>(automaton_.states.size());
        automaton_.states.push_back({id, false, 0});
        return id;
    }

    void add_edge(NFAStateId from, NFAStateId to, std::string symbol, bool epsilon) {
        automaton_.edges.push_back({from, to, std::move(symbol), epsilon});
    }

    Fragment make_epsilon_fragment() {
        const auto start = add_state();
        const auto accept = add_state();
        add_edge(start, accept, {}, true);
        return {start, accept};
    }

    Fragment make_symbol_fragment(const std::string& symbol) {
        const auto start = add_state();
        const auto accept = add_state();
        add_edge(start, accept, symbol, false);
        return {start, accept};
    }

    Fragment build_concat(const RegexNode& node) {
        if (node.children.empty()) {
            return make_epsilon_fragment();
        }

        auto current = build_node(*node.children.front());
        for (std::size_t index = 1; index < node.children.size(); ++index) {
            auto next = build_node(*node.children[index]);
            add_edge(current.accept, next.start, {}, true);
            current.accept = next.accept;
        }
        return current;
    }

    Fragment build_alternate(const RegexNode& node) {
        const auto start = add_state();
        const auto accept = add_state();
        for (const auto& child : node.children) {
            auto fragment = build_node(*child);
            add_edge(start, fragment.start, {}, true);
            add_edge(fragment.accept, accept, {}, true);
        }
        return {start, accept};
    }

    Fragment build_star(const RegexNode& node) {
        auto child = build_node(*node.children.front());
        const auto start = add_state();
        const auto accept = add_state();
        add_edge(start, child.start, {}, true);
        add_edge(start, accept, {}, true);
        add_edge(child.accept, child.start, {}, true);
        add_edge(child.accept, accept, {}, true);
        return {start, accept};
    }

    Fragment build_plus(const RegexNode& node) {
        auto child = build_node(*node.children.front());
        const auto start = add_state();
        const auto accept = add_state();
        add_edge(start, child.start, {}, true);
        add_edge(child.accept, child.start, {}, true);
        add_edge(child.accept, accept, {}, true);
        return {start, accept};
    }

    Fragment build_optional(const RegexNode& node) {
        auto child = build_node(*node.children.front());
        const auto start = add_state();
        const auto accept = add_state();
        add_edge(start, child.start, {}, true);
        add_edge(start, accept, {}, true);
        add_edge(child.accept, accept, {}, true);
        return {start, accept};
    }

    NFA automaton_;
};

}  // namespace

NFA NFABuilder::build(const RegexNode& node, std::size_t rule_priority) const {
    BuilderContext context;
    auto fragment = context.build_node(node);
    return context.finalize(fragment, rule_priority);
}

NFA NFABuilder::combine(const std::vector<NFA>& automata) const {
    NFA automaton;
    automaton.start_state = 0;
    automaton.accept_state = -1;
    automaton.states.push_back({0, false, 0});

    int next_id = 1;
    for (const auto& part : automata) {
        const auto offset = next_id;
        for (const auto& state : part.states) {
            auto copy = state;
            copy.id += offset;
            automaton.states.push_back(std::move(copy));
        }
        for (const auto& edge : part.edges) {
            automaton.edges.push_back({
                edge.from + offset,
                edge.to + offset,
                edge.symbol,
                edge.epsilon,
            });
        }
        automaton.edges.push_back({automaton.start_state, part.start_state + offset, {}, true});
        next_id += static_cast<int>(part.states.size());
    }

    return automaton;
}

}  // namespace compilerlab::seulex
