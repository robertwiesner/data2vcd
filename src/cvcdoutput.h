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

#ifndef CVCDOUTPUT_H
#define CVCDOUTPUT_H

#include <stdio.h>
#include <string.h>
#include "coutput.h"

class cVCDOutput : public cOutput {
    static const int maxRange = 127 - 33;
    size_t wireCount;
    size_t wireCountMax;
    int wireCountChar; 
    long   firstTimeOff;
    long long lastTime; 
    char *pRenameBuffer;
    size_t renameBufferSize;
    const char *shortWireGenerator(char *pBuffer) {
        *--pBuffer = 0;
        size_t val = wireCount;

        if (wireCountChar == 0) {
            size_t cnt = wireCountMax;

            while (cnt) {
                cnt = cnt / maxRange;
                wireCountChar ++;
            }
            wireCountChar = wireCountChar ? wireCountChar : 1;
        }
        int charCnt = wireCountChar;
        
        do {
            *--pBuffer = 33 + val % maxRange;
            val /= maxRange;
        } while (--charCnt);
        wireCount++;
        return pBuffer;
    }

    const char *printableCharOnly(const char *pStr) {
        size_t len = strlen(pStr);
        if (renameBufferSize <= len) {
            renameBufferSize = (len + 0x1000) & ~0xfff;
            if (pRenameBuffer) { delete[] pRenameBuffer;}
            pRenameBuffer = new char[renameBufferSize];
        }
        char *pDst = pRenameBuffer;
        char c;
        while ((c = *pStr++)) {
            if ('A' <= c && c <='Z') {
                *pDst++ = c;
            } else 
            if ('a' <= c && c <='z') {
                *pDst++ = c;
            } else 
            if ('0' <= c && c <='9') {
                *pDst++ = c;
            } else {
                *pDst++ = '_';
            }
        }
        *pDst = 0;
        return pRenameBuffer;
    }
    public:
    cVCDOutput(FILE *pO) : cOutput(pO) {
        wireCount     = 0;
        wireCountMax  = 0;
        firstTimeOff  = 0;
        wireCountChar = 0;
        renameBufferSize = 0;
        pRenameBuffer = 0;
    }

    virtual ~cVCDOutput();
    
    virtual const char *getStringValue(cWire *);

    virtual void headerStart();
    virtual void headerSetStartTime(long long);
    virtual void headerModuleStart(cModule *, std::string prefix);
    virtual void headerModuleEnd(cModule *);
    virtual void headerWire(cWire *, std::string prefix);
    virtual void headerEnd();
    
    virtual void setTime(long long);
    virtual void print(cWire *pW) { if (pOut && pW && pW->hasChanged()) { fprintf(pOut, "%s\n", getStringValue(pW)); }}

    virtual void finish();

};

#endif
