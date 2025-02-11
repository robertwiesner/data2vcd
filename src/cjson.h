

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

    public:
    cJSONbase(eJSONtype t, cJSONbase *pP) {
        pParent = pP;
        type = t;
    }

    virtual ~cJSONbase() {
    }

    bool isEnd(char c) { return c <= ' '; }

    eJSONtype getType() const { return this ? type : eJT_NONE; }

    cJSONbase *getParent() { return this ? pParent : 0; }

    virtual char *toStr(int &rLen, char *pBuffer) = 0;
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

    char *toStr(int &rLen, char *pBuffer) {
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
    char *toStr(int &rLen, char *pBuffer);
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

    char *toStr(int &rLen, char *pBuffer);
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

    char *toStr(int &rLen, char *pBuffer);
    char *fromStr(char *pBuffer);
};

class cJSONstring : public cJSONbase {
    bool withTripple;
    size_t length;
    char *value;
    void resizeBuffer(size_t l, const char *v) {
        int align_len = (l + 0x100) & 0xff;
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

    char *toStr(int &rLen, char *pBuffer);
    char *fromStr(char *pBuffer);
};

class cJSONarray : public cJSONbase {
    std::vector<cJSONbase *> value;
    public:
    cJSONarray(cJSONbase *pP) : cJSONbase(eJT_ARRAY, pP) { }
    ~cJSONarray() { }
    cJSONbase *getValue(int idx) {
        return 0 <= idx && idx < value.size() ? value[idx] : 0;
    }
    void addValue(cJSONbase *v) {
        value.push_back(v);
    }
    void setValue(int idx, cJSONbase *v) {
        if (0 <= idx && idx < value.size()) {
            value[idx] = v;
        }
    }

    char *toStr(int &rLen, char *pBuffer);
    char *fromStr(char *pBuffer);
};

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

class cJSONobject : public cJSONbase {
    cJSONbase *pParent;
    std::vector<cJSONobj *>value;
    int getIndex(const char *s) {
        for (int idx = 0; idx < value.size(); idx++) {
            if (value[idx]->checkName(s)) {
                return idx;
            }
        }
        return -1;
    }
    int getIndex(std::string s) {
        for (int idx = 0; idx < value.size(); idx++) {
            if (value[idx]->checkName(s)) {
                return idx;
            }
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
            } else {
                pP = skipWS(pP + 1);
                prB = pP;
                return true;
            }
        }
        return false;
    }

    public:
    cJSONobject(cJSONbase *pP) : cJSONbase(eJT_OBJECT, pP) { }
    ~cJSONobject() { }

    cJSONobj * getObj(int idx) {
        return this != 0 && 0 <= idx && idx < value.size() ? value[idx] : 0;
    }

    cJSONobj *getObj(const char *s) {
        return getObj(getIndex(s));
    }

    cJSONbase *getValue(const char *s) {
        return getObj(getIndex(s))->getValue();
    }

    void setValue(const char *s, cJSONbase *v) {
        getObj(getIndex(s))->setValue(v);
    }

    void setValue(std::string s, cJSONbase *v) {
        getObj(getIndex(s))->setValue(v);
    }

    char *toStr(int &rLen, char *pBuffer);
    char *fromStr(char *pBuffer);
};

#endif
