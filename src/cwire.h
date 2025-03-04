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

#ifndef CWIRE_H
#define CWIRE_H

#include <string>
#include <string.h>

// specify the wire data type
enum eWireType {
    eWT_NONE,
    eWT_BOOL, // a single true/false bit
    eWT_BIT,  // bit number
    eWT_OCT,  // octal number
    eWT_DEC,  // signed decimal number
    eWT_HEX,  // unsigned hex number
    eWT_HEXs, // signed hex number (highest bit is the sign bit)
    eWT_STR,  // ascii string
};

class cWire {
    size_t index;           // user data 
    size_t bits;            // number of valid bits for this value
    eWireType wireType; 
    unsigned char *pBuffer; // buffer to store the value
    std::string name;       // regular name
    std::string shortName;  // short name as needed by the output
    bool valueChanged;      // track if a new value has been updated - meaning this needs to be dumped into the output file
    bool valueInvalid;      // true if value is invalid (X)
    public:
    cWire(eWireType wt, size_t b, const char *pName) {
        wireType = wt;
        bits = b;
        name = pName;
        shortName = "";
        pBuffer = new unsigned char[(b + 7) / 8];
        valueChanged = true;
        valueInvalid = true;
        index = 0;
    }

    // user data for additional information
    void setIndex(size_t idx) { index = idx; }
    size_t getIndex() { return index; }

    void setShortName(const char *pSN) {
        shortName = pSN;
    }

    // return and clean if value has changed since the last checked
    bool hasChanged() { bool ret = valueChanged; valueChanged = false; return ret; }

    // access functions
    size_t      getBits()      { return bits;         }
    eWireType   getWireType()  { return wireType;     }
    const char *getName()      { return name.c_str(); }
    const char *getShortName() { return shortName.c_str(); }
    const unsigned char *getBuffer() { return pBuffer; }
    
    bool isInvalid() const { return valueInvalid; }
    bool isValid()   const { return ! valueInvalid; }

    void setInvalid() {
        valueChanged = false == valueInvalid;
        valueInvalid = true;
    }

    // set functions
    void setValue(size_t bytes, unsigned char *pV) {
        size_t localSize = (bits+7)/8;

        bytes = bytes < localSize ? bytes : localSize;

        if (valueInvalid || memcmp(pBuffer, pV, bytes)) {
            valueInvalid = false;
            valueChanged = true;
            memcpy(pBuffer, pV, bytes);
        }
    }

    void setValue(size_t bytes, char *pV) {
        setValue(bytes, (unsigned char *)pV);
    }

    void setValue(unsigned int val) {
        setValue(sizeof(val), (unsigned char *) &val);
    }

    void setValue(unsigned long long val) {
        setValue(sizeof(val), (unsigned char *) &val);
    }

    void setValue(int val) {
        setValue(sizeof(val), (unsigned char *) &val);
    }
    
    void setValue(long long val) {
        setValue(sizeof(val), (unsigned char *) &val);
    }
    

    // helper function to access the buffer values as signed and unsinged int/lonlong values
    unsigned int getAsUnsignedInt() {
        unsigned int ret = 0;
        unsigned char *pP = pBuffer; 
        for (size_t idx = 0; idx < bits; idx += 8) {
            ret |= ((unsigned int) *pP++) << idx;
        }
        return ret;
    }

    int getAsSignedInt() {
        int ret = (int) getAsUnsignedInt();
        if (bits < 31) {
            ret <<= 31 - bits;
            ret >>= 31 - bits;
        }
        return ret;
    }

    unsigned long long getAsUnsignedLongLong() {
        unsigned long long ret = 0;
        unsigned char *pP = pBuffer; 
        for (size_t idx = 0; idx < bits; idx += 8) {
            ret |= ((unsigned int) *pP++) << idx;
        }
        return ret;
    }
    long long getAsSignedLongLong() {
        long long ret = (long long) getAsUnsignedLongLong();
        if (bits < 63) {
            ret <<= 63 - bits;
            ret >>= 63 - bits;
        }
        return ret;
    }
};

#endif

