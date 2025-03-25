
#include "cjson.h"

#include <stdio.h>
const char *pTest0_0 = "{}";
const char *pTest0_1 = "{ }";
const char *pTest1_0 = "{ 'Hello' : 'world' }";
const char *pTest1_1 = "{ 'Hello' : 'world', }";
const char *pTest2_0 = "{ 'Hello' : 'world', 'Hallo' : 'Welt'} ";
const char *pTest2_1 = "{ 'Hello' : 'world', , 'Hallo' : 'Welt' ,}";
const char *pTest3_0 = "[1, nil, 2, nil, 3 ]";
const char *pTest3_1 = "[1, , 2, , 3 ]";
const char *pTest3_2 = "[1, , 2, , 3, ]";

const char *pTest4_0 = "[]";
const char *pTest4_1 = "[ ]";

char aaBuffer[3][1024];

void
strip(char *pS) {
    char *pD = pS;
    while (*pS) {
        if (*pS != ' ' && *pS != '\n') {
            *pD++ = *pS;
        }
        pS++;
    }
    *pD = 0;
}
void
test(cJSONbase *pJ0, cJSONbase *pJ1)
{

    int t0, t1;

    t0 = t1 = sizeof(aaBuffer[0]);

    pJ0->toStr(t0, aaBuffer[0]);
    pJ1->toStr(t1, aaBuffer[1]);

    if (strcmp(aaBuffer[0], aaBuffer[1])) {
        strip(aaBuffer[0]);
        strip(aaBuffer[1]);
        printf("ERROR: \n%s\n%s\n", aaBuffer[0], aaBuffer[1]);
    } else {
        strip(aaBuffer[0]);
        printf("PASS: %s\n", aaBuffer[0]);
    }
}

int main()
{
    cJSONbase *pJ0_0 = cJSONbase::genStr(0, pTest0_0);
    cJSONbase *pJ0_1 = cJSONbase::genStr(0, pTest0_1);
    cJSONbase *pJ1_0 = cJSONbase::genStr(0, pTest1_0);
    cJSONbase *pJ1_1 = cJSONbase::genStr(0, pTest1_1);
    cJSONbase *pJ2_0 = cJSONbase::genStr(0, pTest2_0);
    cJSONbase *pJ2_1 = cJSONbase::genStr(0, pTest2_1);
    cJSONbase *pJ3_0 = cJSONbase::genStr(0, pTest3_0);
    cJSONbase *pJ3_1 = cJSONbase::genStr(0, pTest3_1);
    cJSONbase *pJ3_2 = cJSONbase::genStr(0, pTest3_1);
    cJSONbase *pJ4_0 = cJSONbase::genStr(0, pTest4_0);
    cJSONbase *pJ4_1 = cJSONbase::genStr(0, pTest4_1);

    test(pJ0_0, pJ0_1);
    test(pJ1_0, pJ1_1);
    test(pJ2_0, pJ2_1);
    test(pJ3_0, pJ3_1);
    test(pJ3_0, pJ3_2);
    test(pJ4_0, pJ4_1);

}
