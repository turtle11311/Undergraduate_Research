%{
#include <cstdio>
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include "ThresholdNetwork.hpp"
extern ThresholdNetwork network;

std::vector<GateAttr> inputGates;
extern int yylex(void);
extern int yylineno;
extern char* yytext;
int yyerror(const char* text) {
    fprintf(stderr, "%s is %s in %d\n", text, yytext, yylineno);
    return 1;
}
%}

%union {
    char* tok_str;
    int num;
    GateAttr gate_attr;
}

%token NL
%token <num> _tok_NUMBER
%token <tok_str> _tok_IDENTIFY

%type <gate_attr> threshold
%%
statements              : statements statement
                        | statement
                        ;

statement               : threshold thresholds NL
                        {
                            Gate* curGate = network.accessGateByName($1.name);
                            curGate->thresholdVal = $1.thresholdVal;
                            // sort fan_in sequence by threshold value from large to small
                            for (GateAttr &attr : inputGates) {
                                Gate* inputGate = network.accessGateByName(attr.name);
                                if (attr.thresholdVal > 0) {
                                    curGate->addInput(inputGate, attr.thresholdVal, false);
                                } else {
                                    curGate->thresholdVal -= attr.thresholdVal;
                                    curGate->addInput(inputGate, -attr.thresholdVal, true);
                                }
                            }
                            std::sort(curGate->fan_in.begin(), curGate->fan_in.end(),
                                [](const ThresholdInput &a, const ThresholdInput &b) {
                                    return std::get<1>(a) > std::get<1>(b);
                            });
                            inputGates.clear();
                        }
                        ;

thresholds              : thresholds threshold
                        {
                            inputGates.push_back($2);
                        }
                        | threshold
                        {
                            inputGates.push_back($1);
                        }
                        ;

threshold               : _tok_IDENTIFY '=' _tok_NUMBER
                        {
                            $$.name = $1;
                            $$.thresholdVal = $3;
                        }
                        ;
%%
