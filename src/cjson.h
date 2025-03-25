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

#ifndef SRC_CJSON_H_
#define SRC_CJSON_H_ 1

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iterator>
#include "ctrackmem.h"

enum eJSONtype {
    eJT_NONE,
    eJT_BOOL,
    eJT_SCALAR,
    eJT_FLOAT,
    eJT_STRING,
    eJT_ARRAY,
    eJT_OBJECT,
};
// implementation fo a JSON reader, write, and creator
class cJSONbase TRACKMEM_BASE_COL {
    cJSONbase *pParent;
    eJSONtype type;

    cJSONbase *searchArray(const char *pStr);
    cJSONbase *searchObject(const char *pStr);
    protected:
 
    char *addDeepString(int &rLen, char *pBuffer, int deep, char c, bool first)
    {
        deep *= 3;
        rLen -= 2 + 2 * deep;
        if (0 < rLen) {
            if (true == first) { *pBuffer++ = c; }

            *pBuffer++ = '\n';
            memset(pBuffer, ' ', deep);
            pBuffer += deep;

            if (false == first) { *pBuffer++ = c; }
            *pBuffer = 0;
        }
        return pBuffer;
    }
    char *addDeepStringFirst(int &rLen, char *pBuffer, int deep, char c) {
            return addDeepString(rLen, pBuffer, deep, c, true);
    }

    char *addDeepStringLast(int &rLen, char *pBuffer, int deep, char c) {
        return addDeepString(rLen, pBuffer, deep, c, false);
    }

    static bool isCommaOrClose(char c) {
        return c == ',' || c == '}' || c == ']';
    }

    static bool isScalar(const char* pS) {
        if (pS[0] == '-' || pS[0] == '+') {
            pS++;
        }
        const char* pStart = pS;
        if (pS[0] == '0') {
            if (pS[1] == 'x' || pS[1] == 'X') { // is HEX
                pS += 2;
                while (('0' <= pS[0] && pS[0] <= '9') || ('a' <= pS[0] && pS[0] <= 'f') || ('A' <= pS[0] && pS[0] <= 'F')) {
                    pS++;
                }
            }
            else { // is OCT
                while (('0' <= pS[0] && pS[0] <= '7')) {
                    pS++;
                }
            }
        }
        else { // decimal
            while (('0' <= pS[0] && pS[0] <= '9')) {
                pS++;
            }
        }

        if (pStart == pS) {
            return false;
        }
        pS = skipWS(pS);
        return isCommaOrClose(pS[0]);
    }

    static bool isFloat(const char* pS) {
        if (pS[0] == '-' || pS[0] == '+') {
            pS++;
        }
        const char* pDot = nullptr;
        const char* pExp = nullptr;

        while (pS[0]) {
            switch (pS[0]) {
            case '.':
                if (pDot != nullptr || pExp != nullptr) {
                    return false;
                }
                pDot = pS;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                pS++;
                break;
            case 'e':
            case 'E':
                if (pExp != nullptr) {
                    return false;
                }
                pExp = pS;
                break;
            case '-':
            case '+':
                if (pExp == nullptr || pS != pExp + 1) {
                    return false;
                }
            }
            pS++;
        }
        pS = skipWS(pS);
        return (pDot != nullptr || pExp != nullptr) && isCommaOrClose(pS[0]);
    }

    public:
    cJSONbase(eJSONtype t, cJSONbase *pP) TRACKMEM_CONS_COL {
        pParent = pP;
        type = t;
    }

    virtual ~cJSONbase() {
        if (pParent != nullptr) {
            pParent->removeObject(this);
        }
    }

    virtual int getSize() const { return -1; }

    bool setParent(cJSONbase* pP) {
        if (pParent == nullptr) {
            pParent = pP;
        }
        return pParent == pP;
    }

    bool isEnd(char c) { return c <= ' '; }

    const char *getName(const char *pStr, char *pBuffer, char *pBufferEnd) {
        pBufferEnd -= 1;
        while (*pStr && *pStr != '/' && *pStr != '[' && pBuffer != pBufferEnd ) {
            *pBuffer++ = *pStr++;
        }
        *pBuffer = 0;
        return pStr;
    }

    eJSONtype getType() const { return this ? type : eJT_NONE; }

    cJSONbase *getParent() { return this ? pParent : 0; }

