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
#include "ctmpbuf.h"

cBitfield::cBitfield(cOutput *pO, cJSONbase *pJSON) TRACKMEM_CONS_COL
{
    pOutput   = pO;
    streamCnt = 0;
    pStreamID = nullptr;
    pBaseName = nullptr;
    pFirst    = nullptr;

    if (pJSON != nullptr) {
        pFirst = createAllModule(0, pJSON, 0);
    }
}

void
cBitfield::addJsonModules(cJSONbase *pJSON, uint64_t base, int cnt, int* pSId, const char* pBN)
{
    streamCnt = cnt;
    pStreamID = pSId;
    pBaseName = pBN;
    pFirst = createAllModule(0, pJSON, base);
}

cBitfield::~cBitfield()
{
    pOutput->finish();

    cModule* pThisMod = pFirst;
    while (pThisMod != nullptr) {
        cModule* pNextMod = pThisMod->getNext();
        delete pThisMod;
        pThisMod = pNextMod;
    }
    for (auto &it : entry) {
        delete it.second;
    }
}

void
cBitfield::printHeader(const char *pPrefix)
{
    cModule* pM = getFirstModule();

    if (pM != nullptr) {
        pM->printHeader(pOutput, pPrefix);
    }
}

void
cBitfield::setTime(uint64_t tim)
{
    pOutput->setTime(tim);
}

void
cBitfield::flush()
{
    pOutput->flush();
}

void
cBitfield::finish()
{
    pOutput->finish();
}


uint64_t
cBitfield::createNewWireNode(cModule* pParent, uint64_t modIdx, const char* pName, cJSONbase*pO, uint64_t start)
{
    cJSONobject* pWire = dynamic_cast<cJSONobject*>(pO);

    if (pWire == nullptr) {
        errors.addError(__LINE__, __func__, "pWire object not a wire");
        return -1;
    }
    cJSONscalar* pStart = dynamic_cast<cJSONscalar*>(pWire->getValue("start"));
    cJSONscalar* pLen = dynamic_cast<cJSONscalar*>(pWire->getValue("len"));
    cJSONscalar* pEnd = dynamic_cast<cJSONscalar*>(pWire->getValue("end"));
    cJSONscalar* pVal0 = dynamic_cast<cJSONscalar*>(pWire->getValue("valid0"));
    cJSONscalar* pVal1 = dynamic_cast<cJSONscalar*>(pWire->getValue("valid1"));

    start = pStart ? pStart->getValue() : start;
    uint64_t len = -1;

    if (pLen != nullptr) {
        if (pEnd != nullptr && pEnd->getValue() != (start + len - 1)) {
            errors.addError(__LINE__, __func__, "start+len-1 does not mach end value");
            return -1;
        }
        len = pLen->getValue();
    }
    else if (pEnd != nullptr) {
        uint64_t bitEnd = pEnd->getValue();
        if (bitEnd < start) {
            errors.addError(__LINE__, __func__, "bitEnd < start value");
            return -1;
        }
        len = bitEnd - start + 1;
    }

    cWire* pW = new cWire(eWT_BIT, len, pName);
    if (pVal0 != nullptr) {
        pW->updateValid0(pVal0->getValue());
    }

    if (pVal1 != nullptr) {
        pW->updateValid1(pVal1->getValue());
    }

    pParent->addWire(pW);
    entry[modIdx]->wires[static_cast<int>(start)] = pW;

    return start + len;
}

cModule*
cBitfield::createNewModuleNode(cModule* pParent, cModule* pModule, cJSONbase* pO, uint64_t modIdx, const char *pName)
{
    cJSONobject* pObj = dynamic_cast<cJSONobject*>(pO);
    if (pObj == nullptr) {
        errors.addError(__LINE__, __func__, "input not a JSON object");
    }

    cJSONscalar* pIdx = dynamic_cast<cJSONscalar*>(pObj->getValue("<idx>"));
    cJSONstring* pRef = dynamic_cast<cJSONstring*>(pObj->getValue("<ref>"));

    if (pIdx == nullptr || pRef == nullptr) {
        errors.addError(__LINE__, __func__, "entries <idx> and <ref> are misssing");
        return nullptr;
    }

    size_t newModIdx = modIdx + pIdx->getValue();

    cModule *pRet = new cModule(pName, pParent, pModule);
    entry[newModIdx] = new sModWireInfo(pRet);
    module2index[pRet] = newModIdx;
    createAllModuleNode(pRet, pO->getParent()->search(pRef->getValue()), newModIdx);

    return pRet;
}

