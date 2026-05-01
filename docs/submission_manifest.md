# 提交物整理清单

## 1. 最终规格

- `specs/minic.l`
- `specs/minic.y`

## 2. 生成器源码

- `src/common`
- `src/runtime`
- `src/seulex`
- `src/seuyacc`
- `src/semantic`
- `src/ir`
- `src/tools`

## 3. 生成产物

- `generated/minic_scanner.h`
- `generated/minic_scanner.cpp`
- `generated/minic_scanner_main.cpp`
- `generated/minic_parser.h`
- `generated/minic_parser.cpp`
- `generated/minic_parser_main.cpp`
- `generated/CMakeLists.txt`
- `generated/dfa.dot`

## 4. 测试与结果

- `tests/lex/results/lex_test_results.md`
- `tests/yacc/results/yacc_test_results.md`
- `tests/semantic/results/semantic_test_results.md`
- `tests/ir/results/ir_test_results.md`
- `tests/ast/results/ast_test_results.md`
- `tests/frontend/results/frontend_test_results.md`
- `tests/results/stage6_test_results.md`

## 5. 展示样例

- `output/minic_demo.ast.txt`
- `output/minic_demo.ast.md`
- `output/feature_coverage.ast.txt`
- `output/feature_coverage.ast.md`
- `output/feature_coverage.ir.txt`
- `output/feature_coverage.blocks.txt`

## 6. 实验报告与答辩材料

- `docs/seulex_report.md`
- `docs/seuyacc_report.md`
- `docs/semantic_ir_report.md`
- `docs/final_report.md`
- `submission/final/ppt/compiler_lab_defense.pptx`

## 7. 打包入口

- `submission/package_submission.ps1`

运行后会把提交所需内容复制到：

- `submission/final/`
