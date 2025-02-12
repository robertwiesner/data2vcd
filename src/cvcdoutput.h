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

#ifndef CVCDOUTPUT_H
#define CVCDOUTPUT_H

#include <stdio.h>
#include "coutput.h"

class cVCDOutput : public cOutput {
    size_t wireCount;
    size_t wireCountMax;
    const char *shortWireGenerator(char *pBuffer) {
        *--pBuffer = 0;
        size_t val = wireCount;
        do {
            *--pBuffer = 33 + val % (127 - 33);
            val /= (127 - 33);
        } while (val != 0);
        wireCount++;
        return pBuffer;
    }
    public:
    cVCDOutput(FILE *pO) : cOutput(pO) {
        wireCount = 0;
    }

    virtual ~cVCDOutput();
    
    virtual const char *getStringValue(cWire *);

    virtual void headerStart();
    virtual void headerSetStartTime(long long);
    virtual void headerModuleStart(cModule *, std::string prefix);
    virtual void headerModuleEnd(cModule *);
    virtual void headerWire(cWire *, std::string prefix);
    virtual void headerEnd();
    
    virtual void setTime(long long);
    virtual void print(cWire *);

    virtual void finish();

};

#endif