cModule *
cBitfield::createAllModuleNode(cModule *pParent, cJSONbase *pBase, uint64_t modIdx)
{
    cModule     *pRet = pParent;
    cJSONobject *pV = dynamic_cast<cJSONobject *>(pBase);

    if (pV == nullptr) {
        errors.addError(__LINE__, __func__, "input not a JSON object");
        return nullptr;
    }

    cJSONobject *pNodeList = dynamic_cast<cJSONobject *>(pV->search("<node>"));

    if (pNodeList != nullptr) {
        // create the reference modules
        cModule *pMod = pParent && pParent->getChildren() ? pParent->getChildren()->getLastModule() : 0;
        pMod = pMod || pParent ? pMod : pFirst->getLastModule();

        for (int nodeIdx = 0; nodeIdx < pNodeList->getSize(); nodeIdx++) {
            pMod = createNewModuleNode(pParent, pMod, pNodeList->getValue(nodeIdx), modIdx, pNodeList->getName(nodeIdx));
            if (pMod == nullptr) {
                return pMod;
            }
        }

        pRet = pMod;
    }

    cJSONobject *pWireList = dynamic_cast<cJSONobject *>(pV->search("<wire>"));

    if (pWireList != nullptr) {
        // create Wires for the parent module
        uint64_t bitStart  = 0;

        for (int wireIdx = 0; wireIdx < pWireList->getSize(); wireIdx++) {
            bitStart = createNewWireNode(pParent, modIdx, pWireList->getName(wireIdx), pWireList->getValue(wireIdx), bitStart);
            if (bitStart == 0xFFFFFFFFFFFFFFFFULL) {
                errors.addError(__LINE__, __func__, "bitStart returned as -1");
                return nullptr;
            }
        }
    }

    return pRet->getFirstModule();
}

bool
cBitfield::createNewWireIPX(cModule* pMod, uint64_t modIdx, const char* pName, cJSONbase* pBitVal, cJSONbase* pQual, bool isSingle)
{
    char aBuffer[1024];
    cJSONstring* pBitStr    = dynamic_cast<cJSONstring*>(pBitVal);
    cJSONstring* pQualifier = dynamic_cast<cJSONstring*>(pQual);

    if (pBitStr == nullptr) {
        errors.addError(__LINE__, __func__, "bitStr object bot a JSONstring");
        return false;
    }
    const char* pBits = pBitStr->getValue();
    const char* pCol = strchr(pBits, ':');

    int start, end;

    if (pCol != nullptr) {
        end = strtol(pBits, 0, 0);
        start = strtol(pCol + 1, 0, 0);
        if (end < start) {
            int tmp = end;
            end = start;
            start = tmp;
        }
    }
    else {
        start = end = strtol(pBits, 0, 0);
    }

    if (false == isSingle) {  // rename the signals if there are multiple bitfields
        if (start == end) {
            snprintf(aBuffer, sizeof((aBuffer)), "%s_%d", pName, start);
        }
        else {
            snprintf(aBuffer, sizeof(aBuffer), "%s_%d_%d", pName, end, start);
        }
        pName = aBuffer;
    }

    cWire* pW = new cWire(eWT_BIT, end - start + 1, pName);  //  pWireList->getName(wireIdx)

    if (pQualifier != nullptr) {
        uint64_t bit = strtoull(pQualifier->getValue(), 0, 0);
        pW->updateValid0(bit);
    }

    pMod->addWire(pW);
    entry[modIdx]->wires[static_cast<int>(start)] = pW;

    return true;

}

