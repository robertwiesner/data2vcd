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

#ifndef SRC_CWIRE_H_
#define SRC_CWIRE_H_ 1

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include "./ctrackmem.h"


// specify the wire data type
enum eWireType {
    eWT_NONE,
    eWT_BOOL,  // a single true/false bit
    eWT_BIT,   // bit number
    eWT_OCT,   // octal number
    eWT_DEC,   // signed decimal number
    eWT_HEX,   // unsigned hex number
    eWT_HEXs,  // signed hex number (highest bit is the sign bit)
    eWT_STR,   // ascii string
};

class cWire TRACKMEM_BASE_COL {
    size_t index;           // user data
    size_t bits;            // number of valid bits for this value
    eWireType wireType;
    std::vector<unsigned char>byteBuffer;
    std::string name;       // regular name
    std::string shortName;  // short name as needed by the output
    bool valueChanged;      // track if a new value has been updated - meaning this needs to be dumped into the output file
    bool valueInvalid;      // true if value is invalid (X)
    int checkValid0;        // if != -1, use this bit == 0 to check if this wire is valid
    int checkValid1;        // if != -1, use this bit == 1 to check if this wire is valid

  public:
    cWire(eWireType wt, size_t b, const char *pName) TRACKMEM_CONS_COL {
        wireType = wt;
        bits = b;
        name = pName;
        shortName = "";
        byteBuffer.resize((b + 7) / 8);
        valueChanged = true;
        valueInvalid = true;
        index = 0;
        checkValid0 = -1;
        checkValid1 = -1;
    }

    void updateValid0(uint64_t v0) { checkValid0 = static_cast<int32_t>(v0); }
    void updateValid1(uint64_t v1) { checkValid1 = static_cast<int32_t>(v1); }

    // return true if the associate signal value is valid.
    bool isValid(const char* pData) {
        bool ret = checkValid0 < 0 && checkValid1 < 0;

        if (!ret && 0 <= checkValid0) {
            ret |= (0 == (pData[checkValid0 / 8] & (1 << (checkValid0 & 7))));
        }

        if (!ret && 0 <= checkValid1) {
            ret |= (0 != (pData[checkValid1 / 8] & (1 << (checkValid1 & 7))));
        }
        return ret;
    }

    // user data for additional information
    void setIndex(size_t idx) { index = idx; }
    size_t getIndex() { return index; }

    void setShortName(const char *pSN) { shortName = pSN; }

    // return and clean if value has changed since the last checked
    bool hasChanged() { bool ret = valueChanged; valueChanged = false; return ret; }

    // access functions
    size_t      getBits()      { return bits;         }
    eWireType   getWireType()  { return wireType;     }
    const char *getName()      { return name.c_str(); }
    const char *getShortName() { return shortName.c_str(); }

    const unsigned char *getBuffer() { return byteBuffer.data(); }
    bool isInvalid() const { return valueInvalid; }
    bool isValid()   const { return false == valueInvalid; }

    void setInvalid() {
        valueChanged = false == valueInvalid;
        valueInvalid = true;
    }

    // set functions
    void setValue(size_t bytes, unsigned char *pV) {
        size_t localSize = (bits+7)/8;

        bytes = bytes < localSize ? bytes : localSize;

        if (valueInvalid || memcmp(byteBuffer.data(), pV, bytes)) {
            valueInvalid = false;
            valueChanged = true;
            memcpy(byteBuffer.data(), pV, bytes);
        }
    }

    void setValue(size_t bytes, char *pV) { setValue(bytes, (unsigned char *)pV); }
    void setValue(unsigned int val)       { setValue(sizeof(val), (unsigned char *) &val); }
    void setValue(uint64_t val) { setValue(sizeof(val), (unsigned char *) &val); }
    void setValue(int val)                { setValue(sizeof(val), (unsigned char *) &val); }
    void setValue(int64_t val)          { setValue(sizeof(val), (unsigned char *) &val); }


    // helper function to access the buffer values as signed and unsinged int/lonlong values
    unsigned int getAsUnsignedInt() {
        unsigned int ret = 0;
        size_t bitShift = 0;
        for (auto &it : byteBuffer) {
            ret |= ((unsigned int) it) << bitShift;
            bitShift += 8;
            if (bitShift == 32 || bits < bitShift) {
                break;
            }
        }
        return ret;
    }

    int getAsSignedInt() {
        int ret = static_cast<int32_t>(getAsUnsignedInt());
        if (bits < 31) {
            ret <<= 31 - bits;
            ret >>= 31 - bits;
        }
        return ret;
    }

    uint64_t getAsUnsignedLongLong() {
        uint64_t ret = 0;
        size_t bitShift = 0;
        for (auto& it : byteBuffer) {
            ret |= static_cast<uint64_t>(it) << bitShift;
            bitShift += 8;
            if (bitShift == 64 || bits < bitShift) {
                break;
            }
        }
        return ret;
    }
    int64_t getAsSignedLongLong() {
        int64_t ret = static_cast<int64_t>(getAsUnsignedLongLong());
        if (bits < 63) {
            ret <<= 63 - bits;
            ret >>= 63 - bits;
        }
        return ret;
    }
};

#endif  // SRC_CWIRE_H_

