

#include "cbitfield.h"

cBitfield::cBitfield(cJSONbase *pJSON)
{
    pFirst = createModule(0, pJSON, 0);
}


cBitfield::~cBitfield()
{

}


cModule *
cBitfield::createModule(cModule *pParent, cJSONbase *pBase, unsigned long long modIdx)
{
    int idx = 0;
    cModule     *pMod = 0;
    cJSONobj    *pO;
    cJSONobject *pV = dynamic_cast<cJSONobject *>(pBase);

    while (pO = pV->getObj(idx++)) {
        if (0 == strcmp(pO->getName(), "<node>")) {
            cJSONobject *pNode = dynamic_cast<cJSONobject *>(pO->getValue());
            cJSONobj *pNodeObj;
            int nodeIdx = 0;
            while (pNodeObj = pNode->getObj(nodeIdx++)) {
                cJSONobject *pIndex = dynamic_cast<cJSONobject *>(pNodeObj->getValue());
                cJSONscalar *pIdx = dynamic_cast<cJSONscalar *>(pIndex->getValue("<idx>"));
                int newModIdx = modIdx + (pIdx ? pIdx->getValue() : 0);
                pMod = new cModule(pNodeObj->getName(), pParent, pMod);
                entry[newModIdx] = new sModWireInfo(pMod);
                createModule(pMod, pNodeObj->getValue(), newModIdx);
            }
        } else if (0 == strcmp(pO->getName(), "<ref>")) {
            cJSONobject *pNode = dynamic_cast<cJSONobject *>(pBase->getParent()->getParent());
            cJSONstring *pRef  = dynamic_cast<cJSONstring *>(pO->getValue());

            if (pRef && pNode) {
                cJSONobj *pNodeObj = pNode->getObj(pRef->getValue());
                createModule(pParent, pNodeObj->getValue(), modIdx);
            }
        } else if (0 == strcmp(pO->getName(), "<wire>")) {
            // create Wires for the parent module
            int wireIdx = 0;
            cJSONobject *pWireList = dynamic_cast<cJSONobject *>(pO->getValue());
            cJSONobj *pWireObj;

            while (pWireObj = pWireList->getObj(wireIdx++)) {
                cJSONobject *pWire = dynamic_cast<cJSONobject *>(pWireObj->getValue());
                cWire *pW;
                int start = dynamic_cast<cJSONscalar *>(pWire->getValue("start"))->getValue();
                int end   = dynamic_cast<cJSONscalar *>(pWire->getValue("end"))->getValue();
                pW = new cWire(WT_BITS, end - start + 1, pWireObj->getName());
                pParent->addWire(pW);
                entry[modIdx]->wires[start] = pW;
            }

        } else {
            // skip the none special entries
        }
    }
    return pMod->getFirstModule();

}

void
cBitfield::updateValue(unsigned long long id, int size, const char *pPtr)
{
    sModWireInfo *pRegMap = entry[id];
    
    if (pRegMap) {
        unsigned char *pBuffer = new unsigned char[size+1];
        for (std::map<int, cWire *>::iterator it = pRegMap->wires.begin(); it != pRegMap->wires.end(); it++) {
            int    start = it->first;
            int    bits  = start & 7;
            cWire *pW    = it->second;
            int    len   = pW->getBits();
            int    idx   = start / 8;
            int    val   = pPtr[idx++] & 0xff;
            unsigned char *pVal = pBuffer;

            for (int bitCnt = 0; bitCnt < len; bitCnt += 8) {
                val |= (pPtr[idx++] & 0xff) << 8;
                *pVal++ = val >> bits;
                val >>= 8;
            }
            pW->setValue( (len+7) / 8, pBuffer);
        }
        delete pBuffer;
    }
}
