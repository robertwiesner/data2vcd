
#ifndef COUTPUT_H
#define COUTPUT_H

#include <stdio.h>
#include <vector>
#include <string>

class cWire;
class cModule;

class cOutput {
    protected:
    struct sData {
        cModule *pModule;
        cWire   *pWire;
        sData(cModule *pM, cWire *pW) { pModule = pM; pWire = pW; }
    };
    std::vector<sData *>items;
    cModule *pLastModule;

    char aBuffer[1024];
    FILE *pOut; 
    public:
    cOutput(FILE *pO) {
        pOut = pO;
        pLastModule = NULL;
    }

    virtual ~cOutput();

    virtual const char *getStringValue(cWire *);
    virtual void headerStart();
    virtual void headerSetStartTime(long long);
    virtual void headerModuleStart(cModule *, std::string prefix);
    virtual void headerModuleEnd(cModule *);
    virtual void headerWire(cWire *, std::string prefix);
    virtual void headerEnd();
    
    virtual void setTime(long long);
    virtual void print(cWire *);

    virtual void flush();
    virtual void finish();

};

#endif
