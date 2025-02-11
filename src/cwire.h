#ifndef CWIRE_H
#define CWIRE_H

#include <string>
#include <string.h>

enum eWireType {
    WT_NONE,
    WT_BOOL,
    WT_BITS,
    WT_HEX,
    WT_STR,
};

class cWire {
    size_t index; 
    size_t bits;
    eWireType wireType;
    unsigned char *pBuffer;
    std::string name;
    std::string shortName;
    bool changed;
    public:
    cWire(eWireType wt, size_t b, const char *pName) {
        wireType = wt;
        bits = b;
        name = pName;
        shortName = "";
        pBuffer = new unsigned char[(b + 7) / 8];
        changed = 0;
        index = 0;
    }

    void setIndex(size_t idx) { index = idx; }
    size_t getIndex() { return index; }

    void setShortName(const char *pSN) {
        shortName = pSN;
    }

    size_t      getBits() { return bits; }
    eWireType   getWireType() { return wireType; }
    const char *getName()      { return name.c_str(); }
    const char *getShortName() { return shortName.c_str(); }

    void setValue(size_t bytes, unsigned char *pV) {
        if (memcmp(pBuffer, pV, bytes)) {
            changed = true;
            memcpy(pBuffer, pV, 8 * bytes < bits ? bytes : (bits+7)/8);
        }
    }
    void setValue(unsigned int val) {
        setValue(sizeof(val), (unsigned char *) &val);
    }
    void setValue(unsigned long long val) {
        setValue(sizeof(val), (unsigned char *) &val);
    }

    bool isSet() { bool ret = changed; changed = false; return ret; }
    const unsigned char *getBuffer() { return pBuffer; }
};

#endif

