#include <filesystem>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/diagnostic.h"
#include "common/source.h"
#include "common/string_utils.h"
#include "common/token.h"
#include "runtime/scanner_runtime.h"
#include "seulex/dfa.h"
#include "seulex/dfa_minimizer.h"
#include "seulex/lex_spec_parser.h"
#include "seulex/nfa.h"
#include "seulex/regex_normalizer.h"
#include "seulex/regex_parser.h"
#include "seulex/scanner_emitter.h"

namespace {

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

std::string read_file_or_die(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to open input file: " + path);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
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

int run(int argc, char** argv) {
    const std::string spec_path = argc > 1 ? argv[1] : "specs/minic.l";
    const std::string input_path = argc > 2 ? argv[2] : "specs/samples/minic_demo.c";
    const std::string output_dir = argc > 3 ? argv[3] : "generated";
    const auto scanner_name = std::filesystem::path(spec_path).stem().string();

    compilerlab::common::SourceManager sources;
    compilerlab::common::DiagnosticEngine diagnostics;
    compilerlab::seulex::LexSpecParser spec_parser;
    compilerlab::seulex::RegexNormalizer normalizer;
    compilerlab::seulex::RegexParser regex_parser;
    compilerlab::seulex::NFABuilder nfa_builder;
    compilerlab::seulex::DFABuilder dfa_builder;
    compilerlab::seulex::DFAMinimizer minimizer;
    compilerlab::seulex::ScannerEmitter emitter;

    auto spec = spec_parser.parse_file(spec_path, sources, diagnostics);
    if (diagnostics.has_error()) {
        print_diagnostics(diagnostics);
        return 1;
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
        print_diagnostics(diagnostics);
        return 1;
    }

    auto combined_nfa = nfa_builder.combine(automata);
    auto dfa = dfa_builder.build(combined_nfa);
    auto minimized_dfa = minimizer.minimize(dfa);

    if (!emitter.emit(minimized_dfa, spec.rules, output_dir, scanner_name, diagnostics)) {
        print_diagnostics(diagnostics);
        return 1;
    }

    const auto input = read_file_or_die(input_path);
    compilerlab::runtime::ScannerRuntime scanner(build_scanner_states(minimized_dfa, spec.rules));

    std::size_t offset = 0;
    while (true) {
        auto token = scanner.scan_one(input, offset, input_path);
        if (token.kind == compilerlab::common::TokenKind::Invalid) {
            std::cerr << "error: invalid token starting at " << token.span.begin.line
                      << ":" << token.span.begin.column << " -> `" << token.lexeme << "`\n";
            return 1;
        }

        if (token.kind == compilerlab::common::TokenKind::EndOfFile) {
            std::cout << "EndOfFile\n";
            break;
        }

        std::cout << compilerlab::common::to_string(token.kind)
                  << " \"" << compilerlab::common::escape_text(token.lexeme) << "\""
                  << " @" << token.span.begin.line << ":" << token.span.begin.column
                  << '\n';
    }

    if (diagnostics.has_error()) {
        print_diagnostics(diagnostics);
        return 1;
    }

    std::cout << "DFA states: " << dfa.states.size()
              << " -> " << minimized_dfa.states.size() << '\n';
    std::cout << "Generated scanner: "
              << (std::filesystem::path(output_dir) /
                  (scanner_name + "_scanner.h")).lexically_normal().string()
              << '\n';
    std::cout << "Generated scanner: "
              << (std::filesystem::path(output_dir) /
                  (scanner_name + "_scanner.cpp")).lexically_normal().string()
              << '\n';
    std::cout << "Generated scanner: "
              << (std::filesystem::path(output_dir) /
                  (scanner_name + "_scanner_main.cpp")).lexically_normal().string()
              << '\n';
    std::cout << "Generated build file: "
              << (std::filesystem::path(output_dir) / "CMakeLists.txt").lexically_normal().string()
              << '\n';
    std::cout << "DFA dot written to "
              << (std::filesystem::path(output_dir) / "dfa.dot").lexically_normal().string()
              << '\n';
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
