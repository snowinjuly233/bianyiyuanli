# CompilerLab

This repository is the final submission package for a compiler principles lab project.  
本仓库是编译原理课程实验的最终提交版整理包。

It implements a compact compiler front-end toolchain for a frozen MiniC subset, including:  
它实现了一套面向冻结 MiniC 子集的紧凑型编译前端工具链，包括：

- a Lex-like scanner generator  
  类 Lex 的词法分析程序生成器
- a Yacc-like parser generator  
  类 Yacc 的语法分析程序生成器
- AST construction with semantic actions  
  带语义动作的 AST 构造
- semantic analysis  
  语义分析
- three-address IR generation  
  三地址中间代码生成
- basic block construction  
  基本块划分
- regression tests and submission documents  
  回归测试与提交文档

## Supported MiniC Subset

The final language subset is defined by:  
最终语言子集由以下两个规格文件定义：

- [specs/minic.l](./specs/minic.l)
- [specs/minic.y](./specs/minic.y)

Supported features:  
当前支持的语言特性：

- builtin types: `int`, `float`, `void`  
  基本类型：`int`、`float`、`void`
- function definitions  
  函数定义
- global and local variable declarations  
  全局变量和局部变量声明
- optional initializers  
  可选初始化
- `if`, `if ... else`, `while`, `return`  
  `if`、`if ... else`、`while`、`return`
- identifiers, integer literals, float literals  
  标识符、整数字面量、浮点数字面量
- function calls  
  函数调用
- unary `-` and `!`  
  单目运算 `-` 和 `!`
- arithmetic, relational, equality, logical, and assignment expressions  
  算术、关系、相等、逻辑和赋值表达式

Not included:  
当前不支持的内容：

- arrays  
  数组
- pointers  
  指针
- structs  
  结构体
- strings  
  字符串
- `for`  
  `for`
- `break` / `continue`  
  `break` / `continue`
- multiple declarators in one declaration  
  单条声明中的多个声明符

## Repository Layout

- [specs/](./specs): final lexer and parser specifications  
  最终版词法和语法规格
- [src/](./src): generator, runtime, semantic, IR, and tool source code  
  生成器、运行时、语义分析、IR 和工具源码
- [generated/](./generated): generated scanner and parser source files  
  生成出的 scanner / parser 源码
- [tests/](./tests): regression test reports  
  回归测试报告
- [output/](./output): AST, IR, and basic block showcase outputs  
  AST、IR 和 basic block 展示样例
- [docs/](./docs): component reports and the final report  
  分项实验报告和总报告
- [ppt/](./ppt): defense deck, preview images, and PPTX quality report  
  答辩 PPT、预览图和 PPTX 质量检查报告

## Key Outputs

Generated artifacts:  
生成产物：

- [generated/minic_scanner.cpp](./generated/minic_scanner.cpp)
- [generated/minic_parser.cpp](./generated/minic_parser.cpp)
- [generated/dfa.dot](./generated/dfa.dot)

Showcase outputs:  
展示样例：

- [output/minic_demo.ast.md](./output/minic_demo.ast.md)
- [output/feature_coverage.ast.md](./output/feature_coverage.ast.md)
- [output/feature_coverage.ir.txt](./output/feature_coverage.ir.txt)
- [output/feature_coverage.blocks.txt](./output/feature_coverage.blocks.txt)

Reports:  
实验报告：

- [docs/seulex_report.md](./docs/seulex_report.md)
- [docs/seuyacc_report.md](./docs/seuyacc_report.md)
- [docs/semantic_ir_report.md](./docs/semantic_ir_report.md)
- [docs/final_report.md](./docs/final_report.md)

## Test Status

The packaged submission includes regression reports for:  
当前提交包包含以下几类回归测试报告：

- lexical analysis  
  词法分析
- syntax analysis  
  语法分析
- AST generation  
  AST 生成
- semantic analysis  
  语义分析
- IR golden tests  
  IR golden tests
- end-to-end frontend execution  
  前端端到端执行

Entry summary:  
总汇总入口：

- [tests/results/stage6_test_results.md](./tests/results/stage6_test_results.md)

Detailed reports:  
详细报告：

- [tests/lex/results/lex_test_results.md](./tests/lex/results/lex_test_results.md)
- [tests/yacc/results/yacc_test_results.md](./tests/yacc/results/yacc_test_results.md)
- [tests/semantic/results/semantic_test_results.md](./tests/semantic/results/semantic_test_results.md)
- [tests/ir/results/ir_test_results.md](./tests/ir/results/ir_test_results.md)
- [tests/frontend/results/frontend_test_results.md](./tests/frontend/results/frontend_test_results.md)

## Defense Materials

The defense deck and exported preview images are included here:  
答辩材料和导出的预览图在这里：

- [ppt/compiler_lab_defense.pptx](./ppt/compiler_lab_defense.pptx)
- [ppt/quality-report.json](./ppt/quality-report.json)
- [ppt/previews/](./ppt/previews)

## Notes

This repository is organized as a clean submission snapshot rather than a full development workspace.  
这个仓库是为了展示和提交而整理出的干净快照，不是完整的日常开发工作区。

The goal is not full `flex` / `bison` or full C compatibility. The implementation is intentionally scoped to the course-defined MiniC subset and the required front-end pipeline.  
本项目的目标不是完整兼容 `flex` / `bison` 或完整 C 语言，而是面向课程要求，在约定的 MiniC 子集上实现一条完整的前端工具链。
