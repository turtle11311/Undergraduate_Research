%{
#include <cstdio>
#include <cstring>
extern int yylex(void);
extern int yylineno;
extern char* yytext;
int yyerror(const char* text) {
    fprintf(stderr, "%s is %s in %d\n", text, yytext, yylineno);
    return 1;
}
%}

%union { char* tok_str; int num; }
%token NL
%token <num> _tok_NUMBER
%token <tok_str> _tok_IDENTIFY

%type <tok_str> threshold
%%
statements              : statements statement
                        | statement
                        ;

statement               : threshold thresholds NL
                        ;

thresholds              : thresholds threshold
                        | threshold
                        ;

threshold               : _tok_IDENTIFY '=' _tok_NUMBER
                        ;
%%
