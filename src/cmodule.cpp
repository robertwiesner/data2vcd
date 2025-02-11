

#include "cmodule.h"
#include "coutput.h"

void cModule::printHeader(cOutput *pO, std::string prefix)
{
    if (this == NULL) {
        return;
    }

    std::string newPrefix = 0 < prefix.size() ? prefix + "." + getName() : getName();

    if (pParent == NULL && pPrev == NULL) {
        pO->headerStart();
    }

    pO->headerModuleStart(this, prefix);

    for (std::vector<cWire *>::iterator it= wires.begin(); it != wires.end(); it++) {
        pO->headerWire(*it, newPrefix);
    }

    pChildren->printHeader(pO, newPrefix);

    pO->headerModuleEnd(this);
    
    pNext->printHeader(pO, prefix);

    if (pParent == NULL && pPrev == NULL) {
        pO->headerEnd();
    }
}
