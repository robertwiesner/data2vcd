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

#ifndef SRC_CSIBLINGCHILD_H_
#define SRC_CSIBLINGCHILD_H_ 1
//
// This class is building a B* tree with variable of linked lists
// when an object is removed, it will destroy all children recursiviley.
class cSiblingChild
{
    cSiblingChild* pParent;
    cSiblingChild* pChildren;
    cSiblingChild* pPrev;
    cSiblingChild* pNext;
public:
    // add new sibling after the current one otherwise make the new simbling the first one
    cSiblingChild(cSiblingChild* pP, cSiblingChild* pS) {
        if (pP != nullptr && pS != nullptr && pP != pS->pParent) {
            // ERROS all siblings must have the same parent
            // create a floater and do not add to the list
            pP = nullptr;
            pS = nullptr;
        }
        pParent = pP;
        pChildren = nullptr;
        pPrev = pS;
        if (pS != nullptr) {
            // we add the sibling to an existing list of siblings with the same parent
            // and insert it after the provided sibling
            pNext = pS->pNext;
            pS->pNext = this;
        }
        else {
            if (pP != nullptr) {
                // we don't have a previous sibling but a parent, insert it as first child
                pNext = pP->pChildren;
                pP->pChildren = this;
            } else {
                pNext = nullptr;
            }
        }
    }
    ~cSiblingChild() {
        // delete all children
        while (pChildren) { delete pChildren; }
        unlink();
    }

    // functions to get the various 
    cSiblingChild* getTop() {
        cSiblingChild* pRet = this;
        if (pRet != nullptr) while (pRet->pParent != nullptr) {    pRet = pRet->pParent; }
        if (pRet != nullptr) pRet = pRet->getFirstSibling();
        return pRet;
    }

    cSiblingChild* getParent() { return pParent; }

    cSiblingChild* getFirstSibling() {
        cSiblingChild* pRet = this;
        if (pRet != nullptr) while (pRet->pPrev != nullptr) { pRet = pRet->pPrev; }
        return pRet;
    }

    cSiblingChild* getLastSibling() {
        cSiblingChild* pRet = this;
        if (pRet != nullptr) while (pRet->pNext != nullptr) { pRet = pRet->pNext; }
        return pRet;
    }

    cSiblingChild* getPrev() { return pPrev; }
    cSiblingChild* getNext() { return pNext; }
    cSiblingChild* getChildren() { return pChildren; }

    // insert a list of siblings to and existing one
    // inserting no sibling is always successful
    // if any of the siblings already have a parent, they are not added
    // Assign to all new sbilings the current parent
    // insert the list either at the current location (after/default)
    // or before the current sibling, parent might be updated with first child
    // return true if sucessful added
    bool addSiblings(cSiblingChild* pNewSiblings, bool insertAfter = true) {
        if (pNewSiblings == nullptr) { return true; }

        cSiblingChild* pLast = pNewSiblings->getLastSibling();

        pNewSiblings = pNewSiblings->getFirstSibling();

        cSiblingChild* pTmp = pNewSiblings;
        while (pTmp != nullptr) {
            if (pTmp->pParent != nullptr) { return false; }
        }
        // assign the parent;
        pTmp = pNewSiblings;
        while (pTmp != nullptr) {
            pTmp->pParent = pParent;
        }
        // add it to the list
        if (insertAfter) {
            pNewSiblings->pPrev = this;
            pLast->pNext = pNext;
            if (pNext != nullptr) { pNext->pPrev = pLast; }
            pNext = pNewSiblings;
        }
        else {
            // If added prior to the current sibling, we need to check if this is the first in the list
            // if that is the case, we may need to update the parent as well
            pNewSiblings->pPrev = pPrev;
            if (pPrev != nullptr) {
                pPrev->pNext = pNewSiblings;
            }
            else if (pParent != nullptr) {
                pParent->pChildren = pNewSiblings;
            }
            pPrev = pLast;
            pLast->pNext = this;
        }
    }
    // remove this object from a parent list and remove the link to the parent
    // return true if object changed
    bool unlink() {
        bool ret = false;
        if (pPrev == nullptr) {
            if (pParent != nullptr) {
                pParent->pChildren = pNext;
                pParent = nullptr;
                ret = true;
            }
            if (pNext != nullptr) {
                pNext->pPrev = nullptr;
                pNext = nullptr;
                ret = true;
            }
        }
        else {
            ret = true;
            pParent = nullptr;
            pPrev->pNext = pNext;
            if (pNext != nullptr) {
                pNext->pPrev = pPrev;
                pNext = nullptr;
            }
            pPrev = nullptr;
        }
        return ret;
    }
};
#endif  // SRC_CSIBLINGCHILD_H_
