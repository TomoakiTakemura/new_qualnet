// Copyright (c) 2001-2015, SCALABLE Network Technologies, Inc.  All Rights Reserved.
//                          600 Corporate Pointe
//                          Suite 1200
//                          Culver City, CA 90230
//                          info@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

/// \defgroup Package_SPLAYTREE SPLAYTREE

/// \file
/// \ingroup Package_SPLAYTREE
/// This file describes a tree and a heap structure called
/// splaytree and heapSplaytree to handle the timed activities
/// of the network model

#ifndef _SIMPLE_SPLAYTREE_H_
#define _SIMPLE_SPLAYTREE_H_

#include "clock.h"
#include "memory.h"

/// Structure of each node of a Splaytree
typedef struct simple_splay_node_str {
    clocktype timeValue;
    Message*     element;
    struct simple_splay_node_str* leftPtr;
    struct simple_splay_node_str* rightPtr;
    struct simple_splay_node_str* parentPtr;
} SimpleSplayNode;

/// Structure of a Splaytree
typedef struct simple_splay_tree_str {
    clocktype        timeValue;
    SimpleSplayNode* rootPtr;
    SimpleSplayNode* leastPtr;
} SimpleSplayTree;

/// To insert an element into the Splaytree
///
/// \param tree  Pointer to the splaytree node
/// \param splayNodePtr  Pointer to the splayNode
///    to be inserted
void SPLAY_Insert(SimpleSplayTree* tree,
                  SimpleSplayNode* splayNodePtr);

// API       :: SPLAY_ExtractMin
// PURPOSE   :: To extract an element from the Splaytree
// PARAMETERS ::
// + tree       : SimpleSplayTree* : Pointer to the splaytree node
//                           to be extracted
// RETURN    :: SimpleSplayNode* : Pointer to extracted Splaynode
SimpleSplayNode* SPLAY_ExtractMin(SimpleSplayTree* tree);


// API       :: SPLAY_AllocateNode
// PURPOSE   :: To allocate a node from the SplaytreeFreeList
// PARAMETERS ::
// + node       : Node*    : Pointer to the splaytree node
//                           to be extracted
// RETURN    :: SimpleSplayNode* : Pointer to extracted Splaynode
inline
SimpleSplayNode* SPLAY_AllocateNode() {

    SimpleSplayNode* splayNodePtr =
        (SimpleSplayNode *) MEM_malloc( sizeof(SimpleSplayNode) );
    assert(splayNodePtr != NULL);
    memset(splayNodePtr, 0, sizeof(SimpleSplayNode));

    return splayNodePtr;
}


// API       :: SPLAY_FreeNode
// PURPOSE   :: To free a node from the SplaytreeFreeList
// PARAMETERS ::
// + splayNodePtr       : SimpleSplayNode*        : Pointer to splay node
// RETURN    :: void :
inline
void SPLAY_FreeNode(SimpleSplayNode* splayNodePtr) {

    MEM_free(splayNodePtr);
    splayNodePtr = NULL;
}

/// utility function used by SPLAY_Insert
///
/// \param splayPtr  Pointer to the splaytree node
/// \param nodePtr  Pointer to the splayNode
///    to be inserted
void SPLAY_SplayTreeAtNode(SimpleSplayTree *splayPtr,
                           SimpleSplayNode *nodePtr);

#endif /* _SIMPLE_SPLAYTREE_H_ */

