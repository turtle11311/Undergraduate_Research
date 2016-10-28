%{
#include <cstdio>
#include <cstring>
extern int yylex(void);
extern int yylineno;
extern char* yytext;
int yyerror(const char* text) {
    fprintf(stderr, "%s in %s\n", text, yytext);
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
                        {
                            printf("%s\n", $1);
                        }
                        ;

thresholds              : thresholds threshold
                        | threshold
                        ;

threshold               : _tok_IDENTIFY '=' _tok_NUMBER
                        {
                            $$ = strdup($1);
                            printf("%s\n", $$);
                        }
                        ;
%%
