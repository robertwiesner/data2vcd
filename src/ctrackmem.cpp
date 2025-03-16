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

#include <cstdio>
#include "ctrackmem.h"

#if ENABLE_TRACKING

std::map<void*, cTrackMem::sData *, cTrackMem::sAddrCmp> cTrackMem::collect;

void cTrackMem::dumpData() {
    int cnt = 0;
    for (const auto &it : collect) {
        printf("%p %s:%d\n", it.first, it.second->pFileName, it.second->line);
        cnt++;
    }
    printf("Remaing allocation: %d\n", cnt);
}

#endif
