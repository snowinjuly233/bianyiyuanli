%{
/* Frozen MiniC parser subset shared by parsing, semantic analysis, and IR.
 *
 * This file is the final submission grammar for the frozen MiniC subset.
 * Its semantic action blocks are the translation scheme executed by
 * SemanticActionExecutor to build AST nodes and semantic value lists.
 */
%}

%union {
    compilerlab::common::Token token;
    compilerlab::semantic::TypeSpecifier type_specifier;
    compilerlab::semantic::ProgramPtr program;
    compilerlab::semantic::DeclPtr decl;
    compilerlab::semantic::ParameterDeclPtr param;
    compilerlab::semantic::StmtPtr stmt;
    compilerlab::semantic::ExprPtr expr;
    compilerlab::semantic::DeclList decl_list;
    compilerlab::semantic::ParameterList param_list;
    compilerlab::semantic::StmtList stmt_list;
    compilerlab::semantic::ExprList expr_list;
}

%token <token> Identifier IntegerLiteral FloatLiteral
%token KwInt KwFloat KwVoid KwIf KwElse KwWhile KwReturn
%token LParen RParen LBrace RBrace Comma Semicolon
%token Plus Minus Star Slash Percent
%token Assign Equal NotEqual Less LessEqual Greater GreaterEqual
%token LogicalAnd LogicalOr LogicalNot

%type <program> program
%type <decl_list> external_declaration_list
%type <decl> external_declaration function_definition global_variable_declaration
%type <type_specifier> type_specifier
%type <param> parameter
%type <param_list> parameter_list_opt parameter_list
%type <stmt> statement compound_stmt declaration_stmt expression_stmt return_stmt if_stmt while_stmt
%type <stmt_list> statement_list_opt statement_list
%type <expr> initializer_opt expression assignment_expression logical_or_expression
%type <expr> logical_and_expression equality_expression relational_expression
%type <expr> additive_expression multiplicative_expression unary_expression primary_expression
%type <expr_list> argument_list_opt argument_list

%nonassoc IfThen
%nonassoc KwElse

%start program

%%
program
    : external_declaration_list
        { $$ = make_program($1); }
    ;

external_declaration_list
    : external_declaration_list external_declaration
        { $$ = append_decl($1, $2); }
    | external_declaration
        { $$ = make_decl_list($1); }
    ;

external_declaration
    : function_definition
        { $$ = $1; }
    | global_variable_declaration
        { $$ = $1; }
    ;

global_variable_declaration
    : type_specifier Identifier initializer_opt Semicolon
        { $$ = make_global_var_decl($1, $2, $3); }
    ;

initializer_opt
    : %empty
        { $$ = make_empty_initializer(); }
    | Assign expression
        { $$ = $2; }
    ;

function_definition
    : type_specifier Identifier LParen parameter_list_opt RParen compound_stmt
        { $$ = make_function_decl($1, $2, $4, $6); }
    ;

parameter_list_opt
    : %empty
        { $$ = make_param_list(); }
    | parameter_list
        { $$ = $1; }
    ;

parameter_list
    : parameter_list Comma parameter
        { $$ = append_param($1, $3); }
    | parameter
        { $$ = make_param_list($1); }
    ;

parameter
    : type_specifier Identifier
        { $$ = make_parameter_decl($1, $2); }
    ;

type_specifier
    : KwInt
        { $$ = compilerlab::semantic::TypeSpecifier::Int; }
    | KwFloat
        { $$ = compilerlab::semantic::TypeSpecifier::Float; }
    | KwVoid
        { $$ = compilerlab::semantic::TypeSpecifier::Void; }
    ;

compound_stmt
    : LBrace statement_list_opt RBrace
        { $$ = make_block_stmt($2); }
    ;

statement_list_opt
    : %empty
        { $$ = make_stmt_list(); }
    | statement_list
        { $$ = $1; }
    ;

statement_list
    : statement_list statement
        { $$ = append_stmt($1, $2); }
    | statement
        { $$ = make_stmt_list($1); }
    ;

statement
    : declaration_stmt
        { $$ = $1; }
    | expression_stmt
        { $$ = $1; }
    | return_stmt
        { $$ = $1; }
    | if_stmt
        { $$ = $1; }
    | while_stmt
        { $$ = $1; }
    | compound_stmt
        { $$ = $1; }
    ;

