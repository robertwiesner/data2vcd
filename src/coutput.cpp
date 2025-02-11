
#include "coutput.h"
#include "cmodule.h"
#include "cwire.h"
#include <stdio.h>

void cOutput::headerStart()
{
    fprintf(pOut, "Start Header\n");
}

void cOutput::headerSetStartTime(long long time)
{
    fprintf(pOut, "Start time: %lld", time);
}

void cOutput::headerModuleStart(cModule *pM, std::string prefix)
{
    pLastModule = pM;
    items.push_back(new sData(pM, NULL));
    fprintf(pOut, "%*.*sModule: %s\n", 4*pM->getDepth(), 4*pM->getDepth(), "", pM->getName());
}

void cOutput::headerModuleEnd(cModule *pM)
{
}

void cOutput::headerWire(cWire *pW, std::string prefix)
{
    pW->setIndex(items.size());
    items.push_back(new sData(pLastModule, pW));

    fprintf(pOut, "%*.*s    Wire: %s\n", 4*pLastModule->getDepth(), 4*pLastModule->getDepth(), "", pW->getName());
}

void cOutput::headerEnd()
{
    // over all wires generate the short names

    char aBuffer[16];
    char *pPtr = aBuffer + sizeof(aBuffer);
    size_t wireCount = pLastModule->getWireCount();
    fprintf(pOut, "Total WireCount: %ld\n", wireCount);
    fprintf(pOut, "End Header\n");
}
    
void cOutput::setTime(long long time)
{
    flush();
    fprintf(pOut, "Start time: %lld\n", time);
}

const char *cOutput::getStringValue(cWire *pW)
{
    char *pPtr = aBuffer;
    const unsigned char *pVal = pW->getBuffer();
    size_t bitLen = pW->getBits();

    switch(pW->getWireType()) {
    case WT_NONE:
        break;
    case WT_BOOL:
        strcpy(pPtr, *pVal ? "TRUE" : "false");
        pPtr += strlen(pPtr);
        break;
    case WT_BITS:
        for (size_t idx = bitLen; 0 < idx--; ) {
            *pPtr++ = pVal[idx / 8] & (1 << (idx & 7)) ? '1' : '0';
        }
        break;
    case WT_HEX:
        for (size_t idx = (bitLen + 3) & (~3); 0 < idx; ) {
            idx -= 4;
            int val = pVal[idx / 8];
            val = idx & 4 ? val >> 4 : val & 0xf;
            *pPtr++ = "0123456789ABCDEF"[val];
        }
        break;
    case WT_STR:
        memcpy(pPtr, pVal, (bitLen + 7) / 8);
        pPtr += (bitLen + 7) / 8;
        break;
    }
    *pPtr = 0;
    return aBuffer;
}

void cOutput::print(cWire *pW)
{
    fprintf(pOut, "%s -> %s\n", pW->getName(), getStringValue(pW));
}

void cOutput::finish()
{
    fprintf(pOut, "DONE!!!\n");
}

cOutput::~cOutput()
{

}

void cOutput::flush()
{
    for (int idx = 0; idx < items.size(); idx++) {
        cWire *pW = items[idx]->pWire;
        if (pW && pW->isSet()) {
            print(pW);
        }
    }
}
