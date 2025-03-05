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

#ifndef CJSON_H
#define CJSON_H 1

#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>

enum eJSONtype {
    eJT_NONE,
    eJT_BOOL,
    eJT_SCALAR,
    eJT_FLOAT,
    eJT_STRING,
    eJT_ARRAY,
    eJT_OBJECT,
};

class cJSONbase {
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
    public:
    cJSONbase(eJSONtype t, cJSONbase *pP) {
        pParent = pP;
        type = t;
    }

    virtual ~cJSONbase() {
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
                    return pRet ? pRet->search(pStr+3) : NULL;
                } else if (pStr[2] == 0) {
                    return pRet;
                }
                return NULL;
            case '/':
                return pRet->search(pStr+2);
            case 0:
                return pRet;
            default:
                return NULL;
            }
        case '[':
            return searchArray(pStr);

        default:
            return searchObject(pStr);
        }
    }

    virtual char *toStr(int &rLen, char *pBuffer, int deep = 0) = 0;
    virtual char *fromStr(char *pBuffer) = 0;

    static cJSONbase *generate(cJSONbase *pP, char *&);
    static cJSONbase *generate(cJSONbase *pP, FILE *);

    static char *skipWS(char *pP) {
        while (*pP && *pP <= ' ') {
            pP++;
        }
        return pP;
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
    char *fromStr(char *pBuffer) {
        if (pBuffer[0] == 'n' && pBuffer[1] == 'i' && pBuffer[2] == 'l' && isEnd(pBuffer[3])) {
            return pBuffer + 3;
        } else {
            return 0;
        }
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
    char *fromStr(char *pBuffer);
};

class cJSONscalar : public cJSONbase {
    unsigned long long value;
    const char *pFmt;
    public:
    cJSONscalar(cJSONbase *pP, unsigned long long v = 0, const char *pF = "0x%llX") : cJSONbase(eJT_SCALAR, pP) {
        value = v;
        pFmt = pF;
    }
    ~cJSONscalar() { }
    unsigned long long getValue() { return value; }
    void setValue(unsigned long long v) { value = v; }

    char *toStr(int &rLen, char *pBuffer, int deep = 0);
    char *fromStr(char *pBuffer);
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
    char *fromStr(char *pBuffer);
};

class cJSONstring : public cJSONbase {
    bool withTripple;
    size_t length;
    char *value;
    void resizeBuffer(size_t l, const char *v) {
        size_t align_len = (l + 0x100) & 0xff;
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
            delete value;
        }
    }

    const char *getValue() { return value; }
    void setValue(char *v) { resizeBuffer(strlen(v), v); }

    char *toStr(int &rLen, char *pBuffer, int deep = 0);
    char *fromStr(char *pBuffer);
};

class cJSONarray : public cJSONbase {
    std::vector<cJSONbase *> value;
    public:
    cJSONarray(cJSONbase *pP) : cJSONbase(eJT_ARRAY, pP) { }
    ~cJSONarray() { }

    int getSize() const { return (int) value.size(); }

    cJSONbase *getValue(int idx) {
        return 0 <= idx && ((size_t) idx) < value.size() ? value[idx] : 0;
    }

    void addValue(cJSONbase *v) {
        value.push_back(v);
    }

    void setValue(int idx, cJSONbase *v, bool createAsNeeded = false) {
        if (createAsNeeded) {
            while (((size_t) idx) <= value.size()) {
                value.push_back(new cJSONnone(this));
            }
        }
        if (0 <= idx && ((size_t) idx) < value.size()) {
            value[idx] = v;
        }
    }

    char *toStr(int &rLen, char *pBuffer, int deep = 0);
    char *fromStr(char *pBuffer);
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
        }

        const char *getName() { return name.c_str(); }
        cJSONbase *getValue() { return value; }
        void setValue(cJSONbase *v) { if (this) { value = v; } }
        bool checkName(const char *s) { return 0 == strcmp(name.c_str(), s); }
        bool checkName(std::string s) { return name == s; }

    };

    std::vector<cJSONobj *>value;
    int getIndex(const char *s, bool createAsNeeded) {
        for (size_t idx = 0; idx < value.size(); idx++) {
            if (value[idx]->checkName(s)) {
                return (int) idx;
            }
        }
        if (createAsNeeded) {
            cJSONobj *pO = new cJSONobj(s, NULL);
            value.push_back(pO);
            return ((int) value.size()) - 1;
        }
        return -1;
    }

    int getIndex(std::string &s, bool createAsNeeded) {
        for (size_t idx = 0; idx < value.size(); idx++) {
            if (value[idx]->checkName(s)) {
                return (int) idx;
            }
        }
        if (createAsNeeded) {
            cJSONobj *pO = new cJSONobj(s, NULL);
            value.push_back(pO);
            return ((int) value.size()) - 1;
        }
        return -1;
    }

    bool
    getString(std::string &ret, char *&prB) {
        char *pP = skipWS(prB);
        if (*pP == '"') {
            pP++;
            while(*pP != 0 && *pP != '"') {
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
        return this != 0 && 0 <= idx && ((size_t)idx) < value.size() ? value[idx] : 0;
    }

    cJSONobj *getObj(const char *s) {
        return getObj(getIndex(s, false));
    }

    public:
    cJSONobject(cJSONbase *pP) : cJSONbase(eJT_OBJECT, pP) { }
    ~cJSONobject() { }

    int getSize() const { return (int) value.size(); }
    
    const char *getName(int idx) {
        return this != 0 && 0 <= idx && ((size_t)idx) < value.size() ? value[idx]->getName() : 0;
    }

    cJSONbase *getValue(int idx) {
        return this != 0 && 0 <= idx && ((size_t)idx) < value.size() ? value[idx]->getValue() : 0;
    }

    cJSONbase *getValue(const char *s) {
        cJSONobj *pO = getObj(getIndex(s, false));
        return pO ? pO->getValue() : NULL;
    }

    cJSONbase *getValue(std::string &s) {
        cJSONobj *pO = getObj(getIndex(s, false));
        return pO ? pO->getValue() : NULL;
    }

    void setValue(const char *s, cJSONbase *v, bool createAsNeeded = false) {
        cJSONobj *pO = getObj(getIndex(s, createAsNeeded));
        if (createAsNeeded && pO == NULL) {
            pO = new cJSONobj(s, v);
            value.push_back(pO);
        } else if (pO) {
            pO->setValue(v);
        }
    }

    void setValue(std::string &s, cJSONbase *v, bool createAsNeeded = false) {
        cJSONobj *pO = getObj(getIndex(s, createAsNeeded));
        if (createAsNeeded && pO == NULL) {
            pO = new cJSONobj(s, v);
            value.push_back(pO);
        } else if (pO) {
            pO->setValue(v);
        }
    }

    char *toStr(int &rLen, char *pBuffer, int deep = 0);
    char *fromStr(char *pBuffer);
};

#endif
