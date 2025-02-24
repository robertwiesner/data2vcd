#include <stdio.h>

#include "cmodule.h"
#include "cwire.h"
#include "cvcdoutput.h"
#include "cjson.h"
#include "cbitfield.h"

class cDataBase {
    cBitfield *pELA;
    unsigned long long modIdx;
    public:

    cDataBase() { pELA = 0; modIdx = 0; }

    cDataBase(cBitfield *pE, unsigned long long mi) {
        pELA = pE;
        modIdx = mi;
    }

    ~cDataBase() {}

    void update(cBitfield *pE, unsigned long long mi) {
        pELA   = pE;
        modIdx = mi;
    }
    void setTime(unsigned long long t) { pELA->setTime(t); }
    void updateData(int bitSize, const char *pData) {
        pELA->updateValue(modIdx, bitSize, pData);
    }
    void updateData(unsigned long long val) {
        pELA->updateValue(modIdx, 8*sizeof(val), (const char *) &val);
    }
};

class cELAtrace {
    cDataBase aTrace[8];
    public:
    cELAtrace(cBitfield *pE, unsigned long long base) {
        for (int idx = 0; idx < 8; idx++) {
            aTrace[idx].update(pE, base + idx);
        }
    }

    void setTime(unsigned long long t) { aTrace[0].setTime(t); }
    void outputTrace(int channel, int size, const char *pData) {
        if (0 <= channel && channel < 8) {
            aTrace[channel].updateData(size, pData);
        }
    }
    void outputTrace(int channel, int size, const unsigned char *pData) {
        outputTrace(channel, size, (const char *) pData);
    }
};
int
main(int argc, char **ppArgv)
{
    char *pEnd;
    char aBuffer[64*1024];
    int len = sizeof(aBuffer);

    if (argc != 3) {
        printf("%s <json> <outputfile>", ppArgv[0]);
    }

    FILE *pIn = fopen(ppArgv[1], "r");
    cJSONbase *pJsonEla = cJSONbase::generate(0, pIn);
    FILE *pOut = fopen(ppArgv[2], "w");
    cVCDOutput output(pOut);

    len = sizeof(aBuffer);
    pEnd = pJsonEla->toStr(len, aBuffer);
    printf("ELA: %d:%s\n", (int) (sizeof(aBuffer) - len), aBuffer);

    cBitfield *pELA = new cBitfield(pJsonEla, output);
    cELAtrace ELAtrace(pELA, 0x1000);
    pELA->printHeader("");

    unsigned char aELA1[] = {0x6c, 0x00, 0x55, 0x00, 0x77, 0x99, 0xf1, 0x12};
    unsigned char aELA2[] = {0x70, 0x01, 0x54, 0x00, 0x66, 0x99, 0xf1, 0x12};

    ELAtrace.setTime(0x1000);
    ELAtrace.outputTrace(0, 8, aELA1);
    ELAtrace.outputTrace(1, 8, aELA2);

    ELAtrace.setTime(0x1030);

    ELAtrace.outputTrace(0, 8, aELA2);
    ELAtrace.outputTrace(1, 8, aELA1);


    ELAtrace.setTime(0x1040);

    ELAtrace.outputTrace(3, 8, aELA1);
    pELA->flush();
    pELA->finish();
    fclose(pOut);
}
