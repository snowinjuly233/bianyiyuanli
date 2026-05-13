#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common/diagnostic.h"
#include "common/source.h"
#include "common/string_utils.h"
#include "common/token.h"
#include "ir/basic_block.h"
#include "ir/ir_builder.h"
#include "ir/ir_printer.h"
#include "runtime/parser_runtime.h"
#include "runtime/scanner_runtime.h"
#include "semantic/ast_printer.h"
#include "semantic/semantic_analyzer.h"
#include "semantic/semantic_action_executor.h"
#include "seulex/dfa.h"
#include "seulex/dfa_minimizer.h"
#include "seulex/lex_spec_parser.h"
#include "seulex/nfa.h"
#include "seulex/regex_normalizer.h"
#include "seulex/regex_parser.h"
#include "seuyacc/grammar.h"
#include "seuyacc/lalr.h"
#include "seuyacc/lr1.h"
#include "seuyacc/parse_table.h"
#include "seuyacc/yacc_spec_parser.h"

namespace {

struct GrammarBundle {
    compilerlab::seuyacc::Grammar grammar;
    std::unordered_map<int, compilerlab::seuyacc::SymbolId> token_symbols;
    std::vector<compilerlab::runtime::ProductionInfo> productions;
};

std::string severity_name(compilerlab::common::DiagnosticSeverity severity) {
    using compilerlab::common::DiagnosticSeverity;
    switch (severity) {
        case DiagnosticSeverity::Note: return "note";
        case DiagnosticSeverity::Warning: return "warning";
        case DiagnosticSeverity::Error: return "error";
    }
    return "unknown";
}

void print_diagnostics(const compilerlab::common::DiagnosticEngine& diagnostics) {
    for (const auto& item : diagnostics.diagnostics()) {
        std::cerr << severity_name(item.severity) << ": " << item.message;
        if (!item.span.file_path.empty()) {
            std::cerr << " (" << item.span.file_path << ":" << item.span.begin.line
                      << ":" << item.span.begin.column << ")";
        }
        std::cerr << '\n';
    }
}

std::size_t count_diagnostics(const compilerlab::common::DiagnosticEngine& diagnostics,
                              compilerlab::common::DiagnosticSeverity severity) {
    std::size_t count = 0;
    for (const auto& item : diagnostics.diagnostics()) {
        if (item.severity == severity) {
            ++count;
        }
    }
    return count;
}

std::string read_file_or_die(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to open input file: " + path);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

void write_file_or_die(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("failed to open output file for writing: " + path.string());
    }
    output << content;
}

std::vector<compilerlab::runtime::ScannerState> build_scanner_states(
    const compilerlab::seulex::DFA& dfa,
    const std::vector<compilerlab::seulex::LexRule>& rules) {
    std::vector<compilerlab::runtime::ScannerState> states;
    states.resize(dfa.states.size());

    for (const auto& state : dfa.states) {
        auto& runtime_state = states[static_cast<std::size_t>(state.id)];
        runtime_state.id = state.id;
        if (state.accepting && state.rule_priority < rules.size()) {
            const auto& rule = rules[state.rule_priority];
            runtime_state.accept = compilerlab::runtime::ScannerAcceptAction{
                rule.token_kind,
                rule.priority,
                rule.action_kind == compilerlab::seulex::LexActionKind::Skip,
            };
        }
    }

    for (const auto& transition : dfa.transitions) {
        states[static_cast<std::size_t>(transition.from)].transitions[transition.symbol] = transition.to;
    }

    return states;
}

std::vector<compilerlab::common::Token> scan_tokens(
    const std::string& lex_spec_path,
    const std::string& input_path,
    compilerlab::common::SourceManager& sources,
    compilerlab::common::DiagnosticEngine& diagnostics) {
    compilerlab::seulex::LexSpecParser spec_parser;
    compilerlab::seulex::RegexNormalizer normalizer;
    compilerlab::seulex::RegexParser regex_parser;
    compilerlab::seulex::NFABuilder nfa_builder;
    compilerlab::seulex::DFABuilder dfa_builder;
    compilerlab::seulex::DFAMinimizer minimizer;

    auto spec = spec_parser.parse_file(lex_spec_path, sources, diagnostics);
    if (diagnostics.has_error()) {
        return {};
    }

    std::unordered_map<std::string, std::string> definitions;
    for (const auto& definition : spec.definitions) {
        definitions[definition.name] = definition.pattern;
    }

    std::vector<compilerlab::seulex::NFA> automata;
    automata.reserve(spec.rules.size());
    for (auto& rule : spec.rules) {
        rule.normalized_regex = normalizer.normalize(rule.regex_text, definitions, diagnostics);
        auto regex = regex_parser.parse(rule.normalized_regex, diagnostics);
        if (!regex) {
            diagnostics.error("failed to parse normalized regex: " + rule.normalized_regex);
            continue;
        }
        automata.push_back(nfa_builder.build(*regex, rule.priority));
    }

    if (diagnostics.has_error()) {
        return {};
    }

    const auto combined_nfa = nfa_builder.combine(automata);
    const auto dfa = dfa_builder.build(combined_nfa);
    const auto minimized_dfa = minimizer.minimize(dfa);
    compilerlab::runtime::ScannerRuntime scanner(build_scanner_states(minimized_dfa, spec.rules));

    const auto input = read_file_or_die(input_path);
    std::vector<compilerlab::common::Token> tokens;
    std::size_t offset = 0;
    while (true) {
        auto token = scanner.scan_one(input, offset, input_path);
        if (token.kind == compilerlab::common::TokenKind::Invalid) {
            diagnostics.error("invalid token `" + compilerlab::common::escape_text(token.lexeme) + "`",
                              token.span);
            return {};
        }

        tokens.push_back(token);
        if (token.kind == compilerlab::common::TokenKind::EndOfFile) {
            break;
        }
    }

    return tokens;
}

std::string join_symbols(const std::vector<std::string>& symbols) {
    if (symbols.empty()) {
        return "/* empty */";
    }

    std::string text;
    for (std::size_t index = 0; index < symbols.size(); ++index) {
        if (index > 0) {
            text += ' ';
        }
        text += symbols[index];
    }
    return text;
}

GrammarBundle build_grammar(const compilerlab::seuyacc::YaccSpec& spec,
                            compilerlab::common::DiagnosticEngine& diagnostics) {
    GrammarBundle bundle;
    auto& grammar = bundle.grammar;

    struct PrecedenceInfo {
        int level {0};
        compilerlab::seuyacc::Assoc assoc {compilerlab::seuyacc::Assoc::None};
    };

    std::unordered_map<std::string, PrecedenceInfo> precedence_by_token;
    for (std::size_t level = 0; level < spec.precedences.size(); ++level) {
        const auto precedence_level = static_cast<int>(level + 1);
        for (const auto& token_name : spec.precedences[level].tokens) {
            precedence_by_token[token_name] = {precedence_level, spec.precedences[level].assoc};
        }
    }

    for (const auto& token : spec.tokens) {
        const auto symbol_id = grammar.add_terminal(token.name);
        if (const auto precedence = precedence_by_token.find(token.name);
            precedence != precedence_by_token.end()) {
            grammar.set_symbol_precedence(symbol_id, precedence->second.level, precedence->second.assoc);
        }

        if (const auto token_kind = compilerlab::common::token_kind_from_name(token.name);
            token_kind.has_value()) {
            bundle.token_symbols[static_cast<int>(*token_kind)] = symbol_id;
        }
    }

    for (const auto& [token_name, precedence] : precedence_by_token) {
        const auto symbol_id = grammar.add_terminal(token_name);
        grammar.set_symbol_precedence(symbol_id, precedence.level, precedence.assoc);
    }

    const auto eof_symbol = grammar.add_terminal("EndOfFile");
    bundle.token_symbols[static_cast<int>(compilerlab::common::TokenKind::EndOfFile)] = eof_symbol;

    std::unordered_set<std::string> nonterminal_names;
    for (const auto& production : spec.productions) {
        nonterminal_names.insert(production.lhs);
    }

    for (const auto& name : nonterminal_names) {
        grammar.add_nonterminal(name);
    }

    if (spec.start_symbol.empty()) {
        diagnostics.error("yacc specification is missing a start symbol");
        return bundle;
    }

    const auto* start_symbol = grammar.lookup_symbol(spec.start_symbol);
    if (start_symbol == nullptr) {
        diagnostics.error("start symbol is not defined by any production: " + spec.start_symbol);
        return bundle;
    }
    const auto start_symbol_id = start_symbol->id;

    std::string augmented_name = "__start";
    while (grammar.lookup_symbol(augmented_name) != nullptr) {
        augmented_name += "_";
    }

    const auto augmented_symbol = grammar.add_nonterminal(augmented_name);
    grammar.set_start_symbol(augmented_symbol);
    grammar.add_rule(augmented_symbol, {start_symbol_id}, 0);
    bundle.productions.push_back(
        {augmented_symbol, 1, augmented_name + " -> " + spec.start_symbol, "$$ = $1;"});

    for (const auto& production : spec.productions) {
        const auto* lhs_symbol = grammar.lookup_symbol(production.lhs);
        if (lhs_symbol == nullptr) {
            diagnostics.error("unknown production head: " + production.lhs);
            continue;
        }

        std::vector<compilerlab::seuyacc::SymbolId> rhs_symbols;
        rhs_symbols.reserve(production.rhs.size());
        for (const auto& symbol_name : production.rhs) {
            const auto* symbol = grammar.lookup_symbol(symbol_name);
            if (symbol == nullptr) {
                diagnostics.error("unknown grammar symbol in production `" + production.lhs +
                                  "`: " + symbol_name);
                continue;
            }
            rhs_symbols.push_back(symbol->id);
        }

        const auto production_index = bundle.productions.size();
        int rule_precedence_level = 0;
        compilerlab::seuyacc::Assoc rule_assoc = compilerlab::seuyacc::Assoc::None;

        if (!production.precedence_token.empty()) {
            if (const auto precedence = precedence_by_token.find(production.precedence_token);
                precedence != precedence_by_token.end()) {
                rule_precedence_level = precedence->second.level;
                rule_assoc = precedence->second.assoc;
            } else {
                diagnostics.error("unknown precedence token in production `" + production.lhs +
                                  "`: " + production.precedence_token);
            }
        } else {
            for (auto it = rhs_symbols.rbegin(); it != rhs_symbols.rend(); ++it) {
                const auto& symbol = grammar.symbols()[static_cast<std::size_t>(*it)];
                if (symbol.terminal && symbol.precedence_level > 0) {
                    rule_precedence_level = symbol.precedence_level;
                    rule_assoc = symbol.assoc;
                    break;
                }
            }
        }

        grammar.add_rule(lhs_symbol->id, rhs_symbols, production_index,
                         rule_precedence_level, rule_assoc);
        bundle.productions.push_back(
            {lhs_symbol->id, static_cast<int>(rhs_symbols.size()),
             production.lhs + " -> " + join_symbols(production.rhs),
             production.action_text});
    }

    return bundle;
}

int run(int argc, char** argv) {
    const std::string input_path = argc > 1 ? argv[1] : "specs/samples/minic_demo.c";
    const std::string lex_spec_path = argc > 2 ? argv[2] : "specs/minic.l";
    const std::string yacc_spec_path = argc > 3 ? argv[3] : "specs/minic.y";
    const std::string output_dir = argc > 4 ? argv[4] : "output";

    compilerlab::common::SourceManager sources;
    compilerlab::common::DiagnosticEngine diagnostics;
    compilerlab::seuyacc::YaccSpecParser spec_parser;
    compilerlab::seuyacc::LR1Builder lr1_builder;
    compilerlab::seuyacc::LALRMerger lalr_merger;
    compilerlab::seuyacc::ParseTableBuilder table_builder;
    compilerlab::runtime::ParserRuntime parser_runtime;
    compilerlab::semantic::AstPrinter ast_printer;
    compilerlab::semantic::SemanticActionExecutor action_executor(diagnostics);
    compilerlab::semantic::SemanticAnalyzer semantic_analyzer(diagnostics);
    compilerlab::ir::IRBuilder ir_builder;
    compilerlab::ir::IRPrinter ir_printer;
    compilerlab::ir::BasicBlockPrinter basic_block_printer;

    const auto yacc_spec = spec_parser.parse_file(yacc_spec_path, sources, diagnostics);
    if (diagnostics.has_error()) {
        print_diagnostics(diagnostics);
        return 1;
    }

    const auto grammar_bundle = build_grammar(yacc_spec, diagnostics);
    if (diagnostics.has_error()) {
        print_diagnostics(diagnostics);
        return 1;
    }

    const auto lr1_automaton = lr1_builder.build(grammar_bundle.grammar, diagnostics);
    const auto lalr_automaton = lalr_merger.merge(lr1_automaton);
    const auto parse_table = table_builder.build(grammar_bundle.grammar, lalr_automaton, diagnostics);
    if (diagnostics.has_error()) {
        print_diagnostics(diagnostics);
        return 1;
    }

    const auto tokens = scan_tokens(lex_spec_path, input_path, sources, diagnostics);
    if (diagnostics.has_error()) {
        print_diagnostics(diagnostics);
        return 1;
    }

    std::size_t reduction_count = 0;
    const auto semantic_result = parser_runtime.parse(
        tokens,
        parse_table.actions,
        parse_table.gotos,
        grammar_bundle.productions,
        [&](const compilerlab::common::TokenKind kind) {
            const auto it = grammar_bundle.token_symbols.find(static_cast<int>(kind));
            if (it == grammar_bundle.token_symbols.end()) {
                return -1;
            }
            return it->second;
        },
        [&](int production_index, const std::vector<compilerlab::semantic::SemanticValue>& rhs_values) {
            if (production_index > 0 &&
                static_cast<std::size_t>(production_index) < grammar_bundle.productions.size()) {
                ++reduction_count;
            }
            return action_executor.execute(
                grammar_bundle.productions[static_cast<std::size_t>(production_index)],
                rhs_values);
        },
        diagnostics);

    std::cout << "Input: " << std::filesystem::path(input_path).lexically_normal().string() << '\n';
    std::cout << "Tokens: " << (tokens.empty() ? 0 : tokens.size() - 1) << '\n';
    std::cout << "LR(1) states: " << lr1_automaton.states.size() << '\n';
    std::cout << "LALR(1) states: " << lalr_automaton.states.size() << '\n';
    std::cout << "Reductions: " << reduction_count << '\n';

    if (diagnostics.has_error()) {
        std::cout << "REJECT\n";
        print_diagnostics(diagnostics);
        return 1;
    }

    const auto* program = semantic_result.get_if<compilerlab::semantic::ProgramPtr>();
    if (program == nullptr || !*program) {
        diagnostics.error("parser accepted input but did not produce a program AST");
        std::cout << "REJECT\n";
        print_diagnostics(diagnostics);
        return 1;
    }

    std::cout << "AST root: "
              << compilerlab::semantic::to_string((*program)->kind)
              << '\n';

    const auto input_stem = std::filesystem::path(input_path).stem().string();
    const auto output_root = std::filesystem::path(output_dir);
    const auto ast_text_path =
        output_root / (input_stem.empty() ? "frontend.ast.txt" : input_stem + ".ast.txt");
    const auto ast_markdown_path =
        output_root / (input_stem.empty() ? "frontend.ast.md" : input_stem + ".ast.md");
    std::filesystem::create_directories(output_root);
    write_file_or_die(ast_text_path, ast_printer.print_text(*program));
    write_file_or_die(ast_markdown_path, ast_printer.print_markdown(*program));
    std::cout << "AST text: " << ast_text_path.lexically_normal().string() << '\n';
    std::cout << "AST markdown: " << ast_markdown_path.lexically_normal().string() << '\n';

    const auto semantic_ok = semantic_analyzer.analyze(*program);
    const auto warning_count = count_diagnostics(diagnostics, compilerlab::common::DiagnosticSeverity::Warning);
    std::cout << "Semantic analysis: " << (semantic_ok ? "OK" : "FAILED") << '\n';
    std::cout << "Warnings: " << warning_count << '\n';

    if (diagnostics.has_error()) {
        std::cout << "REJECT\n";
        print_diagnostics(diagnostics);
        return 1;
    }

    const auto quads = ir_builder.build(*program, diagnostics);
    if (diagnostics.has_error()) {
        std::cout << "REJECT\n";
        print_diagnostics(diagnostics);
        return 1;
    }

    const auto ir_text = ir_printer.print(quads);
    const auto ir_output_path =
        output_root / (input_stem.empty() ? "frontend.ir.txt" : input_stem + ".ir.txt");
    const auto basic_blocks = compilerlab::ir::build_basic_blocks(quads);
    const auto block_output_path =
        output_root / (input_stem.empty() ? "frontend.blocks.txt" : input_stem + ".blocks.txt");
    std::filesystem::create_directories(ir_output_path.parent_path());
    write_file_or_die(ir_output_path, ir_text);
    write_file_or_die(block_output_path, basic_block_printer.print(basic_blocks));

    std::cout << "IR quads: " << quads.size() << '\n';
    std::cout << "Basic blocks: " << basic_blocks.size() << '\n';
    std::cout << "IR output: " << ir_output_path.lexically_normal().string() << '\n';
    std::cout << "Block output: " << block_output_path.lexically_normal().string() << '\n';
    std::cout << "ACCEPT\n";
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        return run(argc, argv);
    } catch (const std::exception& ex) {
        std::cerr << "fatal: " << ex.what() << '\n';
        return 1;
    }
}
