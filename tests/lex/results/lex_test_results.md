# SeuLex Test Results

- Generated at: 2026-05-01 09:38:15
- Total cases: 6
- Passed: 6
- Failed: 0

## Generator And Minimization Summary

| Spec | Original DFA States | Minimized DFA States | Standalone Scanner Build | Output Directory |
| --- | ---: | ---: | --- | --- |
| `specs/minic.l` | 61 | 58 | success | `tests/lex/results/generated/minic` |
| `specs/lex_features.l` | 27 | 25 | success | `tests/lex/results/generated/feature_subset` |

## Case Results

| Suite | Case | Focus | Expected Exit Code | Actual Exit Code | Result |
| --- | --- | --- | ---: | ---: | --- |
| minic | keywords_and_identifiers | keyword priority and identifier fallback | 0 | 0 | PASS |
| minic | operators_and_delimiters | longest match for operators and delimiter coverage | 0 | 0 | PASS |
| minic | literals_and_layout | integer or float literals plus line and column tracking | 0 | 0 | PASS |
| minic | bom_input | UTF-8 BOM compatibility | 0 | 0 | PASS |
| minic | invalid_char | negative case for unsupported characters | 1 | 1 | PASS |
| feature_subset | feature_mix | comment skip, wildcard, negated class, and repetition bounds | 0 | 0 | PASS |

## Raw Output Files

- `tests/lex/results/keywords_and_identifiers.actual.txt`
- `tests/lex/results/operators_and_delimiters.actual.txt`
- `tests/lex/results/literals_and_layout.actual.txt`
- `tests/lex/results/bom_input.actual.txt`
- `tests/lex/results/invalid_char.actual.txt`
- `tests/lex/results/feature_mix.actual.txt`

## Generated Artifacts

- minic
  - `tests/lex/results/generated/minic/minic_scanner.h`
  - `tests/lex/results/generated/minic/minic_scanner.cpp`
  - `tests/lex/results/generated/minic/minic_scanner_main.cpp`
  - `tests/lex/results/generated/minic/CMakeLists.txt`
  - `tests/lex/results/generated/minic/dfa.dot`
- feature_subset
  - `tests/lex/results/generated/feature_subset/lex_features_scanner.h`
  - `tests/lex/results/generated/feature_subset/lex_features_scanner.cpp`
  - `tests/lex/results/generated/feature_subset/lex_features_scanner_main.cpp`
  - `tests/lex/results/generated/feature_subset/CMakeLists.txt`
  - `tests/lex/results/generated/feature_subset/dfa.dot`