cModule*
cBitfield::createNewModuleIPX(cModule* pParent, cModule* pModule, cJSONarray* pSignals, uint64_t modIdx, const char* pName)
{
    cModule* pRet = new cModule(pName, pParent, pModule);
    entry[modIdx] = new sModWireInfo(pRet);
    module2index[pRet] = modIdx;

    // the module is created, now create all wires
    for (int signalIdx = 0; signalIdx < pSignals->getSize(); signalIdx++) {
        cJSONobject* pSignal = dynamic_cast<cJSONobject*>(pSignals->getValue(static_cast<int>(signalIdx)));
        if (pSignal == nullptr) {
            errors.addError(__LINE__, __func__, "pSignal JSON object is missing");
            return 0;
        }
        cJSONstring* pName       = dynamic_cast<cJSONstring*>(pSignal->getValue("name"));
        cJSONstring* pDebugName  = dynamic_cast<cJSONstring*>(pSignal->getValue("debugname"));
        cJSONarray*  pBitmap     = dynamic_cast<cJSONarray*>(pSignal->search("bitmap"));
        cJSONarray*  pQualifier  = dynamic_cast<cJSONarray*>(pSignal->search("qualifier"));

        if (pName == nullptr) {
            errors.addError(__LINE__, __func__, "pName JSON string is missing");
            return 0;
        }
        if (pDebugName == nullptr) {
            errors.addError(__LINE__, __func__, "pDebugName JSON string is missing");
            return 0;
        }
        if (pBitmap == nullptr) {
            errors.addError(__LINE__, __func__, "pBitmap JSON array is missing");
            return 0;
        }

        for (auto &it : *pBitmap) {  // int bitIdx = 0; bitIdx < pBitmap->getSize(); bitIdx++) {
            cJSONstring* pQ = nullptr;
            if (pQualifier != nullptr) {
                if (pQualifier->getSize() == 1) {
                    pQ = dynamic_cast<cJSONstring*>(pQualifier->getValue(0));
                }
            }
            if (false == createNewWireIPX(pRet, modIdx, pDebugName->getValue(), it, pQ, pBitmap->getSize() == 1)) {
                return 0;
            }
        }
    }

    return pRet;
}

cModule*
cBitfield::createAllModuleIPX(cModule* pParent, cJSONarray* pIParr, uint64_t modIdx)
{
    cModule* pMod = nullptr;

    if (0xff < pIParr->getSize()) {
        errors.addError(__LINE__, __func__, "JSON IP array contains too many IP entries in 'ipnames' (max: 256)");
        return 0;
    }

    for (int ipIdx = 0; ipIdx < pIParr->getSize(); ipIdx++) {
        cJSONobject* pIP = dynamic_cast<cJSONobject*>(pIParr->getValue(static_cast<int>(ipIdx)));
        if (pIP == nullptr) {
            errors.addError(__LINE__, __func__, "pIP JSON object is missing");
            return 0;
        }
        cJSONstring* pIPname = dynamic_cast<cJSONstring*>(pIP->getValue("ipname"));
        cJSONarray* pSigGrps = dynamic_cast<cJSONarray*>(pIP->search("signalgrps"));
        if (pIPname == nullptr) {
            errors.addError(__LINE__, __func__, "pIPname JSON string is missing");
            return 0;
        }
        if (pSigGrps == nullptr) {
            errors.addError(__LINE__, __func__, "pSigGrps JSON array is missing");
            return 0;
        }

        for (int sigIdx = 0; sigIdx < pSigGrps->getSize(); sigIdx++) {
            cJSONobject* pSigGrp  = dynamic_cast<cJSONobject*>(pSigGrps->getValue(static_cast<int>(sigIdx)));
            if (pSigGrp == nullptr) {
                errors.addError(__LINE__, __func__, "pSigGrp JSON object is missing");
                return 0;
            }
            cJSONscalar* pSignalGrp = dynamic_cast<cJSONscalar*>(pSigGrp->getValue("signalgrp"));
            cJSONarray*  pSignals   = dynamic_cast<cJSONarray*>(pSigGrp->search("signals"));

            if (pSignalGrp == nullptr) {
                errors.addError(__LINE__, __func__, "pSignalGrp JSON scalar is missing");
                return 0;
            }
            if (pSignals == nullptr) {
                errors.addError(__LINE__, __func__, "pSignals JSON array is missing");
                return 0;
            }

            if (pSignals->getSize() == 0) {
                // this is an empty module
                continue;
            }
            if (pSignals->getSize() == 1) {
                cJSONobject* pO = dynamic_cast<cJSONobject*>(pSignals->getValue(0));
                cJSONstring* pN = pO ? dynamic_cast<cJSONstring*>(pO->search("name")) : 0;

                if (pN != nullptr && 0 == strcmp(pN->getValue(), "not connected")) {
                    // just skip it, non of the signal is of any interrest.
                    continue;
                }
            }
            // create the module and use modIdx + pSignalGrp value as a module index
            // and <ipname>_<idx> as the module name
            char aBuffer[1024];
            snprintf(aBuffer, sizeof(aBuffer) - 1, "%s_%d", pIPname->getValue(), static_cast<int>(pSignalGrp->getValue()));
            pMod = createNewModuleIPX(pParent, pMod, pSignals, modIdx + (ipIdx << 16) + pSignalGrp->getValue(), aBuffer);
        }
    }
    return pMod->getTop();
}

