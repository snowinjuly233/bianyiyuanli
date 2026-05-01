# Semantic Test Results

| Case | Exit | Expected Exit | Expected Text | Result |
| --- | ---: | ---: | --- | --- |
| valid_minimal | 0 | 0 | `ACCEPT` | PASS |
| valid_scope_shadowing | 0 | 0 | `ACCEPT` | PASS |
| error_var_redefinition | 1 | 1 | `redefinition of variable` | PASS |
| error_param_redefinition | 1 | 1 | `redefinition of parameter` | PASS |
| error_function_redefinition | 1 | 1 | `redefinition of function` | PASS |
| error_undeclared_identifier | 1 | 1 | `assignment to undeclared identifier` | PASS |
| error_call_undeclared_function | 1 | 1 | `call to undeclared function` | PASS |
| error_call_non_function | 1 | 1 | `call of non-function` | PASS |
| error_arg_count | 1 | 1 | `function call argument count mismatch` | PASS |
| error_arg_type | 1 | 1 | `function argument type mismatch` | PASS |
| error_assign_type | 1 | 1 | `assignment type mismatch` | PASS |
| error_init_type | 1 | 1 | `initialization type mismatch for variable` | PASS |
| error_condition_type | 1 | 1 | `if condition must be scalar type` | PASS |
| error_while_condition_type | 1 | 1 | `while condition must be scalar type` | PASS |
| error_return_void_value | 1 | 1 | `void function must not return a value` | PASS |
| error_return_missing_value | 1 | 1 | `non-void function must return a value` | PASS |
| error_return_type_mismatch | 1 | 1 | `return type mismatch` | PASS |
