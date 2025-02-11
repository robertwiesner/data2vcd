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
    cWire *getWire(int idx) { return 0 <= idx && idx < wires.size() ? wires[idx] : 0; }
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
