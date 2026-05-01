# AST Test Results

| Case | Check | Result |
| --- | --- | --- |
| minic_demo | frontend accept | PASS |
| minic_demo | ast root | PASS |
| minic_demo | text fragment Program | PASS |
| minic_demo | text fragment FunctionDecl name="main" return=int | PASS |
| minic_demo | text fragment AssignExpr target="x" | PASS |
| minic_demo | text fragment IfStmt | PASS |
| minic_demo | text fragment ReturnStmt | PASS |
| minic_demo | markdown fragment # AST | PASS |
| minic_demo | markdown fragment - `Program` | PASS |
| minic_demo | markdown fragment `FunctionDecl name="main" return=int` | PASS |
| minic_demo | markdown fragment `IfStmt` | PASS |
| feature_coverage | frontend accept | PASS |
| feature_coverage | ast root | PASS |
| feature_coverage | text fragment VarDecl name="seed" type=int | PASS |
| feature_coverage | text fragment FunctionDecl name="twice" return=int | PASS |
| feature_coverage | text fragment CallExpr callee="twice" | PASS |
| feature_coverage | text fragment Else | PASS |
| feature_coverage | text fragment WhileStmt | PASS |
| feature_coverage | markdown fragment # AST | PASS |
| feature_coverage | markdown fragment `VarDecl name="seed" type=int` | PASS |
| feature_coverage | markdown fragment `CallExpr callee="twice"` | PASS |
| feature_coverage | markdown fragment `WhileStmt` | PASS |
