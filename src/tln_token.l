%{
#include <cstdio>
extern "C" int yylex();
char tmp[300];
%}

%option yylineno
%s COMMENT

%%
<COMMENT>{
    .               ;
    \n              BEGIN(INITIAL);
}

<INITIAL>{
    "#"             BEGIN(COMMENT);
    .               ECHO;
}
%%
int main(int argc,const char* argv[]) {
    for (int i = 0; i < 7; ++i) {
        fgets(tmp, 299, stdin);
    }
    yylex();
}
