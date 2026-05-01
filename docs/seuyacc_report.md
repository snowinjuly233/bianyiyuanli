# SeuYacc 语法实验报告

## 1. 实验范围

本阶段提交的是 `MiniC` 冻结子集上的 `SeuYacc` 实现。最终提交语法规格文件为 `specs/minic.y`，它同时承担三项职责：

- 描述冻结后的 `MiniC` 语法子集
- 用 `%union` 和 `%type` 固化语义值类型
- 用动作块给出最终的 translation scheme

当前 `specs/minic.y` 已经与词法、AST、语义分析和 IR 生成链路统一，不再是“占位文法”。

## 2. 最终提交的 `.y` 文件

最终提交规格文件：`specs/minic.y`

它覆盖的冻结子集包括：

- 顶层单元：函数定义、带可选初始化的全局变量声明
- 局部语句：块、局部变量声明、表达式语句、`if`、`if ... else`、`while`、`return`
- 表达式：标识符、整数字面量、浮点字面量、括号表达式、函数调用、单目 `-` / `!`、乘加、关系、相等、逻辑与或、赋值

该文件同时包含完整语义动作，例如：

- `make_program`
- `make_function_decl`
- `make_global_var_decl`
- `make_local_decl_stmt`
- `make_if_stmt`
- `make_while_stmt`
- `make_binary_expr`
- `make_call_expr`

这些动作并不是注释性文本，而是会在规约时由 `SemanticActionExecutor` 执行。

## 3. 关键数据结构

### 3.1 语法规格表示

- `YaccProduction`
  - 保存 `lhs`、`rhs`、`precedence_token`、`action_text`
- `YaccSpec`
  - 保存 `%token`、`%union`、`%type`、优先级、开始符号和全部产生式

### 3.2 文法与分析表

- `Grammar`
  - 管理终结符、非终结符、规则、优先级和开始符号
- `LR1Builder`
  - 构造 `LR(1)` 项目集规范族
- `LALRMerger`
  - 合并同核项目集，得到 `LALR(1)` 自动机
- `ParseTableBuilder`
  - 生成 action/goto 分析表

### 3.3 运行时规约接口

- `ProductionInfo`
  - 保存 `lhs_symbol`、`rhs_size`、`debug_name`、`action_text`
- `ParserRuntime`
  - 按分析表驱动移进/规约流程
- `SemanticActionExecutor`
  - 解释执行 `action_text`，构造 AST 节点或列表语义值

## 4. 算法流程

`SeuYacc` 的主流程如下：

1. 解析 `.y` 文件，构造 `YaccSpec`
2. 建立终结符、非终结符和产生式，得到 `Grammar`
3. 计算 `FIRST` 集并构造 `LR(1)` 项目集规范族
4. 合并同核项目集，得到 `LALR(1)` 自动机
5. 生成 action/goto 分析表
6. 将产生式和 `action_text` 一并固化到运行时 `ProductionInfo`
7. 解析 token 流，在规约时调用 `SemanticActionExecutor`
8. 构造 `ProgramPtr` AST，并交给后续语义/IR 链路

## 5. 语义动作与生成器一致性

当前工程已经完成两条链路的一致化：

- 手写 `frontend` 链路会执行 `.y` 中的语义动作并构造 AST
- 生成出来的独立 parser 也会复用 `ParserRuntime + SemanticActionExecutor` 执行动作

也就是说，`generated/minic_parser.cpp` 不再只是“接受/拒绝演示程序”，而是真正和主工程共享语义动作语义。

## 6. 当前分析规模

基于当前最终版 `specs/minic.y`，实测结果为：

- `LR(1)` 状态数：253
- `LALR(1)` 状态数：116

对应输出可见：

- `tests/generated/results/generated_parser.stdout.txt`
- `tests/frontend/results/feature_coverage.stdout.txt`

## 7. 系统测试设计

本项目为 `Yacc` 侧补充了系统化测试：

- 合法样例接受测试
- 非法样例拒绝测试
- 独立生成 parser 构造 AST 测试
- 主工程 `frontend` 端到端接入测试

测试脚本：

- `tests/yacc/run_yacc_tests.ps1`
- `tests/generated/run_generated_parser_tests.ps1`
- `tests/frontend/run_frontend_tests.ps1`

## 8. 测试结果

当前回归测试均已通过：

- `SeuYacc` 合法输入接受：PASS
- `SeuYacc` 非法输入拒绝：PASS
- 生成 parser 返回 `AST root: Program`：PASS
- 端到端 `frontend` 输出状态数、AST、语义结果和 IR：PASS

汇总报告见：

- `tests/yacc/results/yacc_test_results.md`
- `tests/generated/results/generated_parser_test_results.md`
- `tests/frontend/results/frontend_test_results.md`

## 9. 结论

当前 `SeuYacc` 已经满足课程实验在冻结 `MiniC` 子集上的要求：

- 有最终提交版 `.y` 文件
- 有 `%union` / `%type` / 优先级 / 悬挂 `else` 处理
- 有 `LR(1)` 到 `LALR(1)` 的分析表生成链路
- 有独立可编译的 parser 生成结果
- 有语义动作执行链路
- 有系统化测试与回归报告

仍然保留的边界是：本实现服务于课程实验定义的 `MiniC` 子集，而不是完整的 `bison` 兼容实现。
