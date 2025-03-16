/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.

(c) 2025 Robert Wiesner
*/

#include "coutput.h"
#include "cmodule.h"
#include "cwire.h"
#include <stdio.h>

void cOutput::headerStart()
{
    fprintf(pOut, "Start Header\n");
}

void cOutput::headerSetStartTime(int64_t time)
{
    fprintf(pOut, "Start time: %ld", time);
}

void cOutput::headerModuleStart(cModule *pM, std::string prefix)
{
    pLastModule = pM;
    items.push_back(new sData(pM, nullptr));
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
    size_t wireCount = pLastModule->getWireCount();
    fprintf(pOut, "Total WireCount: %d\n", static_cast<int>(wireCount));
    fprintf(pOut, "End Header\n");
}
    
void cOutput::setTime(int64_t time)
{
    flush();
    fprintf(pOut, "Start time: %ld\n", time);
}

const char *cOutput::getStringValue(cWire *pW)
{
    char *pPtr = aBuffer;
    char *pEnd = aBuffer + sizeof(aBuffer);
    const unsigned char* pVal = pW->getBuffer();
    size_t bitLen = pW->getBits();

    switch (pW->getWireType()) {
    case eWT_NONE:
        break;

    case eWT_BOOL:
        strcpy(pPtr, *pVal ? "TRUE" : "false");
        pPtr += strlen(pPtr);
        break;

    case eWT_BIT:
        for (size_t idx = bitLen; 0 < idx--; ) {
            *pPtr++ = pVal[idx / 8] & (1 << (idx & 7)) ? '1' : '0';
        }
        break;

    case eWT_OCT:
        snprintf(pPtr, pEnd - pPtr, "%lo", pW->getAsUnsignedLongLong());
        pPtr += strlen(pPtr);
        break;

    case eWT_DEC:
        snprintf(pPtr, pEnd - pPtr, "%ld", pW->getAsSignedLongLong());
        pPtr += strlen(pPtr);
        break;

    case eWT_HEX:
    case eWT_HEXs:
        for (size_t idx = (bitLen + 3) & (~3); 0 < idx; ) {
            idx -= 4;
            int val = pVal[idx / 8];
            val = idx & 4 ? val >> 4 : val & 0xf;
            *pPtr++ = "0123456789ABCDEF"[val];
        }
        break;

    case eWT_STR:
        memcpy(pPtr, pVal, (bitLen + 7) / 8);
        pPtr += (bitLen + 7) / 8;
        break;

    }
    *pPtr = 0;
    return aBuffer;
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
    for (size_t idx = 0; idx < items.size(); idx++) {
        cWire *pW = items[idx]->pWire;
        print(pW);
    }
}
