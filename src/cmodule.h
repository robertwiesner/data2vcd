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

#ifndef CMODULE_H
#define CMODULE_H 1

#include <stdint.h>
#include <string>
#include <vector>
#include "cwire.h"

#include "ctrackmem.h"
#include "csiblingchild.h"

class cOutput;
// handle a single module and wires associate with this module
// 
class cModule : public cSiblingChild TRACKMEM_BASE_COM {
    std::string name;
    std::vector<cWire *> wires;
    int depth;
    public:
    cModule(const char *pName, cModule *parent = nullptr, cModule *prev = nullptr) : cSiblingChild(parent, prev) TRACKMEM_CONS_COM {
        depth = parent != nullptr ? parent->depth + 1: 0;
        name = pName;
    }

    ~cModule();

    cModule* getTop()         { return static_cast<cModule*>(cSiblingChild::getTop()); }
    cModule* getParent()      { return static_cast<cModule*>(cSiblingChild::getParent()); }
    cModule* getPrev()        { return static_cast<cModule*>(cSiblingChild::getPrev()); }
    cModule* getNext()        { return static_cast<cModule*>(cSiblingChild::getNext()); }
    cModule* getChildren()    { return static_cast<cModule*>(cSiblingChild::getChildren()); }
    cModule* getFirstModule() { return static_cast<cModule*>(getFirstSibling()); }
    cModule* getLastModule()  { return static_cast<cModule*>(getLastSibling()); }

    void addWire(cWire *pW) { wires.push_back(pW); }
    cWire *getWire(int idx) { return 0 <= idx && ((size_t) idx) < wires.size() ? wires[idx] : nullptr; }
    int getDepth() { return this ? depth : 0; }

    const char *getName() { return name.c_str(); }


    size_t getWireCount() {
        size_t ret = 0;
        for (cModule *p = getFirstModule(); p; p = p->getNext()) {
            cModule* pMod = p->getChildren();
            if (pMod != nullptr) {
                ret += pMod->getWireCount();
            }
            ret += p->wires.size();
        }
        return ret;
    }

    void printHeader(cOutput *pO, std::string prefix);

    cModule *searchModule(const char *pPath) {
        const char *pSlash = strchr(pPath, '/');
        size_t len = pSlash ? pSlash - pPath : strlen(pPath);
        cModule *pRet = nullptr;
        if (pPath == pSlash) {
            pRet = getTop();
        } else if (len == 2 && pPath[0] == '.' && pPath[1] == '.') {
            pRet = getParent();
            pRet = pRet ? pRet : this;
        } else if (len == 1 && pPath[0] == '.') {
            pRet = this;
        } else {
            pRet = getFirstModule();
            while (pRet != nullptr && strncmp(pRet->getName(), pPath, len)) {
                pRet++;
            }
        }
        if (pRet != nullptr && pPath && pPath[len] == '/') {
            pRet = pRet->searchModule(pPath + len + 1);
        }

        return pRet;
    }
};
    
#endif
