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

class cBitfield {
    cModule *pFirst;
    struct sModWireInfo {
        cModule *pMod;
        std::map<int, cWire *>wires;
        sModWireInfo(cModule *pM) { pMod = pM; }
    };
    std::map<unsigned long long, sModWireInfo *>entry;
    cModule *createModule(cModule *pParent, cJSONbase *, unsigned long long);
    public:
    cBitfield(cJSONbase *pJSON);
    ~cBitfield();
    void updateValue(unsigned long long id, int size, const char *pPtr);
    cModule *getFirstModule() { return pFirst; }
};

#endif