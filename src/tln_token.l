%{
#include <cstdio>
#include "tln_parser.hpp"
%}

%option yylineno
%s COMMENT

%%
<COMMENT>{
    .                   ;
    \n                  BEGIN(INITIAL);

}
<INITIAL>{
    "#"                 BEGIN(COMMENT);
    \n                  return NL;
    [[:alnum:]_[]-]+    { yylval.tok_str = strdup(yytext); return _tok_IDENTIFY; }
    [+-]?[[:digit:]]+   { sscanf(yytext, "%d", &yylval.num); return _tok_NUMBER; }
    [[:blank:]]+        ;
    .                   return *yytext;
}
%%
