#include <cstdio>
extern int yyparse(void);

int main(int argc, char const *argv[]) {
    char tmp[300];
    for (int i = 0; i < 6; ++i) {
        fgets(tmp, 300, stdin);
    }
    yyparse();
    return 0;
}
