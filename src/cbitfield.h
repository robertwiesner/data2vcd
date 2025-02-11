

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