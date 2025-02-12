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

#include "cmodule.h"
#include "coutput.h"

void cModule::printHeader(cOutput *pO, std::string prefix)
{
    if (this == NULL) {
        return;
    }

    std::string newPrefix = 0 < prefix.size() ? prefix + "." + getName() : getName();

    if (pParent == NULL && pPrev == NULL) {
        pO->headerStart();
    }

    pO->headerModuleStart(this, prefix);

    for (std::vector<cWire *>::iterator it= wires.begin(); it != wires.end(); it++) {
        pO->headerWire(*it, newPrefix);
    }

    pChildren->printHeader(pO, newPrefix);

    pO->headerModuleEnd(this);
    
    pNext->printHeader(pO, prefix);

    if (pParent == NULL && pPrev == NULL) {
        pO->headerEnd();
    }
}
