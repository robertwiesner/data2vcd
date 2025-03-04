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

#ifndef COUTPUT_H
#define COUTPUT_H

#include <stdio.h>
#include <vector>
#include <string>
#include "cwire.h"

class cModule;

class cOutput {
    protected:
    struct sData {
        cModule *pModule;
        cWire   *pWire;
        sData(cModule *pM, cWire *pW) { pModule = pM; pWire = pW; }
    };
    std::vector<sData *>items;
    std::vector<std::string>errors;
    cModule *pLastModule;

    char aBuffer[1024];
    FILE *pOut; 
    public:
    cOutput(FILE *pO = NULL) {
        pOut = pO;
        pLastModule = NULL;
    }

    void addError(const char *pErr) { errors.push_back(std::string(pErr)); }
    void addError(std::string err) { errors.push_back(err); }
    virtual ~cOutput();

    void setOutputFile(FILE *pO) { pOut = pO; }
    
    virtual const char *getStringValue(cWire *);
    virtual void headerStart();
    virtual void headerSetStartTime(long long);
    virtual void headerModuleStart(cModule *, std::string prefix);
    virtual void headerModuleEnd(cModule *);
    virtual void headerWire(cWire *, std::string prefix);
    virtual void headerEnd();
    
    virtual void setTime(long long);
    virtual void print(cWire *pW) {
        if (pOut && pW && pW->hasChanged()) { 
            fprintf(pOut, "%s: 0x%0*llX\n", pW->getName(),
                    (((int) pW->getBits()) + 7) / 8, 
                    pW->getAsUnsignedLongLong());
        }
    }

    virtual void flush();
    virtual void finish();

};

#endif
