# SeuLex 词法实验报告

## 1. 实验范围

本阶段提交的是 `MiniC` 子集上的 `SeuLex` 实现。最终提交的词法规格文件为 `specs/minic.l`，它只使用当前实现已经稳定支持的 `Lex` 子集，因此“提交规格”和“实现能力”是一致的。

当前支持的输入规格能力如下：

- 三段式 `.l` 文件：定义区、规则区、用户代码区。
- 规则动作：`return TokenKind;`、`skip;`、短动作 `;`、续行动作 `|`。
- 正则定义展开：`{NAME}`。
- 扩展正规式：分组 `(...)`、选择 `|`、连接、`*`、`+`、`?`、`{m}`、`{m,n}`、`{m,}`。
- 字符集合：`[...]`、`[^...]`、范围如 `[A-Z]`。
- 字面量与转义：`"..."`、`\n`、`\r`、`\t`。
- 通配符：`.`。

当前未实现的标准 `Lex` 特性如下，因此它们没有出现在最终提交的 `specs/minic.l` 中：

- 开始条件 `<STATE>`。
- 尾随上下文 `r/s`。
- 行首或行尾锚定 `^`、`$`。
- 任意 C 动作代码、`yytext`、`yyleng`、`ECHO`、`yywrap` 等运行时接口。

## 2. 最终提交的 `.l` 文件

最终提交规格文件：`specs/minic.l`

该文件覆盖了本阶段 `MiniC` 子集需要的全部 token：

- 关键字：`int`、`float`、`void`、`if`、`else`、`while`、`return`
- 标识符：`Identifier`
- 常量：`IntegerLiteral`、`FloatLiteral`
- 界符：`(`、`)`、`{`、`}`、`,`、`;`
- 运算符：`+`、`-`、`*`、`/`、`%`、`=`、`==`、`!=`、`<`、`<=`、`>`、`>=`、`&&`、`||`、`!`

规格文件中的规则全部满足当前实现约束：

- 每条规则占一行，便于 `LexSpecParser` 解析。
- 动作统一为 `return Xxx;` 或 `skip;`。
- 浮点数规则写为 `{DIGIT}+"."{DIGIT}+`，与当前正则能力严格匹配。
- 空白符统一由 `{WS}` 规则跳过。

## 3. 关键数据结构

### 3.1 词法规格

- `LexDefinition`
  - 字段：`name`、`pattern`
  - 作用：保存定义区中的命名正则，例如 `DIGIT`、`ID`
- `LexRule`
  - 字段：`regex_text`、`normalized_regex`、`action_text`、`priority`、`action_kind`、`token_kind`
  - 作用：保存单条规则，并显式记录“最长匹配后按规则优先级决议”的顺序
- `LexSpec`
  - 字段：`prologue`、`epilogue`、`definitions`、`rules`
  - 作用：作为整个 `.l` 文件的内存表示

### 3.2 正则抽象语法树

- `RegexKind`
  - 取值：`Empty`、`Literal`、`Concat`、`Alternate`、`Star`、`Plus`、`Optional`、`CharacterClass`
- `RegexNode`
  - 字段：`kind`、`text`、`children`
  - 作用：把扩展正规式转换成统一的树形结构，供 NFA 构造使用

### 3.3 自动机

- `NFAState`
  - 字段：`id`、`accepting`、`rule_priority`
- `NFAEdge`
  - 字段：`from`、`to`、`symbol`、`epsilon`
- `NFA`
  - 字段：`start_state`、`accept_state`、`states`、`edges`
- `DFAState`
  - 字段：`id`、`accepting`、`rule_priority`、`nfa_states`
- `DFATransition`
  - 字段：`from`、`to`、`symbol`
- `DFA`
  - 字段：`start_state`、`states`、`transitions`

### 3.4 扫描运行时

- `ScannerAcceptAction`
  - 字段：`kind`、`rule_priority`、`skip`
  - 作用：把接受态映射回词法动作
- `ScannerState`
  - 字段：`id`、`accept`、`transitions`
  - 作用：作为运行时表驱动扫描器的状态结构
- `Token`
  - 字段：`kind`、`lexeme`、`span`
  - 作用：作为词法分析与后续语法分析之间的统一接口

## 4. 算法流程

整个 `SeuLex` 的处理流程如下：

