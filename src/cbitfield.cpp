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

#include "cbitfield.h"

cBitfield::cBitfield(cJSONbase *pJSON, cOutput &rO) : rOutput(rO)
{
    pFirst = createModule(0, pJSON, 0);
}


cBitfield::~cBitfield()
{
    rOutput.finish();
}

void
cBitfield::printHeader(const char *pPrefix)
{
    getFirstModule()->printHeader(&rOutput, pPrefix);
}

void
cBitfield::setTime(unsigned long long tim)
{
    rOutput.setTime(tim);
}

void
cBitfield::flush()
{
    rOutput.flush();
}

void
cBitfield::finish()
{
    rOutput.finish();
}

cModule *
cBitfield::createModule(cModule *pParent, cJSONbase *pBase, unsigned long long modIdx)
{
    int idx = 0;
    cModule     *pMod = 0;
    cJSONobj    *pO;
    cJSONobject *pV = dynamic_cast<cJSONobject *>(pBase);

    while (0 != (pO = pV->getObj(idx++))) {
        if (0 == strcmp(pO->getName(), "<node>")) {
            cJSONobject *pNode = dynamic_cast<cJSONobject *>(pO->getValue());
            cJSONobj *pNodeObj;
            int nodeIdx = 0;
            while (0 != (pNodeObj = pNode->getObj(nodeIdx++))) {
                cJSONobject *pIndex = dynamic_cast<cJSONobject *>(pNodeObj->getValue());
                cJSONscalar *pIdx = dynamic_cast<cJSONscalar *>(pIndex->getValue("<idx>"));
                int newModIdx = modIdx + (pIdx ? pIdx->getValue() : 0);
                pMod = new cModule(pNodeObj->getName(), pParent, pMod);
                entry[newModIdx]   = new sModWireInfo(pMod);
                module2index[pMod] = newModIdx;
                createModule(pMod, pNodeObj->getValue(), newModIdx);
            }
        } else if (0 == strcmp(pO->getName(), "<ref>")) {
            cJSONstring *pRef  = dynamic_cast<cJSONstring *>(pO->getValue());

            if (pRef) {
                cJSONbase *pNode = pV->search(pRef->getValue());
                createModule(pParent, pNode, modIdx);
            }
        } else if (0 == strcmp(pO->getName(), "<wire>")) {
            // create Wires for the parent module
            int wireIdx = 0;
            unsigned long long bitStart  = 0;
            unsigned long long bitLength = 0;
            cJSONobject *pWireList = dynamic_cast<cJSONobject *>(pO->getValue());
            cJSONobj *pWireObj;

            while (0 != (pWireObj = pWireList->getObj(wireIdx++))) {
                cJSONobject *pWire = dynamic_cast<cJSONobject *>(pWireObj->getValue());
                cWire *pW;
                cJSONscalar *pStart = dynamic_cast<cJSONscalar *>(pWire->getValue("start"));
                cJSONscalar *pLen = dynamic_cast<cJSONscalar *>(pWire->getValue("len"));
                cJSONscalar *pEnd = dynamic_cast<cJSONscalar *>(pWire->getValue("end"));

                bitStart = pStart ? pStart->getValue() : bitStart;

                if (pLen) {
                    if (pEnd) {
                        // later error handling
                    }
                    bitLength = pLen->getValue();
                } else if (pEnd) {
                    unsigned long long bitEnd = pEnd->getValue();
                    if (bitEnd < bitStart) {
                        // later error 
                    }
                    bitLength = bitEnd - bitStart + 1;
                }

                pW = new cWire(eWT_BIT, bitLength, pWireObj->getName());
                pParent->addWire(pW);
                entry[modIdx]->wires[bitStart] = pW;
                bitStart += bitLength;
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
        delete[] pBuffer;
    }
}
