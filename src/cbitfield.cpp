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
    cModule     *pRet = pParent;
    cJSONobject *pV = dynamic_cast<cJSONobject *>(pBase);

    if (pV == 0) {
        return 0;
    }

    cJSONobject *pNodeList = dynamic_cast<cJSONobject *>(pV->search("<node>"));

    if (pNodeList) {
        // create the reference modules
        cModule     *pMod = 0;

        for (int nodeIdx = 0; nodeIdx < pNodeList->getSize(); nodeIdx++) {
            cJSONobject *pObj = dynamic_cast<cJSONobject *>(pNodeList->getValue(nodeIdx));
            cJSONscalar *pIdx = dynamic_cast<cJSONscalar *>(pObj->getValue("<idx>"));
            cJSONstring *pRef = dynamic_cast<cJSONstring *>(pObj->getValue("<ref>"));

            if (pObj == 0 || pIdx ==0 || pRef == 0) {
                return NULL;
            }

            size_t newModIdx = modIdx + pIdx->getValue();

            pMod = new cModule(pNodeList->getName(nodeIdx), pParent, pMod);
            entry[newModIdx]   = new sModWireInfo(pMod);
            module2index[pMod] = newModIdx;
            createModule(pMod, pNodeList->search(pRef->getValue()), newModIdx);
        }

        pRet = pMod;
    }

    cJSONobject *pWireList = dynamic_cast<cJSONobject *>(pV->search("<wire>"));

    if (pWireList) {
        // create Wires for the parent module
        unsigned long long bitStart  = 0;
        unsigned long long bitLength = 0;

        for (int wireIdx = 0; wireIdx < pWireList->getSize(); wireIdx++) {
            cJSONobject *pWire = dynamic_cast<cJSONobject *>(pWireList->getValue(wireIdx));
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

            pW = new cWire(eWT_BIT, bitLength, pWireList->getName(wireIdx));
            pParent->addWire(pW);
            entry[modIdx]->wires[(int) bitStart] = pW;
            bitStart += bitLength;
        }
    }
    return pRet->getFirstModule();
}

void
cBitfield::updateValue(unsigned long long id, int byteSize, const char *pPtr)
{
    sModWireInfo *pRegMap = entry[id];
    
    if (pRegMap) {
        int bitSize = 8*byteSize;
        unsigned char *pBuffer = new unsigned char[byteSize+1];
        for (std::map<int, cWire *>::iterator it = pRegMap->wires.begin(); it != pRegMap->wires.end(); it++) {
            int    start = it->first;
            int    bits  = start & 7;
            cWire *pW    = it->second;
            size_t len   = pW->getBits();
            int    idx   = start / 8;
            int    val   = pPtr[idx++] & 0xff;
            unsigned char *pVal = pBuffer;

            if (start + len <= bitSize) {
                for (size_t bitCnt = 0; bitCnt < len; bitCnt += 8) {
                    val |= (pPtr[idx++] & 0xff) << 8;
                    *pVal++ = val >> bits;
                    val >>= 8;
                }
                pW->setValue( (len+7) / 8, pBuffer);
            }
        }
        delete[] pBuffer;
    }
}
