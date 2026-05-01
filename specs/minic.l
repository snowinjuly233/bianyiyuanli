%{
/* Frozen MiniC lexer subset shared by parsing, semantic analysis, and IR. */
%}

DIGIT      [0-9]
LETTER     [A-Za-z_]
ID         {LETTER}({LETTER}|{DIGIT})*
WS         [ \t\r\n]+
LINE_COMMENT "//"[^\n]*

%%
{WS}               { skip; }
{LINE_COMMENT}     { skip; }
"int"              { return KwInt; }
"float"            { return KwFloat; }
"void"             { return KwVoid; }
"if"               { return KwIf; }
"else"             { return KwElse; }
"while"            { return KwWhile; }
"return"           { return KwReturn; }
"("                { return LParen; }
")"                { return RParen; }
"{"                { return LBrace; }
"}"                { return RBrace; }
","                { return Comma; }
";"                { return Semicolon; }
"=="               { return Equal; }
"!="               { return NotEqual; }
"<="               { return LessEqual; }
">="               { return GreaterEqual; }
"&&"               { return LogicalAnd; }
"||"               { return LogicalOr; }
"="                { return Assign; }
"<"                { return Less; }
">"                { return Greater; }
"!"                { return LogicalNot; }
"+"                { return Plus; }
"-"                { return Minus; }
"*"                { return Star; }
"/"                { return Slash; }
"%"                { return Percent; }
{DIGIT}+"."{DIGIT}+ { return FloatLiteral; }
{DIGIT}+           { return IntegerLiteral; }
{ID}               { return Identifier; }
%%
