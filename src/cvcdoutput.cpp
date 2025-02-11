#include "cvcdoutput.h"
#include "cmodule.h"
#include "cwire.h"
#include <stdio.h>

void cVCDOutput::headerStart()
{
    fprintf(pOut, "$date\n");
    fprintf(pOut, "    Date Text: \n");
    fprintf(pOut, "$end\n");

    fprintf(pOut, "$version\n");
    fprintf(pOut, "    VCD generator V0.0.1\n");
    fprintf(pOut, "$end\n");

    fprintf(pOut, "$comment\n");
    fprintf(pOut, "    Revision information\n");
    fprintf(pOut, "$version\n");


    fprintf(pOut, "$timescale 1ps $end\n");
}
void cVCDOutput::headerSetStartTime(long long time)
{
    fprintf(pOut, "Start time: %lld", time);
}

void cVCDOutput::headerModuleStart(cModule *pM, std::string prefix)
{
    pLastModule = pM;
    items.push_back(new sData(pM, NULL));
    fprintf(pOut, "$scope module %s $end\n", pM->getName());
}

void cVCDOutput::headerModuleEnd(cModule *pM)
{
    pLastModule = pM;
    items.push_back(new sData(pM, NULL));
    fprintf(pOut, "$upscope $end\n");
}

void cVCDOutput::headerWire(cWire *pW, std::string prefix)
{
    char aBuffer[256];
    pW->setIndex(items.size());
    items.push_back(new sData(pLastModule, pW));

    if (wireCountMax == 0) {
        wireCountMax = pLastModule->getWireCount();
    }

    pW->setShortName(shortWireGenerator(aBuffer + sizeof(aBuffer)));
    fprintf(pOut, "$var wire %ld %s %s $end\n", pW->getBits(), pW->getShortName(), pW->getName());
}

void cVCDOutput::headerEnd()
{
    // over all wires generate the short names
    size_t wireCount = pLastModule->getWireCount();
    fprintf(pOut, "$enddefinitions $end\n");
    fprintf(pOut, "$dumpvars\n");

    for (std::vector<sData *>::iterator it = items.begin(); it != items.end(); it++) {
        cWire *pW = (*it)->pWire;
        if (pW != NULL) {
            char *pPtr = aBuffer;
            const char *pSN = pW->getShortName();
            size_t bitLen = pW->getBits();
            bool addSpace = 1 < strlen(pSN);

            switch(pW->getWireType()) {

            case WT_BOOL:
                *pPtr++ = 'x';
                break;

            case WT_BITS:
            case WT_HEX:
                if (bitLen == 1) {
                    *pPtr++ = 'x';
                    break;
                }
                addSpace = true;
                *pPtr++ = 'b';
                for (size_t idx = bitLen; 0 < idx--; ) {
                    *pPtr++ = 'x';
                }
                break;
            }

            if (addSpace) {
                *pPtr++ = ' ';
            }

            while (*pSN) {
                *pPtr++ = *pSN++;
            }
            *pPtr = 0;
            fprintf(pOut, "%s\n", aBuffer);
        }
    }
}
    
void cVCDOutput::setTime(long long time)
{
    flush();
    fprintf(pOut, "#%lld\n", time);
}

const char *
cVCDOutput::getStringValue(cWire *pW)
{
    char *pPtr = aBuffer;
    const char *pSN = pW->getShortName();
    const unsigned char *pVal = pW->getBuffer();
    size_t bitLen = pW->getBits();
    bool addSpace = 1 < strlen(pSN);

    switch(pW->getWireType()) {
    case WT_NONE:
        break;

    case WT_BOOL:
        *pPtr++ = *pVal ? '1' : '0';
        break;

    case WT_BITS:
    case WT_HEX:
        if (bitLen == 1) {
            *pPtr++ = *pVal & 1 ? '1' : '0';
            break;
        }
        addSpace = true;
        *pPtr++ = 'b';
        for (size_t idx = bitLen; 0 < idx--; ) {
            *pPtr++ = pVal[idx / 8] & (1 << (idx & 7)) ? '1' : '0';
        }
        break;

    case WT_STR:
        memcpy(pPtr, pVal, (bitLen + 7) / 8);
        pPtr += (bitLen + 7) / 8;
        break;
    }

    if (addSpace) {
        *pPtr++ = ' ';
    }

    while (*pSN) {
        *pPtr++ = *pSN++;
    }

    *pPtr = 0;
    return aBuffer;
}

void cVCDOutput::print(cWire *pW)
{
    fprintf(pOut, "%s\n", getStringValue(pW));
}

void cVCDOutput::finish()
{
    fprintf(pOut, "DONE!!!\n");
}

cVCDOutput::~cVCDOutput()
{

}