declaration_stmt
    : type_specifier Identifier initializer_opt Semicolon
        { $$ = make_local_decl_stmt($1, $2, $3); }
    ;

expression_stmt
    : expression Semicolon
        { $$ = make_expr_stmt($1); }
    | Semicolon
        { $$ = make_empty_expr_stmt(); }
    ;

return_stmt
    : KwReturn Semicolon
        { $$ = make_return_stmt(nullptr); }
    | KwReturn expression Semicolon
        { $$ = make_return_stmt($2); }
    ;

if_stmt
    : KwIf LParen expression RParen statement %prec IfThen
        { $$ = make_if_stmt($3, $5, nullptr); }
    | KwIf LParen expression RParen statement KwElse statement
        { $$ = make_if_stmt($3, $5, $7); }
    ;

while_stmt
    : KwWhile LParen expression RParen statement
        { $$ = make_while_stmt($3, $5); }
    ;

expression
    : assignment_expression
        { $$ = $1; }
    ;

assignment_expression
    : Identifier Assign assignment_expression
        { $$ = make_assign_expr($1, $3); }
    | logical_or_expression
        { $$ = $1; }
    ;

logical_or_expression
    : logical_or_expression LogicalOr logical_and_expression
        { $$ = make_binary_expr(LogicalOr, $1, $3); }
    | logical_and_expression
        { $$ = $1; }
    ;

logical_and_expression
    : logical_and_expression LogicalAnd equality_expression
        { $$ = make_binary_expr(LogicalAnd, $1, $3); }
    | equality_expression
        { $$ = $1; }
    ;

equality_expression
    : equality_expression Equal relational_expression
        { $$ = make_binary_expr(Equal, $1, $3); }
    | equality_expression NotEqual relational_expression
        { $$ = make_binary_expr(NotEqual, $1, $3); }
    | relational_expression
        { $$ = $1; }
    ;

relational_expression
    : relational_expression Less additive_expression
        { $$ = make_binary_expr(Less, $1, $3); }
    | relational_expression LessEqual additive_expression
        { $$ = make_binary_expr(LessEqual, $1, $3); }
    | relational_expression Greater additive_expression
        { $$ = make_binary_expr(Greater, $1, $3); }
    | relational_expression GreaterEqual additive_expression
        { $$ = make_binary_expr(GreaterEqual, $1, $3); }
    | additive_expression
        { $$ = $1; }
    ;

additive_expression
    : additive_expression Plus multiplicative_expression
        { $$ = make_binary_expr(Plus, $1, $3); }
    | additive_expression Minus multiplicative_expression
        { $$ = make_binary_expr(Minus, $1, $3); }
    | multiplicative_expression
        { $$ = $1; }
    ;

multiplicative_expression
    : multiplicative_expression Star unary_expression
        { $$ = make_binary_expr(Star, $1, $3); }
    | multiplicative_expression Slash unary_expression
        { $$ = make_binary_expr(Slash, $1, $3); }
    | multiplicative_expression Percent unary_expression
        { $$ = make_binary_expr(Percent, $1, $3); }
    | unary_expression
        { $$ = $1; }
    ;

unary_expression
    : LogicalNot unary_expression
        { $$ = make_unary_expr(LogicalNot, $2); }
    | Minus unary_expression
        { $$ = make_unary_expr(Minus, $2); }
    | primary_expression
        { $$ = $1; }
    ;

argument_list_opt
    : %empty
        { $$ = make_expr_list(); }
    | argument_list
        { $$ = $1; }
    ;

argument_list
    : argument_list Comma expression
        { $$ = append_expr($1, $3); }
    | expression
        { $$ = make_expr_list($1); }
    ;

primary_expression
    : Identifier
        { $$ = make_identifier_expr($1); }
    | IntegerLiteral
        { $$ = make_integer_literal_expr($1); }
    | FloatLiteral
        { $$ = make_float_literal_expr($1); }
    | LParen expression RParen
        { $$ = $2; }
    | Identifier LParen argument_list_opt RParen
        { $$ = make_call_expr($1, $3); }
    ;
%%
