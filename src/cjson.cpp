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

#include "cjson.h"
#include "ctmpbuf.h"

cJSONbase *
cJSONbase::searchArray(const char *pStr)
{
    cJSONarray *pArr = dynamic_cast<cJSONarray *>(this);

    if (pArr != nullptr) {
        char *pEnd = 0;
        int idx = strtol(pStr, &pEnd, 0);

        if (pEnd && *pEnd == ']') {
            cJSONbase *pRet = pArr->getValue(idx);

            if (pRet != nullptr) {
                switch (pEnd[1]) {
                case '/': return pRet->search(pEnd + 2);
                case 0  : return pRet;
                default: return 0;
                }
            }
        }
    }
    return nullptr;
}

cJSONbase *
cJSONbase::searchObject(const char *pStr)
{
    cJSONobject *pObj = dynamic_cast<cJSONobject *>(this);

    if (pObj != nullptr) {
        char aBuffer[1024];

        pStr = getName(pStr, aBuffer, aBuffer + sizeof(aBuffer));

        cJSONbase *pRet = pObj->getValue(aBuffer);
        if (pRet != nullptr) {
            switch (*pStr) {
            case '/': return pRet->search(pStr + 1);
            case '[': return pRet->search(pStr);
            case 0: return pRet;
            default: return 0;
            }
        }
    }
    return 0;
}

// Return the last object and create path as needed
// Pathe containing .. or . are not supported
// if the VAR is a nullptr pointer, the first created objects is stored there.
cJSONbase *
cJSONbase::searchOrGenerate(const char *pStr)
{
    cJSONbase *pRet = nullptr;

    if (pStr[0] == '/') {
        pRet = getParent();
        if (pRet != nullptr) {
            return pRet->searchOrGenerate(pStr);
        }
        // we are at the top level now
        pStr++;
    }

    pRet = this;
    while (*pStr) {
        if (pStr[0] == '[') {
            char *pEnd = 0;
            cJSONarray *pArr = dynamic_cast<cJSONarray*>(pRet);
            int idx = strtol(pStr+1, &pEnd, 0);
            pEnd = skipWS(pEnd);

            if (pArr == nullptr) {
                return 0;
            }

            if (*pEnd == ']') {
                for (int i = pArr->getSize(); i <= idx; i++) {
                    pArr->addValue(new cJSONnone(pArr));
                }
                pEnd = skipWS(pEnd+1);
                switch (*pEnd) {
                case '[': {  // Followed by another array
                    cJSONarray *pNext = dynamic_cast<cJSONarray *>(pArr->getValue(idx));
                    if (pNext == nullptr) {
                        pNext = new cJSONarray(pArr);
                        pArr->setValue(idx, pNext);
                    }
                    pStr = pEnd;
                    pRet = pNext;
                } break;
                case '/': {
                    cJSONobject *pNext = dynamic_cast<cJSONobject *>(pArr->getValue(idx));
                    if (pNext == nullptr) {
                        pNext = new cJSONobject(pArr);
                        pArr->setValue(idx, pNext);
                    }
                    pRet = pNext;
                    pStr = pEnd + 1;
                    break;
                }

                case 0:
                    return pArr;
                }
            }
        } else if (pStr[0]) {
            // this must be an object
            cJSONobject *pObj = dynamic_cast<cJSONobject*>(pRet);
            char aBuffer[1024];

            pStr = getName(pStr, aBuffer, aBuffer + sizeof(aBuffer)+1);

            if (pObj == nullptr) {
                return nullptr;
            }

            cJSONbase *pFound = pObj->getValue(aBuffer);

            if (pFound == nullptr) {
                switch (*pStr) {
                case 0:
                    pObj->setValue(aBuffer, new cJSONnone(pObj), true);
                    pRet = pObj;
                    break;
                case '[':
                    pRet = new cJSONarray(pObj);
                    pObj->setValue(aBuffer, pRet, true);
                    break;
                case '/':
                    pRet = new cJSONobject(pObj);
                    pObj->setValue(aBuffer, pRet, true);
                    pStr++;
                    break;
                }
            } else {
                switch (*pStr) {
                case 0:
                    pRet = pFound;
                    break;
                case '[':
                    pRet = dynamic_cast<cJSONarray*>(pFound);
                    if (pRet == nullptr) {
                        return nullptr;
                    }
                    break;
                case '/':
                    pRet = dynamic_cast<cJSONobject*>(pFound);
                    if (pRet == nullptr) {
                        return nullptr;
                    }
                    pStr++;
                    break;
                }
            }
        }
    }
    return pRet;
}

