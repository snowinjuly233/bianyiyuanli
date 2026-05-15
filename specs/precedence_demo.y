%{
/* Ambiguous expression grammar used to validate precedence and associativity. */
%}

%token Identifier IntegerLiteral
%token LParen RParen
%token Plus Star Assign Less

%right Assign
%left Plus
%left Star
%nonassoc Less

%start program

%%
program
    : expression
    ;

expression
    : Identifier Assign expression
    | expression Plus expression
    | expression Star expression
    | expression Less expression
    | LParen expression RParen
    | Identifier
    | IntegerLiteral
    ;
%%
