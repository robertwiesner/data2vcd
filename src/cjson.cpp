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

cJSONbase *
cJSONbase::generate(cJSONbase *pP, FILE *pIn)
{
    size_t size;
    fseek(pIn, 0, SEEK_END);
    size = ftell(pIn);
    fseek(pIn, 0, SEEK_SET);

    char *pBuffer   = new char[size + 1];
    char *pStart    = pBuffer;

    fread(pBuffer, size, 1, pIn);
    pBuffer[size] = 0;
    cJSONbase *pRet = cJSONbase::generate(pP, pStart);

    delete[] pBuffer;

    return pRet;
}

cJSONbase *
cJSONbase::generate(cJSONbase *pP, char *&prStart)
{
    cJSONbase *pRet = NULL;

    char *pS = skipWS(prStart);
    char *pT = pS;

    switch (*pS) {
    case '"': // string
        pRet = new cJSONstring(pP, 0);
        break;
    case '[': // array
        pRet = new cJSONarray(pP);
        break;
    case '{': // object
        pRet = new cJSONobject(pP);
        break;

    case '-':
    case '+':
        pT++;
    case '0': // scalar or float
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
        char *pEnd;
        strtoull(pT, &pEnd, 0);
        if (pEnd) {
            pEnd = skipWS(pEnd);
            if (*pEnd == ',' || *pEnd == '}' || *pEnd == ']') {
                pRet = new cJSONscalar(pP);
            }
        }
        if (pRet == 0) {
            strtod(pS, &pEnd);
            if (pEnd) {
                pEnd = skipWS(pEnd);
                if (*pEnd == ',' || *pEnd == '}' || *pEnd == ']') {
                    pRet = new cJSONfloat(pP);
                }
            }
        }
    }
        break;
    case 't':
    case 'f': // true or false
        if (0 == strncmp(pS, "true", 4) || 0 == strncmp(pS, "false", 5)) {
            pRet = new cJSONbool(pP);
        }
        break;
    case 'n':
        if (0 == strncmp(pS, "nil", 3)) {
            pRet = new cJSONnone(pP);
        }
    default:
        break;
    }

    if (pRet) {
        pS = pRet->fromStr(pS);
        if (pS == 0) {
            delete pRet;
            pRet = 0;
        } else {
            pS = skipWS(pS);
            if (*pS != ',' && *pS != ']' && *pS != '}' && *pS != 0) {
                delete pRet;
                pRet = 0;
            }
        }
        prStart = pS;
    }
    return pRet;
}



char *cJSONbool::toStr(int &rLen, char *pBuffer)
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
char *cJSONbool::fromStr(char *pBuffer)
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

char *cJSONscalar::toStr(int &rLen, char *pBuffer)
{
    char aBuffer[64];
    sprintf(aBuffer, pFmt, value);
    int len = strlen(aBuffer);

    if (len < rLen) {
        strcpy(pBuffer, aBuffer);
        pBuffer += len;
    }
    rLen -= len;

    return pBuffer;
}
char *cJSONscalar::fromStr(char *pBuffer)
{
    bool neg = false;
    if (*pBuffer == '-') {
        pBuffer++;
        neg = true;
    } else if (*pBuffer == '+') {
        pBuffer++;
    }
    value = strtoull(pBuffer, &pBuffer, 0);
    if (neg) {
        value = -value;
    }
    return pBuffer;
}

char *cJSONfloat::toStr(int &rLen, char *pBuffer)
{
    char aBuffer[64];
    sprintf(aBuffer, pFmt, value);
    int len = strlen(aBuffer);

    if (len < rLen) {
        strcpy(pBuffer, aBuffer);
        pBuffer += len;
    }
    rLen -= len;

    return pBuffer;
}
char *cJSONfloat::fromStr(char *pBuffer)
{

    value = strtod(pBuffer, &pBuffer);
    return pBuffer;
}

char *cJSONstring::toStr(int &rLen, char *pBuffer)
{
    int len = strlen(value) + (withTripple ? 6 : 2);

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

char *cJSONstring::fromStr(char *pBuffer)
{
    withTripple = pBuffer[0] == '"' && pBuffer[1] == '"' && pBuffer[2] == '"';
    char *pB = pBuffer + (withTripple ? 3 : 1);

    if (withTripple) {
        while (*pB && pB[0] != '"' && pB[1] != '"' && pB[2] != '"') {
            pB++;
        }

        if (*pB) {
            resizeBuffer(pB - pBuffer - 1, pBuffer + 1);
            pB += 3;
        }
    } else {
        while (*pB && *pB != '"') {
            pB++;
        }

        if (*pB) {
            resizeBuffer(pB - pBuffer - 1, pBuffer + 1);
            pB += 1;
        }
    }
    return pB;
}

char *cJSONarray::toStr(int &rLen, char *pBuffer)
{
    char sep = '[';

    for (size_t idx = 0; idx < value.size(); idx ++) {
        if (1 < rLen) { *pBuffer++ = sep; }
        sep = ',';
        rLen -= 1;
        pBuffer = value[idx]->toStr(rLen, pBuffer);
    }

    if (2 < rLen) {
        *pBuffer++ = ']'; 
        *pBuffer = 0;
    } else { pBuffer = 0; }

    rLen -= 1;

    return pBuffer;
}
char *cJSONarray::fromStr(char *pBuffer)
{
    char sep = '[';

    while (*pBuffer == sep) {
        pBuffer += 1;
        cJSONbase *pObj = generate(this, pBuffer);
        if (pObj == NULL) return NULL;
        value.push_back(pObj);
        sep = ',';
    }
    return pBuffer;
}


char *cJSONobject::toStr(int &rLen, char *pBuffer)
{
    char sep = '{';

    for (size_t idx = 0; idx < value.size(); idx ++) {
        cJSONobj *pO = value[idx];
        int len = strlen(pO->getName());
        if (1 < rLen) { *pBuffer++ = sep; }

        sep = ',';
        rLen -= 1 + 3 + len;

        if (3 + len < rLen) {
            *pBuffer++ = '"';
            strcpy(pBuffer, pO->getName());
            pBuffer += len;
            *pBuffer++ = '"';
            *pBuffer++ = ':';
        }
        pBuffer = pO->getValue()->toStr(rLen, pBuffer);
    }
    if (2 < rLen) {
        *pBuffer++ = '}';
        *pBuffer = 0;
    }
    rLen -= 1;

    return pBuffer;
}
char *cJSONobject::fromStr(char *pBuffer)
{
    char sep = '{';

    while (*pBuffer == sep) {
        pBuffer += 1;
        std::string name;
        
        if (false == getString(name, pBuffer)) {
            return 0;
        }

        if (*pBuffer == ':') {
            pBuffer ++;
            cJSONbase *pObj = generate(this, pBuffer);
            if (pObj != NULL) {
                int idx = getIndex(name);
                if (idx < 0) {
                    cJSONobj *pO = new cJSONobj(name, pObj);
                    value.push_back(pO);
                } else {
                    value[idx]->setValue(pObj);
                }
            } else {
                return NULL;
            }
        } else {
            return NULL;
        }
        sep = ',';
    }

    if (*pBuffer != '}') {
        return 0;
    }

    return pBuffer + 1;
}

