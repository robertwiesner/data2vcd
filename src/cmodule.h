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
#define CMODULE_H

#include <string>
#include <vector>
#include "cwire.h"

class cOutput;

class cModule {
    cModule *pParent;
    cModule *pChildren;
    cModule *pPrev;
    cModule *pNext;
    std::string name;
    std::vector<cWire *> wires;
    int depth;
    public:
    cModule(const char *pName, cModule *parent = NULL, cModule *prev = NULL) {
        depth = parent ? parent->depth + 1: 0;
        name = pName;
        pParent = parent;
        if (parent && parent->pChildren == NULL) {
            parent->pChildren = this;
        }
        if (prev) {
            while (prev->pNext) {
                prev = prev->pNext;
            }
            prev->pNext = this;
        }
        pPrev = prev;
        pNext = NULL;
    }

    void addWire(cWire *pW) { wires.push_back(pW); }
    cWire *getWire(int idx) { return 0 <= idx && ((size_t) idx) < wires.size() ? wires[idx] : 0; }
    int getDepth() { return this ? depth : 0; }

    const char *getName() { return name.c_str(); }

    cModule *getFirstModule() {
        cModule *pRet = this;
        if (pRet) {
            while (pRet->pPrev) {
                pRet = pRet->pPrev;
            }
        }
        return pRet;
    }

    size_t getWireCount() {
        size_t ret = 0;
        for (cModule *p = getFirstModule(); p; p = p->pNext) {
            ret += p->pChildren->getWireCount();
            ret += p->wires.size();
        }
        return ret;
    }

    void printHeader(cOutput *pO, std::string prefix);
};

#endif
