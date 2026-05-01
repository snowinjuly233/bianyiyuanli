# 编译原理课程实验总报告

## 1. 实验目标

本实验要求在冻结的 `MiniC` 子集上实现一套简化版编译前端工具链，覆盖：

- 词法分析程序生成
- 语法分析程序生成
- AST 构造
- 语义分析
- 中间代码生成
- 基本块划分
- 测试与提交物整理

当前仓库已经完成上述链路，并将可交付内容整理到 `submission/` 目录下。

## 2. 最终支持的 MiniC 子集

最终子集统一以 `specs/minic.l` 和 `specs/minic.y` 为准，支持：

- 基本类型：`int`、`float`、`void`
- 顶层单元：函数定义、全局变量声明（可选初始化）
- 语句：块、局部变量声明、表达式语句、`if`、`if ... else`、`while`、`return`
- 表达式：标识符、整数字面量、浮点字面量、括号、函数调用、单目、双目、逻辑、关系、赋值

不支持数组、指针、结构体、字符串、`for`、`break/continue`、多声明符等扩展。

## 3. 系统总体结构

工程目录划分如下：

- `src/common`：跨模块公共设施
- `src/seulex`：词法生成器
- `src/seuyacc`：语法生成器
- `src/runtime`：扫描器/解析器运行时
- `src/semantic`：AST、语义动作执行器、符号表、语义分析
- `src/ir`：三地址码与基本块
- `src/tools`：`seulex`、`seuyacc`、`frontend` 命令行入口
- `specs`：最终提交规格
- `generated`：生成出的 scanner/parser
- `tests`：系统化测试
- `output`：展示样例输出
- `submission`：整理后的提交物

## 4. 各部分完成情况

### 4.1 SeuLex

已完成：

- `.l` 规格解析
- 扩展正规式归一化
- `NFA -> DFA -> 最小化 DFA`
- 独立 scanner 代码生成
- DFA `.dot` 导出
- 系统化 golden tests

详见：`docs/seulex_report.md`

### 4.2 SeuYacc

已完成：

- `.y` 规格解析
- `%token` / `%union` / `%type` 处理
- 优先级和结合性处理
- `LR(1)` 项目集构造
- `LALR(1)` 合并
- action/goto 表生成
- 生成独立 parser 源码
- 运行时语义动作执行

详见：`docs/seuyacc_report.md`

### 4.3 语义与 IR

已完成：

- AST 构造
- AST 纯文本 / Markdown 可视化
- 语义规则检查
- 三地址码生成
- basic block 划分
- 端到端前端联调

详见：`docs/semantic_ir_report.md`

## 5. 测试体系

当前至少覆盖了四大类提交要求：

- 词法测试：`tests/lex`
- 语法测试：`tests/yacc`
- 语义测试：`tests/semantic`
- IR 测试：`tests/ir`

另外还有：

- AST 测试：`tests/ast`
- 端到端前端测试：`tests/frontend`
- 汇总入口：`tests/run_stage6_tests.ps1`

最新汇总结果见：`tests/results/stage6_test_results.md`

## 6. 样例输出

仓库中可直接展示的中间结果包括：

- AST：
  - `output/minic_demo.ast.txt`
  - `output/minic_demo.ast.md`
  - `output/feature_coverage.ast.txt`
  - `output/feature_coverage.ast.md`
- IR：
  - `output/feature_coverage.ir.txt`
- Basic Blocks：
  - `output/feature_coverage.blocks.txt`

这些文件可直接作为答辩演示材料。

## 7. 提交物清单

建议最终提交包含：

- 最终 `.l` / `.y` 文件
- 词法和语法生成器源码
- 语义分析与 IR 源码
- 生成出的 scanner/parser 源码
- 系统化测试用例与测试报告
- AST / IR / basic block 输出样例
- 分项报告与总报告
- 答辩 PPT

本仓库已把这些内容整理到 `submission/final/` 目录。

## 8. 结论

当前项目已经从“课程 skeleton”推进到“可测试、可演示、可提交”的完整实验状态。它并不追求完整 `flex/bison` 或 C 编译器兼容性，而是面向课程要求，在冻结 `MiniC` 子集上实现了一条闭合的前端工具链。
