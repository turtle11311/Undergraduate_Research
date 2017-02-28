#include <cstdio>
#include <iostream>
#include "ThresholdNetwork.hpp"
#include "ThresholdNetworkDebugger.hpp"
extern int yyparse(void);
extern int yylineno;
ThresholdNetwork network;

int main(int argc, char const *argv[]) {
    char tmp[300];
    for (int i = 0; i < 6; ++i) {
        fgets(tmp, 300, stdin);
        ++yylineno;
    }
    yyparse();
    std::ios_base::sync_with_stdio(false);      // use for speedup I/O
    network.gateClassify();
    network.foreachGateAttr();
    network.evalMandatoryAssignments();
#ifdef DEBUG
    // ThresholdNetworkDebugger debugger(network);
    // debugger.listen();
#endif
    return 0;
}
