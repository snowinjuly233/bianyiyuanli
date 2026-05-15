%{
/* Feature-focused Lex subset validation spec. */
%}

HEX        [0-9A-Fa-f]
ID_HEAD    [A-Za-z_]
ID_TAIL    [A-Za-z0-9_]
ID         {ID_HEAD}{ID_TAIL}*

%%
" "        |
"\t"       |
"\n"       ;
"//".*     ;
0x{HEX}{2,4}           { return IntegerLiteral; }
\"[^"\n]{1,8}\"       { return Identifier; }
{ID}                   { return Identifier; }
"+"                    { return Plus; }
"-"                    { return Minus; }
%%
