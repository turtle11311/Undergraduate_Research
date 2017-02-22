#include <cstdio>
#include "ThresholdNetwork.hpp"
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
    network.gateClassify();
    network.foreachGateAttr();
    return 0;
}
