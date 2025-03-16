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

#ifndef SRC_CTRACKMEM_H_
#define SRC_CTRACKMEM_H_ 1

#define ENABLE_TRACKING 1

#if ENABLE_TRACKING

#include <map>

class cTrackMem
{
    struct sAddrCmp {
        bool operator()(void* a, void* b) const { return a < b; }
    };
    struct sData {
        const char* pFileName;
        int line;
        sData(const char* pFN, int l) { pFileName = pFN; line = l; }
    };
    static std::map<void *, sData *, sAddrCmp> collect;

    public:
    cTrackMem(const char *pFN, int l) {
        collect[reinterpret_cast<void *>(this)] = new sData(pFN, l);
    }
    ~cTrackMem() {
        collect.erase(reinterpret_cast<void *>(this));
    }
    static void dumpData();
};

#define TRACKMEM_BASE_COL : cTrackMem
#define TRACKMEM_BASE_COM , cTrackMem

#define TRACKMEM_CONS_COL : cTrackMem(__FILE__, __LINE__)
#define TRACKMEM_CONS_COM , cTrackMem(__FILE__, __LINE__)
#else
#define TRACKMEM_BASE_COL
#define TRACKMEM_BASE_COM
#define TRACKMEM_CONS_COL
#define TRACKMEM_CONS_COM
#endif

#endif  // SRC_CTRACKMEM_H_

