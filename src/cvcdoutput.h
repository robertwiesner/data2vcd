#ifndef CVCDOUTPUT_H
#define CVCDOUTPUT_H

#include <stdio.h>
#include "coutput.h"

class cVCDOutput : public cOutput {
    size_t wireCount;
    size_t wireCountMax;
    const char *shortWireGenerator(char *pBuffer) {
        *--pBuffer = 0;
        size_t val = wireCount;
        do {
            *--pBuffer = 33 + val % (127 - 33);
            val /= (127 - 33);
        } while (val != 0);
        wireCount++;
        return pBuffer;
    }
    public:
    cVCDOutput(FILE *pO) : cOutput(pO) {
        wireCount = 0;
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
    virtual void print(cWire *);

    virtual void finish();

};

#endif
