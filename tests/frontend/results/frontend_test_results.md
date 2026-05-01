# Frontend End-to-End Test Results

| Case | Check | Result |
| --- | --- | --- |
| feature_coverage | exit code | PASS |
| feature_coverage | output contains Tokens: | PASS |
| feature_coverage | output contains AST root: Program | PASS |
| feature_coverage | output contains Semantic analysis: OK | PASS |
| feature_coverage | output contains IR quads: | PASS |
| feature_coverage | output contains Basic blocks: | PASS |
| feature_coverage | output contains ACCEPT | PASS |
| feature_coverage | output file feature_coverage.ast.txt | PASS |
| feature_coverage | output file feature_coverage.ast.md | PASS |
| feature_coverage | output file feature_coverage.ir.txt | PASS |
| feature_coverage | output file feature_coverage.blocks.txt | PASS |
| minic_invalid | exit code | PASS |
| minic_invalid | output contains REJECT | PASS |
| minic_invalid | output contains error: | PASS |
| error_var_redefinition | exit code | PASS |
| error_var_redefinition | output contains AST root: Program | PASS |
| error_var_redefinition | output contains Semantic analysis: FAILED | PASS |
| error_var_redefinition | output contains REJECT | PASS |
| error_var_redefinition | output contains redefinition of variable | PASS |
| error_var_redefinition | output file error_var_redefinition.ast.txt | PASS |
| error_var_redefinition | output file error_var_redefinition.ast.md | PASS |
