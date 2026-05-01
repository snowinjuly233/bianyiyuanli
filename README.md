# CompilerLab

This repository is the final submission package for a compiler principles lab project.

It implements a compact compiler front-end toolchain for a frozen MiniC subset, including:

- a Lex-like scanner generator
- a Yacc-like parser generator
- AST construction with semantic actions
- semantic analysis
- three-address IR generation
- basic block construction
- regression tests and submission documents

## Supported MiniC Subset

The final language subset is defined by:

- [specs/minic.l](./specs/minic.l)
- [specs/minic.y](./specs/minic.y)

Supported features:

- builtin types: `int`, `float`, `void`
- function definitions
- global and local variable declarations
- optional initializers
- `if`, `if ... else`, `while`, `return`
- identifiers, integer literals, float literals
- function calls
- unary `-` and `!`
- arithmetic, relational, equality, logical, and assignment expressions

Not included:

- arrays
- pointers
- structs
- strings
- `for`
- `break` / `continue`
- multiple declarators in one declaration

## Repository Layout

- [specs/](./specs): final lexer and parser specifications
- [src/](./src): generator, runtime, semantic, IR, and tool source code
- [generated/](./generated): generated scanner and parser source files
- [tests/](./tests): regression test reports
- [output/](./output): AST, IR, and basic block showcase outputs
- [docs/](./docs): component reports and the final report
- [ppt/](./ppt): defense deck, preview images, and PPTX quality report

## Key Outputs

Generated artifacts:

- [generated/minic_scanner.cpp](./generated/minic_scanner.cpp)
- [generated/minic_parser.cpp](./generated/minic_parser.cpp)
- [generated/dfa.dot](./generated/dfa.dot)

Showcase outputs:

- [output/minic_demo.ast.md](./output/minic_demo.ast.md)
- [output/feature_coverage.ast.md](./output/feature_coverage.ast.md)
- [output/feature_coverage.ir.txt](./output/feature_coverage.ir.txt)
- [output/feature_coverage.blocks.txt](./output/feature_coverage.blocks.txt)

Reports:

- [docs/seulex_report.md](./docs/seulex_report.md)
- [docs/seuyacc_report.md](./docs/seuyacc_report.md)
- [docs/semantic_ir_report.md](./docs/semantic_ir_report.md)
- [docs/final_report.md](./docs/final_report.md)

## Test Status

The packaged submission includes regression reports for:

- lexical analysis
- syntax analysis
- AST generation
- semantic analysis
- IR golden tests
- end-to-end frontend execution

Entry summary:

- [tests/results/stage6_test_results.md](./tests/results/stage6_test_results.md)

Detailed reports:

- [tests/lex/results/lex_test_results.md](./tests/lex/results/lex_test_results.md)
- [tests/yacc/results/yacc_test_results.md](./tests/yacc/results/yacc_test_results.md)
- [tests/semantic/results/semantic_test_results.md](./tests/semantic/results/semantic_test_results.md)
- [tests/ir/results/ir_test_results.md](./tests/ir/results/ir_test_results.md)
- [tests/frontend/results/frontend_test_results.md](./tests/frontend/results/frontend_test_results.md)

## Defense Materials

The defense deck and exported preview images are included here:

- [ppt/compiler_lab_defense.pptx](./ppt/compiler_lab_defense.pptx)
- [ppt/quality-report.json](./ppt/quality-report.json)
- [ppt/previews/](./ppt/previews)

## Notes

This repository is organized as a clean submission snapshot rather than a full development workspace.

The goal is not full `flex` / `bison` or full C compatibility. The implementation is intentionally scoped to the course-defined MiniC subset and the required front-end pipeline.