cJSONbase *
cJSONbase::generate(cJSONbase *pP, FILE *pIn)
{
    size_t size;
    size_t readSize;
    fseek(pIn, 0, SEEK_END);
    size = ftell(pIn);
    fseek(pIn, 0, SEEK_SET);

    cTmpBuf buffer(size + 1);

    readSize = fread(buffer.getBuffer(), 1, size, pIn);
    buffer.getBuffer()[readSize] = 0;
    const char* pStart = buffer.getBuffer();
    cJSONbase *pRet = cJSONbase::generate(pP, pStart);

    return pRet;
}

cJSONbase *
cJSONbase::generate(cJSONbase *pP, const char *&prStart)
{
    cJSONbase *pRet = nullptr;

    const char *pS = skipWS(prStart);

    switch (*pS) {
    case '\'':
    case '"':  // string
        pRet = new cJSONstring(pP, 0);
        break;
    case '[':  // array
        pRet = new cJSONarray(pP);
        break;
    case '{':  // object
        pRet = new cJSONobject(pP);
        break;

    case '-':
    case '+':
    case '0':  // scalar or float
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        if (isScalar(pS)) {
            pRet = new cJSONscalar(pP);
        } else if (isFloat(pS)) {
            pRet = new cJSONfloat(pP);
        }
        break;

    case 't':
    case 'f':  // true or false
        if (0 == strncmp(pS, "true", 4) || 0 == strncmp(pS, "false", 5)) {
            pRet = new cJSONbool(pP);
        }
        break;

    case 'n':
        if (0 == strncmp(pS, "nil", 3) || 0 == strncmp(pS, "null", 3)) {
            pRet = new cJSONnone(pP);
        }
    default:
        break;
    }

    if (pRet != nullptr) {
        const char *pEnd = pRet->fromStr(pS);

        if (pEnd == nullptr) {
            delete pRet;
            pRet = 0;
        } else {
            pEnd = skipWS(pEnd);
            if (false == isObjectEnd(*pEnd) && *pEnd) {
                delete pRet;
                pRet = 0;
            }
        }
        prStart = pEnd;
    }
    return pRet;
}

char *
cJSONbool::toStr(int &rLen, char *pBuffer, int deep)
{
    if (value) {
        if (4 < rLen) {
            strcpy(pBuffer, "true");
            pBuffer += 4;
        }
        rLen -= 4;
    } else {
        if (5 < rLen) {
            strcpy(pBuffer, "false");
            pBuffer += 5;
        }
        rLen -= 5;
    }

    return pBuffer;
}

const char *
cJSONbool::fromStr(const char *pBuffer)
{
    if (0 == strncmp(pBuffer, "true", 4)) {
        value = true;
        pBuffer += 4;
    } else if (0 == strncmp(pBuffer, "false", 5)) {
        value = false;
        pBuffer += 5;
    }
    return pBuffer;
}

char *
cJSONscalar::toStr(int &rLen, char *pBuffer, int deep)
{
    char aBuffer[64];
    snprintf(aBuffer, sizeof(aBuffer) - 1, pFmt, value);
    int len = static_cast<int>(strlen(aBuffer));

    if (len < rLen) {
        strcpy(pBuffer, aBuffer);
        pBuffer += len;
    }
    rLen -= len;

    return pBuffer;
}

const char *
cJSONscalar::fromStr(const char *pBuffer)
{
    bool neg = false;
    if (*pBuffer == '-') {
        pBuffer++;
        neg = true;
    } else if (*pBuffer == '+') {
        pBuffer++;
    }
    char *pEnd = nullptr;
    value = strtoull(pBuffer, &pEnd, 0);
    pBuffer = pEnd;
    if (neg) {
        value = 1 + (~value);
    }
    return pEnd;
}

char *
cJSONfloat::toStr(int &rLen, char *pBuffer, int deep)
{
    char aBuffer[64];
    snprintf(aBuffer, sizeof(aBuffer) - 1, pFmt, value);
    int len = static_cast<int>(strlen(aBuffer));

    if (len < rLen) {
        strcpy(pBuffer, aBuffer);
        pBuffer += len;
    }
    rLen -= len;

    return pBuffer;
}

const char *
cJSONfloat::fromStr(const char *pBuffer)
{
    char *pEnd = nullptr;
    value = strtod(pBuffer, &pEnd);
    return pEnd;
}

