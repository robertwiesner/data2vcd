
#include <stdio.h>

#include "cmodule.h"
#include "cwire.h"
#include "cvcdoutput.h"
#include "cjson.h"
#include "cbitfield.h"

int
main(int argc, char **ppArgv)
{
    char aJsonObj[] = "[1, { \"s\" : 0x123456 }, \"asd\"]";
    char aBuffer[64*1024];
    int len = sizeof(aBuffer);

    char *pJsonObj = aJsonObj;
    printf("%p\n", pJsonObj);
    cJSONbase *pJson = cJSONbase::generate(0, pJsonObj);
    printf("%p\n", pJson);

    if (argc != 3) {
        printf("%s <json> <outputfile>", ppArgv[0]);
    }

    FILE *pOut = fopen(ppArgv[2], "w");
    cVCDOutput output(pOut);

    FILE *pIn = fopen(ppArgv[1], "r");
    cJSONbase *pJsonEla = cJSONbase::generate(0, pIn);

    char *pEnd = pJson->toStr(len, aBuffer);
    len = sizeof(aBuffer);
    pEnd = pJsonEla->toStr(len, aBuffer);
    printf("ELA: %d:%s\n", len, aBuffer);

    cBitfield *pELA = new cBitfield(pJsonEla, output);
    pELA->printHeader("");
    
    unsigned int clk = 0;
    for (int tick = 0; tick < 10; tick += 1) {
        char aBuffer[16] = { (char) tick, (char) tick, (char) tick, (char) tick, (char) tick, (char) tick};
        pELA->setTime(2*tick);
        pELA->updateValue(0ull, 1, (char *) &clk);
        pELA->updateValue(0x1000, 16, aBuffer);

        clk ^= 1;
        pELA->setTime(2*tick + 1);
        pELA->updateValue(0ull, 1, (char *) &clk);
        clk ^= 1;
        
    }

    pELA->flush();
    pELA->finish();
    fclose(pOut);
}
