
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
    cJSONbase *pJson = cJSONbase::generate(0, pJsonObj);
    FILE *pIn = fopen("ela.json", "r");

    cJSONbase *pJsonEla = cJSONbase::generate(0, pIn);

    char *pEnd = pJson->toStr(len, aBuffer);
    printf("In: %s -> Out: %d:%s\n", aJsonObj, len, aBuffer);

    len = sizeof(aBuffer);
    pEnd = pJsonEla->toStr(len, aBuffer);
    printf("ELA: %d:%s\n", len, aBuffer);

    cBitfield *pELA = new cBitfield(pJsonEla);

#if 0 
    cJSONscalar *pS1 = dynamic_cast<cJSONscalar *>(pJson);
    cJSONscalar *pS2 = (cJSONscalar *) pJson ;
    printf("%p %p\n", pS1, pS2);
    cModule *pOne   = new cModule("ONE");
    cModule *pTwo   = new cModule("TWO", NULL, pOne);
    cModule *pThree = new cModule("THREE", NULL, pOne);
    cWire *pW1, *pW2, *pW3;
    pOne->addWire(pW1 = new cWire(WT_BITS, 1, "one"));
    pOne->addWire(pW2 = new cWire(WT_BITS, 2, "two"));
    pOne->addWire(pW3 = new cWire(WT_BITS, 3, "three"));

    cModule *pOneOne = new cModule("ONEone", pOne);
    new cModule("ONEtwo", pOne, pOneOne);
    pOneOne->addWire(new cWire(WT_HEX, 64, "addr"));
    pOneOne->addWire(new cWire(WT_HEX, 64, "data"));
#endif

    FILE *pOut = fopen("output.vcd", "w");
    cVCDOutput output(pOut);

    pELA->getFirstModule()->printHeader(&output, "");
    
    unsigned int clk = 0;
    for (int tick = 0; tick < 10; tick += 1) {
        char aBuffer[16] = { (char) tick, (char) tick, (char) tick, (char) tick, (char) tick, (char) tick};
        output.setTime(2*tick);
        pELA->updateValue(0, 1, (char *) &clk);
        pELA->updateValue(0x1000, 16, aBuffer);

        clk ^= 1;
        output.setTime(2*tick + 1);
        pELA->updateValue(0, 1, (char *) &clk);
        clk ^= 1;
        
    }

    output.flush();
    fclose(pOut);
}