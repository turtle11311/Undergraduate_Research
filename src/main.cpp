#include "ThresholdNetwork.hpp"
#include "ThresholdNetworkDebugger.hpp"
#include <cstdio>
#include <ctime>
#include <iostream>
extern int yyparse(void);
extern int yylineno;
ThresholdNetwork network;

int main(int argc, char const* argv[])
{
    char tmp[300];
    for (int i = 0; i < 6; ++i) {
        fgets(tmp, 300, stdin);
        ++yylineno;
    }
    clock_t start = clock();
    yyparse();
    clock_t yyparse_end = clock();
    std::ios_base::sync_with_stdio(false); // use for speedup I/O
    network.gateClassify();
    clock_t classify_end = clock();
    network.foreachGateAttr();
    clock_t foreach_end = clock();
    network.evalMandatoryAssignments();
    clock_t evalMA_end = clock();
#ifdef DEBUG
// ThresholdNetworkDebugger debugger(network);
// debugger.listen();
#endif
    return 0;
}
