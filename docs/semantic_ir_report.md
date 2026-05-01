# 语义分析与中间代码实验报告

## 1. 实验范围

本阶段的目标是把前端链路从“词法 + 语法接受/拒绝”补全到：

`scan -> parse -> AST -> semantic analyze -> IR -> basic blocks`

当前工程已经完成上述完整链路，并通过 `frontend` 统一串联。

## 2. AST 与语义动作执行

`specs/minic.y` 中的动作块由 `SemanticActionExecutor` 在规约时执行，最终构造：

- `Program`
- `FunctionDecl`
- `VarDecl`
- `BlockStmt`
- `IfStmt`
- `WhileStmt`
- `ReturnStmt`
- `AssignExpr`
- `BinaryExpr`
- `UnaryExpr`
- `CallExpr`

同时工程新增了 `AstPrinter`，可直接输出：

- 纯文本树：`*.ast.txt`
- Markdown 树：`*.ast.md`

## 3. 语义分析能力

当前 `SemanticAnalyzer` 已覆盖以下错误检查：

- 变量重定义
- 参数重定义
- 函数重定义
- 未声明标识符
- 调用未声明函数
- 调用非函数对象
- 参数个数不匹配
- 参数类型不匹配
- 赋值类型不匹配
- 初始化类型不匹配
- `if` 条件类型检查
- `while` 条件类型检查
- `void` 函数返回值错误
- 非 `void` 函数缺少返回值
- 返回类型不匹配

作用域方面已经明确支持：

- 全局作用域
- 函数参数作用域
- 块级局部作用域
- 合法遮蔽（shadowing）

## 4. 中间代码与基本块

当前 IR 采用三地址码四元式风格，覆盖冻结子集中的：

- 全局变量初始化
- 局部变量初始化
- 赋值
- 单目/双目表达式
- `if/else`
- `while`
- `return`
- 函数调用

控制流已经做过修正：

- `if` 会正确跳转到 `else` / `end`
- `while` 会正确生成回边和退出边
- 函数体不再重复追加无意义 `ret`

同时，工程已经补上 basic block 划分，并输出：

- `*.ir.txt`
- `*.blocks.txt`

## 5. 典型输出样例

当前仓库中可直接展示的样例有：

- `output/minic_demo.ast.txt`
- `output/minic_demo.ast.md`
- `output/feature_coverage.ast.txt`
- `output/feature_coverage.ast.md`
- `output/feature_coverage.ir.txt`
- `output/feature_coverage.blocks.txt`

其中 `feature_coverage` 样例覆盖了：

- 全局初始化
- 局部初始化
- 函数调用
- `if/else`
- `while`
- 返回语句

## 6. 系统测试设计

### 6.1 AST 测试

脚本：`tests/ast/run_ast_tests.ps1`

检查内容：

- `frontend` 是否成功接受合法输入
- AST 根是否为 `Program`
- 关键节点是否出现在 `.ast.txt`
- Markdown AST 是否成功生成

### 6.2 语义测试

脚本：`tests/semantic/run_semantic_tests.ps1`

当前已覆盖：

- 2 个合法样例
- 15 个语义错误样例

### 6.3 IR Golden Tests

脚本：`tests/ir/run_ir_tests.ps1`

通过固定 `expected` 文件校验：

- `feature_coverage.ir.txt`
- `feature_coverage.blocks.txt`

### 6.4 端到端测试

脚本：`tests/frontend/run_frontend_tests.ps1`

验证：

- 合法输入完整走通 `词法 -> 语法 -> AST -> 语义 -> IR`
- 非法语法输入正确 `REJECT`
- 语义错误输入正确 `REJECT`

## 7. 测试结果

当前结果如下：

- AST 测试：PASS
- 语义测试：PASS
- IR Golden Test：PASS
- Frontend 端到端测试：PASS

对应报告见：

- `tests/ast/results/ast_test_results.md`
- `tests/semantic/results/semantic_test_results.md`
- `tests/ir/results/ir_test_results.md`
- `tests/frontend/results/frontend_test_results.md`

## 8. 结论

当前工程已经完成从 AST 构造到语义分析、再到中间代码和基本块输出的实验要求，并具备：

- 可视化 AST 输出
- 系统化语义错误检查
- 可比较的 IR golden 结果
- 可直接演示的端到端前端程序