1. 读取 `.l` 文件并切分三段结构。
2. 解析定义区，建立 `{NAME -> pattern}` 映射。
3. 解析规则区，提取 `regex + action`，并记录规则优先级。
4. 展开命名定义，把扩展正规式归一化。
5. 解析归一化后的正规式，构建 `RegexNode` 抽象语法树。
6. 对每条规则分别使用 Thompson 算法构造 NFA。
7. 把所有规则 NFA 合并成一个总 NFA。
8. 对总 NFA 做子集构造，得到 DFA。
9. 以“接受态属性 + 转移签名”为划分依据，对 DFA 做最小化。
10. 导出两类产物：
   - `dfa.dot`：用于可视化状态图
   - `*_scanner.h/.cpp/.main.cpp`：用于独立编译运行的词法分析程序

## 5. 最小化前后状态数

当前已实测两套规格：

| 规格 | 用途 | 最小化前 | 最小化后 |
| --- | --- | ---: | ---: |
| `specs/minic.l` | 最终提交规格 | 59 | 57 |
| `specs/lex_features.l` | 实现子集回归规格 | 27 | 25 |

状态数变化说明：

- `minic.l` 主要覆盖课程实验最终要提交的 `MiniC` 子集 token。
- `lex_features.l` 不作为最终提交规格，而是专门验证扩展正则子集，例如 `.`、`[^...]`、`{m,n}`、短动作 `;`、续行动作 `|`。

## 6. 代码生成结果

对任意输入规格，`ScannerEmitter` 会生成以下文件：

- `*_scanner.h`
  - 定义自包含的 `TokenKind`、`Token`、位置信息结构和扫描器类接口
- `*_scanner.cpp`
  - 固化最小 DFA 状态表、接受态动作表以及扫描循环
- `*_scanner_main.cpp`
  - 提供独立命令行入口，直接输出 token 序列
- `CMakeLists.txt`
  - 允许生成结果单独配置、单独编译
- `dfa.dot`
  - 导出 DFA 状态图，便于展示和调试

这意味着实验提交的词法生成器不仅能“运行时扫描输入”，还能够“生成一个独立可编译的词法分析程序”，满足实验对代码生成结果的要求。

## 7. 系统测试设计

本项目为 `Lex` 部分补充了系统化测试，分为两组：

### 7.1 最终提交规格测试

- `keywords_and_identifiers`
  - 验证关键字优先级高于标识符，但 `ifelse`、`returnValue` 仍应识别为标识符
- `operators_and_delimiters`
  - 验证 `==`、`!=`、`<=`、`>=`、`&&`、`||` 的最长匹配，以及括号、花括号、逗号、分号覆盖
- `literals_and_layout`
  - 验证整数字面量、浮点字面量，以及跨行行列号定位
- `bom_input`
  - 验证 UTF-8 BOM 文件兼容性
- `invalid_char`
  - 验证非法字符会触发错误输出，并返回非零退出码

### 7.2 实现子集回归测试

- `feature_mix`
  - 验证 `lex_features.l` 中的注释跳过、重复次数、取反字符类和通配符能力

## 8. 测试结果

测试脚本：`tests/lex/run_lex_tests.ps1`

脚本会自动完成以下操作：

1. 重新编译主工程中的 `seulex.exe`
2. 重新生成 `minic` 和 `lex_features` 两套独立 scanner
3. 单独编译这两套生成结果
4. 执行所有 golden case
5. 比较实际输出和期望输出
6. 生成测试结果文档 `tests/lex/results/lex_test_results.md`

当前已经实测通过：

- 共 6 个 case，全部通过
- 汇总结果见 `tests/lex/results/lex_test_results.md`
- 详细原始输出保存在 `tests/lex/results/*.actual.txt`

## 9. 结论

当前 `SeuLex` 已经满足 `MiniC` 子集上的课程实验要求：

- 有最终提交规格文件
- 有完整的 `.l -> NFA -> DFA -> 最小化 DFA -> 独立 scanner` 链路
- 有可视化状态图
- 有独立可编译的生成结果
- 有系统化测试与测试报告

仍然保留的边界如下：

- 本实现面向实验选定的 `MiniC` 子集，而不是完整 `flex` 兼容实现
- 高级 `Lex` 特性如开始条件、尾随上下文和复杂动作代码尚未覆盖
- 后续如果要承接更完整的 `c99.l`，仍需继续扩展规格解析和动作系统