cModule*
cBitfield::createAllModule(cModule* pParent, cJSONbase* pBase, uint64_t modIdx)
{
    cJSONobject* pNodeList = dynamic_cast<cJSONobject*>(pBase->search("<node>"));
    if (pNodeList != nullptr) {
        return createAllModuleNode(pParent, pBase, modIdx);
    }

    cJSONarray *pNodeArray = dynamic_cast<cJSONarray*>(pBase->search("ipnames"));

    if (pNodeArray != nullptr) {
        if (1 < streamCnt) {
            int nibbleCnt = 0;
            for (int idx = 0; idx < streamCnt; idx++) {
                int nc = pStreamID[idx] < 0x10 ? 1 : 2;
                nibbleCnt = nibbleCnt < nc ? nc : nibbleCnt;
            }
            const char *pFormat = nibbleCnt == 1 ? "%s_%1X" : "%s_%02X";
            cModule* pModule = 0;
            for (int idx = 0; idx < streamCnt; idx++) {
                char aNewName[1024];
                snprintf(aNewName, sizeof(aNewName), pFormat, pBaseName, pStreamID[idx]);
                // create the pseudo module and embedd the stream ID
                pModule = new cModule(aNewName, pParent, pModule);
                createAllModuleIPX(pModule, pNodeArray, (static_cast<uint64_t>(pStreamID[idx])) << 24);
            }
            return pModule->getTop();
        }
        else {
            return createAllModuleIPX(pParent, pNodeArray, modIdx);
        }
    }
    errors.addError(__LINE__, __func__, "pBase JSON object does not contain member '<node>' or 'ipnames'");
    return 0;
}

void
cBitfield::updateValue(uint64_t id, int byteSize, const void *pData)
{
    const char *pPtr = static_cast<const char *>(pData);
    sModWireInfo *pRegMap = entry[id];

    if (pRegMap != nullptr) {
        int bitSize = 8*byteSize;
        cTmpBuf tmpbuf(byteSize + 1);
        for (auto& it : pRegMap->wires) {
            int    start = it.first;
            int    bits  = start & 7;
            cWire *pW    = it.second;
            size_t len   = pW->getBits();
            int    idx   = start / 8;
            int    val   = pPtr[idx++] & 0xff;
            unsigned char *pVal = tmpbuf.getUnsignedBuffer();

            if (start + len <= (static_cast<size_t>(bitSize))) {
                for (size_t bitCnt = 0; bitCnt < len; bitCnt += 8) {
                    val |= (pPtr[idx++] & 0xff) << 8;
                    *pVal++ = val >> bits;
                    val >>= 8;
                }
                if (pW->isValid(static_cast<const char*>(pData))) {
                    pW->setValue((len + 7) / 8, tmpbuf.getUnsignedBuffer());
                }
                else {
                    pW->setInvalid();
                }
            }
        }
    }
    else {
        errors.addError(__LINE__, __func__, "module index not found");
    }
}

