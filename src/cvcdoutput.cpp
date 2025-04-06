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

#include <stdio.h>

#include "cvcdoutput.h"
#include "cmodule.h"
#include "cwire.h"

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
    fprintf(pOut, "$end\n");


    fprintf(pOut, "$timescale 1ps $end\n");
}

void cVCDOutput::headerSetStartTime(int64_t time)
{
    fprintf(pOut, "Start time: %" ELLELL "d", time);
}

void cVCDOutput::headerModuleStart(cModule *pM, std::string prefix)
{
    pLastModule = pM;
    items.push_back(new sData(pM, nullptr));
    fprintf(pOut, "$scope module %s $end\n", printableCharOnly(pM->getName()));
}

void cVCDOutput::headerModuleEnd(cModule *pM)
{
    pLastModule = pM;
    items.push_back(new sData(pM, nullptr));
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
    fprintf(pOut, "$var wire %d %s %s $end\n",
        static_cast<int32_t>(pW->getBits()),
        pW->getShortName(),
        printableCharOnly(pW->getName()));
}

void cVCDOutput::headerEnd()
{
    // over all wires generate the short names
    fprintf(pOut, "$enddefinitions $end\n");
    firstTimeOff = ftell(pOut);
    fprintf(pOut, "#0                    \n");
    fprintf(pOut, "$dumpvars\n");

    for (auto &it : items) {
        cWire *pW = it->pWire;
        print(pW);
    }
}

void cVCDOutput::setTime(int64_t time)
{
    flush();
    if (0 < firstTimeOff) {
        long currOff  = ftell(pOut);
        if (1 < time && 0 == fseek(pOut, static_cast<long>(firstTimeOff), SEEK_SET)) {
            fprintf(pOut, "#%" ELLELL "d", time - 1);
            fseek(pOut, currOff, SEEK_SET);
        }
        firstTimeOff = 0;
    }
    if (lastTime != time) {
        lastTime = time;
        fprintf(pOut, "#%" ELLELL "d\n", time);
    }
}

const char *
cVCDOutput::getStringValue(cWire *pW)
{
    char *pPtr = aBuffer;
    const char *pSN = pW->getShortName();
    const unsigned char *pVal = pW->getBuffer();
    size_t bitLen = pW->getBits();
    char one  = pW->isValid() ? '1' : 'x';
    char zero = pW->isValid() ? '0' : 'x';

    if (1 < bitLen) {
        *pPtr++ = 'b';
        for (size_t idx = bitLen; 0 < idx--; ) {
            *pPtr++ = pVal[idx / 8] & (1 << (idx & 7)) ? one : zero;
        }
        *pPtr++ = ' ';
    } else {
        *pPtr++ = pVal[0] & 1 ? one : zero;
    }

    while (*pSN) {
        *pPtr++ = *pSN++;
    }

    *pPtr = 0;
    return aBuffer;
}

void cVCDOutput::finish()
{
    if (pOut != nullptr) {
        fprintf(pOut, "#%" ELLELL "d\n", lastTime + 1);
        fprintf(pOut, "DONE!!!\n");
        pOut = nullptr;
    }
}

cVCDOutput::~cVCDOutput()
{
}
