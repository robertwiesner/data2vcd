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

#ifndef SRC_CBITFIELD_H_
#define SRC_CBITFIELD_H 1

#include <stdint.h>
#include <stdio.h>
#include <map>
#include <string>
#include "cjson.h"
#include "cmodule.h"
#include "cwire.h"
#include "coutput.h"
#include "ctrackmem.h"

// collect the errors while processing the input objects
class cError {
    struct cEntry {
        int line;
        const char* pFkt;
        const char* pMsg;
        cEntry(int l, const char* pF, const char* pM) {
            line = l;
            pFkt = pF;
            pMsg = pM;
        }
    };
    std::vector<cEntry>errors;
public:
    cError() {}
    void addError(int line, const char* pFkt, const char* pMsg) {
        errors.push_back(cEntry(line, pFkt, pMsg));
    }
    size_t size() const { return errors.size(); }
    char * getLastError(size_t len, char *pBuffer) {
        if (errors.size()) {
            cEntry& rEntry = errors[errors.size() - 1];
            snprintf(pBuffer, len, "%s(%d): %s", rEntry.pFkt, rEntry.line, rEntry.pMsg);
            pBuffer[len - 1] = 0;
            errors.pop_back();
            return pBuffer;
        }
        else {
            return 0;
        }
    }
};

// handle the bitfield for a JSOn base module description
class cBitfield TRACKMEM_BASE_COL {
    cError  errors;
    cOutput *pOutput;
    cModule *pFirst;
    int streamCnt;
    int* pStreamID;
    const char* pBaseName;
    struct sModWireInfo {
        cModule *pMod;
        std::map<int, cWire *>wires;
        explicit sModWireInfo(cModule *pM) { pMod = pM; }
    };
    std::map<uint64_t, sModWireInfo *>entry;
    std::map<cModule *, uint64_t>module2index;

    // helper functions to create the module bitfields
    // These are based for the internal representation with "<node>" objects
    uint64_t createNewWireNode(cModule* pMod, uint64_t modIdx, const char *pName, cJSONbase*pObj, uint64_t start);
    cModule* createNewModuleNode(cModule* pParent, cModule* pMod, cJSONbase*, uint64_t, const char *);
    cModule* createAllModuleNode(cModule* pParent, cJSONbase*, uint64_t);

    // This is using the format desciption use by the ELA600 JSON object
    bool createNewWireIPX(cModule* pMod, uint64_t modIdx, const char* pName, cJSONbase* pBits, cJSONbase* pQual, bool isSingle);
    cModule* createNewModuleIPX(cModule* pParent, cModule* pMod, cJSONarray*, uint64_t, const char*);
    cModule* createAllModuleIPX(cModule* pParent, cJSONarray*, uint64_t);
    cModule *createAllModule(cModule *pParent, cJSONbase *, uint64_t);
public:
    cBitfield(cOutput *pO, cJSONbase *pJSON = 0);
    cBitfield(cOutput &rO, cJSONbase *pJSON = 0) : cBitfield(&rO, pJSON) { }
    ~cBitfield();

    void printAllError() {
        char aBuffer[4096];
        char* pErr;
        while ((pErr = errors.getLastError(sizeof(aBuffer), aBuffer)) != nullptr) {
            printf("%s\n", pErr);
        }
    }

    size_t getErrorCount() { return errors.size(); }
    char* getLastError(size_t len, char* pBuffer) {
        return errors.getLastError(len, pBuffer);
    }

    void addJsonModules(cJSONbase* pBase, uint64_t base, int streamCount = 0, int* pStreamId = 0, const char* pBaseName = 0);
    void printHeader(const char *pPrefix);
    void setTime(uint64_t);

    void updateValue(uint64_t id, int byteSize, const void *pPtr);

    void updateValuePtr(cModule *pMod, int byteSize, const void *pPtr) {
        std::map<cModule *, uint64_t>::iterator it = module2index.find(pMod);
        if (it != module2index.end()) {
            updateValue(it->second, byteSize, pPtr);
        }
        else {
            errors.addError(__LINE__, __func__, "module index not found");
        }
    }

    void updateValuePtr(const char *pName, int byteSize, const void *pPtr) {
        cModule *pMod = pFirst->searchModule(pName);
        if (pMod != nullptr) {
            updateValuePtr(pMod, byteSize, pPtr);
        }
        else {
            errors.addError(__LINE__, __func__, "module index not found");
        }
    }
    cModule *getFirstModule() { return pFirst; }

    void flush();
    void finish();
};

#endif  // SRC_CBITFIELD_H_