    cJSONbase *searchOrGenerate(const char *pStr);

    cJSONbase *search(const char *pStr) {
        cJSONbase *pRet = this;
        switch (pStr[0]) {
        case 0:
            return this;

        case '/':
            while (pRet->pParent) {
                pRet = pRet->pParent;
            }
            return pRet->search(pStr+1);

        case '.':
            switch (pStr[1]) {
            case '.':
                pRet = pRet->pParent;
                if (pStr[2] == '/') {
                    return pRet ? pRet->search(pStr+3) : nullptr;
                } else if (pStr[2] == 0) {
                    return pRet;
                }
                return nullptr;
            case '/':
                return pRet->search(pStr+2);
            case 0:
                return pRet;
            default:
                return nullptr;
            }
        case '[':
            return searchArray(pStr);

        default:
            return searchObject(pStr);
        }
    }

    virtual char *toStr(int &rLen, char *pBuffer, int deep = 0) = 0;
    virtual const char *fromStr(const char *pBuffer) = 0;

    virtual cJSONbase* removeObject(cJSONbase *pP) { return 0; } // do nothing
    static cJSONbase *genStr(cJSONbase *pP, const char *pS) {
        return generate(pP, pS);
    }
    static cJSONbase *generate(cJSONbase *pP, const char *&rpStr);
    static cJSONbase *generate(cJSONbase *pP, char *&rpStr) {
        const char *pConstStr = rpStr;
        cJSONbase *pRet = generate(pP, pConstStr);
        rpStr = const_cast<char *>(pConstStr);
        return pRet;
    }

    static cJSONbase *generate(cJSONbase *pP, FILE *);

    static char *skipWS(char *pP) {
        while (*pP && *pP <= ' ') {
            pP++;
        }
        return pP;
    }

    static const char *skipWS(const char *pP) {
        while (*pP && *pP <= ' ') {
            pP++;
        }
        return pP;
    }

    static bool isObjectEnd(char c) {
        return c == ',' || c == '}' || c == ']';
    }
};

class cJSONnone : public cJSONbase {

    public:
    cJSONnone(cJSONbase *pP) : cJSONbase(eJT_NONE, pP) { }
    ~cJSONnone() { }

    char *toStr(int &rLen, char *pBuffer, int deep = 0) {
        if (4 <= rLen ) {
            *pBuffer++ = 'n';
            *pBuffer++ = 'i';
            *pBuffer++ = 'l';
            rLen += 3;
            return pBuffer;
        }
        return 0;
    }
    const char *fromStr(const char *pBuffer) {
        if (pBuffer[0] == 'n' && pBuffer[1] == 'i' && pBuffer[2] == 'l') {
            pBuffer = skipWS(pBuffer + 3);
            if (isCommaOrClose(*pBuffer)) {
                return pBuffer;
            }
        }
        return 0;
    }
};

class cJSONbool : public cJSONbase {
    bool value;

    public:
    cJSONbool(cJSONbase *pP, bool v = false) : cJSONbase(eJT_BOOL, pP) { value = v; }
    ~cJSONbool() { }
    bool getValue() { return value; }
    void setValue(bool v) { value = v; }
    char *toStr(int &rLen, char *pBuffer, int deep);
    const char *fromStr(const char *pBuffer);
};

class cJSONscalar : public cJSONbase {
    uint64_t value;
    const char *pFmt;

    public:
    cJSONscalar(cJSONbase *pP, uint64_t v = 0, const char *pF = "0x%llX") : cJSONbase(eJT_SCALAR, pP) {
        value = v;
        pFmt = pF;
    }
    ~cJSONscalar() { }
    uint64_t getValue() { return value; }
    void setValue(uint64_t v) { value = v; }

    char *toStr(int &rLen, char *pBuffer, int deep = 0);
    const char *fromStr(const char *pBuffer);
};

class cJSONfloat : public cJSONbase {
    double value;
    const char *pFmt;
    
    public:
    cJSONfloat(cJSONbase *pP, double v = 0, const char *pF = "%f") : cJSONbase(eJT_FLOAT, pP) {
        value = v;
        pFmt = pF;
    }
    ~cJSONfloat() { }
    double getValue() { return value; }
    void setValue(double v) { value = v; }

    char *toStr(int &rLen, char *pBuffer, int deep = 0);
    const char *fromStr(const char *pBuffer);
};