char *
cJSONstring::toStr(int &rLen, char *pBuffer, int deep)
{
    int len = static_cast<int>(strlen(value) + (withTripple ? 6 : 2));

    if (len < rLen) {
        *pBuffer++ = '"';
        if (withTripple) {
            *pBuffer++ = '"';
            *pBuffer++ = '"';
        }
        strcpy(pBuffer, value);
        pBuffer += len - 2;
        *pBuffer++ = '"';
        if (withTripple) {
            *pBuffer++ = '"';
            *pBuffer++ = '"';
        }
        *pBuffer = 0;
    } else {
        pBuffer = 0;
    }
    rLen -= len;
    return pBuffer;
}

const char *
cJSONstring::fromStr(const char *pBuffer)
{
    const char termChar = *pBuffer;
    withTripple = pBuffer[0] == termChar && pBuffer[1] == termChar && pBuffer[2] == termChar;
    const char *pB = pBuffer + (withTripple ? 3 : 1);

    if (withTripple) {
        while (*pB && pB[0] != termChar && pB[1] != termChar && pB[2] != termChar) {
            pB++;
        }

        if (*pB) {
            resizeBuffer(pB - pBuffer - 1, pBuffer + 1);
            pB += 3;
        }
    } else {
        while (*pB && *pB != termChar) {
            pB++;
        }

        if (*pB) {
            resizeBuffer(pB - pBuffer - 1, pBuffer + 1);
            pB += 1;
        }
    }
    return pB;
}

char *
cJSONarray::toStr(int &rLen, char *pBuffer, int deep)
{
    deep++;

    pBuffer = addDeepStringFirst(rLen, pBuffer, deep, '[');

    for (size_t idx = 0; idx < value.size(); idx++) {
        if (idx) {
            pBuffer = addDeepStringFirst(rLen, pBuffer, deep, ',');
        }
        pBuffer = value[idx]->toStr(rLen, pBuffer, deep);
    }

    return addDeepStringLast(rLen, pBuffer, deep - 1, ']');
}

const char *
cJSONarray::fromStr(const char *pBuffer)
{
    char sep = '[';

    while (*pBuffer == sep) {
        pBuffer = skipWS(pBuffer + 1);
        if (*pBuffer == ',' && (sep == '[' || sep == ',')) {
            // this is a "[ ," or ", ," string 
            value.push_back(new cJSONnone(this));
        } else if (*pBuffer == ']') {
            // this is a "[ ]" or ", ]" string
            break;
        } else {
            cJSONbase *pObj = generate(this, pBuffer);
            if (pObj == nullptr) {
                return nullptr;
            }
            value.push_back(pObj);
        }
        pBuffer = skipWS(pBuffer);
        sep = ',';
    }
    return pBuffer + (*pBuffer ? 1 : 0);
}

char *
cJSONobject::toStr(int &rLen, char *pBuffer, int deep)
{
    deep++;
    pBuffer = addDeepStringFirst(rLen, pBuffer, deep, '{');

    for (size_t idx = 0; idx < value.size(); idx++) {
        cJSONobj *pO = value[idx];
        int len = static_cast<int>(strlen(pO->getName()));

        if (idx) {
            pBuffer = addDeepStringFirst(rLen, pBuffer, deep, ',');
        }

        rLen -= 5 + len;

        if (0 < rLen) {
            *pBuffer++ = '"';
            strcpy(pBuffer, pO->getName());
            pBuffer += len;
            *pBuffer++ = '"';
            *pBuffer++ = ' ';
            *pBuffer++ = ':';
            *pBuffer++ = ' ';
        }

        pBuffer = pO->getValue()->toStr(rLen, pBuffer, deep);
    }

    return addDeepStringLast(rLen, pBuffer, deep - 1, '}');
}

const char *
cJSONobject::fromStr(const char *pBuffer)
{
    char sep = '{';

    while (*pBuffer == sep) {
        pBuffer = skipWS(pBuffer + 1);

        // if we have additional command ignore them
        if (*pBuffer == '}') {
            // if "{ }" or ", }" is seen skipt it
            break;
        }
        
        if (*pBuffer != ',') {
            std::string name;
            if (false == getString(name, pBuffer)) {
                return 0;
            }

            if (*pBuffer == ':') {
                pBuffer++;
                cJSONbase *pObj = generate(this, pBuffer);
                if (pObj != nullptr) {
                    setValue(name, pObj, true);
                } else {
                    return nullptr;
                }
            } else {
                return nullptr;
            }
        }
        sep = ',';
        pBuffer = skipWS(pBuffer);
    }

    if (*pBuffer != '}') {
        return 0;
    }

    return pBuffer + 1;
}
