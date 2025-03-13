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

#ifndef CBITFIELD_H
#define CBITFIELD_H 1

#include <stdio.h>
#include <map>
#include "cjson.h"
#include "cmodule.h"
#include "cwire.h"
#include <string>
#include "coutput.h"

class cBitfield {
    cOutput &rOutput;
    cModule *pFirst;
    struct sModWireInfo {
        cModule *pMod;
        std::map<int, cWire *>wires;
        sModWireInfo(cModule *pM) { pMod = pM; }
    };
    std::map<unsigned long long, sModWireInfo *>entry;
    std::map<cModule *, unsigned long long>module2index;
    cModule *createModuleNode(cModule *pParent, cJSONbase *, unsigned long long);
    cModule *createModuleIPX(cModule *pParent, cJSONbase *, unsigned long long);
    cModule *createModule(cModule *pParent, cJSONbase *, unsigned long long);
public:
    cBitfield(cOutput &rO, cJSONbase *pJSON);
    ~cBitfield();

    void addJsonModule(cJSONbase *pBase, unsigned long long base);
    void printHeader(const char *pPrefix);
    void setTime(unsigned long long);
    
    void updateValue(unsigned long long id, int byteSize, const void *pPtr);

    void updateValue(cModule *pMod, int byteSize, const void *pPtr) {
        std::map<cModule *, unsigned long long>::iterator it = module2index.find(pMod);
        if (it != module2index.end()) {
            updateValue(it->second, byteSize, pPtr);
        }
    }

    void updateValue(const char *pName, int byteSize, const void *pPtr) {
        cModule *pMod = pFirst->searchModule(pName);
        updateValue(pMod, byteSize, pPtr);
    }
    cModule *getFirstModule() { return pFirst; }

    void flush();
    void finish();
};

#endif