class cJSONstring : public cJSONbase {
    bool withTripple;
    size_t length;
    char *value;
    void resizeBuffer(size_t l, const char *v) {
        size_t align_len = (l + 0x100) & ~0xff;
        if (length != align_len) {
            if (value) {
                delete value;
            }
            length = align_len;
            value = new char[align_len];
        }

        strncpy(value, v, l);
        value[l] = 0;
    }

    public:
    cJSONstring(cJSONbase *pP, const char *v = 0) : cJSONbase(eJT_STRING, pP) {
        withTripple = false;
        length = 0;
        value = 0;
        if (v) {
            resizeBuffer(strlen(v), v);
        }
    }
    ~cJSONstring() {
        if (value) {
            delete[] value;
            value = 0;
        }
    }

    const char *getValue() { return value; }
    void setValue(char *v) { resizeBuffer(strlen(v), v); }

    char *toStr(int &rLen, char *pBuffer, int deep = 0);
    const char *fromStr(const char *pBuffer);
};

class cJSONarray : public cJSONbase {
    std::vector<cJSONbase *> value;

    public:
    cJSONarray(cJSONbase *pP) : cJSONbase(eJT_ARRAY, pP) { }
    ~cJSONarray() {
        while (value.size()) {
            cJSONbase* pB = value[0];
            if (pB != nullptr) {
                delete pB;
            } else {
                value.erase(value.begin());
            }
        }
        for (size_t idx = 0; idx < value.size(); idx++) {
            delete value[idx];
        }
        value.clear();
    }

    int getSize() const { return static_cast<int>(value.size()); }

    std::vector<cJSONbase*>::iterator begin() {
        return value.begin();
    }

    std::vector<cJSONbase*>::iterator end() {
        return value.end();
    }
    cJSONbase *getValue(int idx) {
        return 0 <= idx && static_cast<size_t>(idx) < value.size() ? value[idx] : 0;
    }

    void addValue(cJSONbase *v) {
        value.push_back(v);
    }

    void setValue(int idx, cJSONbase *pV, bool createAsNeeded = false) {
        if (createAsNeeded) {
            while (static_cast<size_t>(idx) <= value.size()) {
                value.push_back(new cJSONnone(this));
            }
        }

        if (pV != nullptr && false == pV->setParent(this)) {
            return;
        }

        if (0 <= idx && static_cast<size_t>(idx) < value.size()) {
            value[idx] = pV;
        }
    }

    cJSONbase *removeObject(int idx) {
        if (0 <= idx && static_cast<size_t>(idx) < value.size()) {
            cJSONbase *pRet = value[idx];
            value.erase(value.begin() + idx);
            return pRet;
        }
        return 0;
    }
    cJSONbase* removeObject(cJSONbase* pO) {
        for (size_t idx = 0; idx < value.size(); idx++) {
            if (value[idx] == pO) {
                return removeObject(static_cast<int>(idx));
            }
        }
        return 0;
    }

    char *toStr(int &rLen, char *pBuffer, int deep = 0);
    const char *fromStr(const char *pBuffer);
};

class cJSONobject : public cJSONbase {
    class cJSONobj {
        std::string name;
        cJSONbase *value;

        public:
        cJSONobj(std::string s, cJSONbase *v) {
            name = s;
            value = v;
        }
        ~cJSONobj() {
            if (value) {
                delete value;
            }
            value = 0;
        }

        const char *getName() { return name.c_str(); }
        cJSONbase *getValue() { return value; }
        void setValue(cJSONbase *v) { value = v; }
        bool checkName(const char *s) { return 0 == strcmp(name.c_str(), s); }
        bool checkName(std::string s) { return name == s; }
    };

    std::vector<cJSONobj *>value;
    int getIndex(const char *s, bool createAsNeeded) {
        for (size_t idx = 0; idx < value.size(); idx++) {
            if (value[idx]->checkName(s)) {
                return static_cast<int>(idx);
            }
        }
        if (createAsNeeded) {
            cJSONobj *pO = new cJSONobj(s, nullptr);
            value.push_back(pO);
            return static_cast<int>(value.size()) - 1;
        }
        return -1;
    }

    int getIndex(std::string &s, bool createAsNeeded) {
        for (size_t idx = 0; idx < value.size(); idx++) {
            if (value[idx]->checkName(s)) {
                return static_cast<int>(idx);
            }
        }
        if (createAsNeeded) {
            cJSONobj *pO = new cJSONobj(s, nullptr);
            value.push_back(pO);
            return static_cast<int>(value.size()) - 1;
        }
        return -1;
    }

    bool
    getString(std::string &ret, const char *&prB) {
        const char *pP = skipWS(prB);
        if (*pP == '"' || *pP == '\'') {
            char termChar = *pP++;
            while (*pP != 0 && *pP != termChar) {
                ret += *pP++;
            }
            if (*pP == 0) {
                ret.clear();
                return false;
            } else {
                pP = skipWS(pP + 1);
                prB = pP;
                return true;
            }
        }

        ret.clear();
        return false;
    }

    cJSONobj * getObj(int idx) {
        return this != nullptr && 0 <= idx && static_cast<size_t>(idx) < value.size() ? value[idx] : 0;
    }

    cJSONobj *getObj(const char *s) {
        return getObj(getIndex(s, false));
    }

    public:
    explicit cJSONobject(cJSONbase *pP) : cJSONbase(eJT_OBJECT, pP) { }
    ~cJSONobject() {
        while (value.size()) {
            cJSONbase *pB = value[0]->getValue();
            if (pB != nullptr) {
                delete pB;
            }
            else {
                value.erase(value.begin());
            }
        }
    }

    int getSize() const {
        return static_cast<int>(value.size());
    }

    const char *getName(int idx) {
        return 0 <= idx && static_cast<size_t>(idx) < value.size() ? value[idx]->getName() : 0;
    }

    cJSONbase *getValue(int idx) {
        return 0 <= idx && static_cast<size_t>(idx) < value.size() ? value[idx]->getValue() : 0;
    }

    cJSONbase *getValue(const char *s) {
        cJSONobj *pO = getObj(getIndex(s, false));
        return pO != nullptr ? pO->getValue() : nullptr;
    }

    cJSONbase *getValue(std::string &s) {
        cJSONobj *pO = getObj(getIndex(s, false));
        return pO != nullptr ? pO->getValue() : nullptr;
    }

    void setValue(const char *pS, cJSONbase *pV, bool createAsNeeded = false) {
        cJSONobj *pO = getObj(getIndex(pS, createAsNeeded));
        if (pV != nullptr) {
            if (false == pV->setParent(this)) {
                return;
            }
        }
        if (createAsNeeded && pO == nullptr) {
            pO = new cJSONobj(pS, pV);
            value.push_back(pO);
        } else if (pO != nullptr) {
            pO->setValue(pV);
        }
    }

    void setValue(std::string &s, cJSONbase *pV, bool createAsNeeded = false) {
        cJSONobj *pO = getObj(getIndex(s, createAsNeeded));
        if (pV != nullptr) {
            if (false == pV->setParent(this)) {
                return;
            }
        }
        if (createAsNeeded && pO == nullptr) {
            pO = new cJSONobj(s, pV);
            value.push_back(pO);
        } else if (pO != nullptr) {
            pO->setValue(pV);
        }
    }

    cJSONbase *removeObject(int idx) {
        if (0 <= idx && (static_cast<size_t>(idx)) < value.size()) {
            cJSONobj  *pO   = value[idx];
            cJSONbase *pRet = pO->getValue();
            if (pRet != nullptr) {
                pRet->setParent(0);
            }
            value.erase(value.begin() + idx);
            pO->setValue(0);
            delete pO;
            return pRet;
        }
        return 0;
    }

    cJSONbase *removeObject(cJSONbase *pObj) {
        for (size_t idx = 0; idx < value.size(); idx++) {
            cJSONobj  *pO   = value[idx];
            if (pO->getValue() == pObj) {
                return removeObject(static_cast<int>(idx));
            }
        }
        return 0;
    }

    cJSONbase *removeObject(const char *pName) {
        for (size_t idx = 0; idx < value.size(); idx++) {
            cJSONobj  *pO   = value[idx];
            if (strcmp(pO->getName(), pName) == 0) {
                return removeObject(static_cast<int>(idx));
            }
        }
        return 0;
    }

    char *toStr(int &rLen, char *pBuffer, int deep = 0);
    const char *fromStr(const char *pBuffer);
};

#endif  // SRC_CJSON_H_
