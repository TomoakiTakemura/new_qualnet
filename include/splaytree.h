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
/// This file describes data structures and functions used for
/// scheduling simulation events.

#ifndef _SPLAYTREE_H_
#define _SPLAYTREE_H_

#include "main.h"

/// Maximum number of free lists in splaytree.
#define SPLAYTREE_MAX_FREE_LIST 50000

/// A heap that determines the earliest time from a bunch
/// of splay trees
typedef struct heap_splay_tree_str {
    Node **heapNodePtr;
    Int32 heapSize;
    Int32 length;
} HeapSplayTree;

/// Structure of each node of a Splaytree
typedef struct splay_node_str {
    clocktype timeValue;
    void *msg;
    struct splay_node_str *leftPtr;
    struct splay_node_str *rightPtr;
    struct splay_node_str *parentPtr;
} SplayNode;

/// Structure of a Splaytree
typedef struct splay_tree_str {
    SplayNode *rootPtr;
    SplayNode *leastPtr;
    Int32 heapPos;
} SplayTree;

/// To insert a node into the HeapSplaytree
///
/// \param heapSplayTreePtr  Pointer to the heapsplaytree
/// \param node  Pointer to the heapsplaytree node
///    to be inserted
void HeapSplayInsert(HeapSplayTree *heapSplayTreePtr, Node *node);

/// To Delete a node from the HeapSplaytree
///
/// \param heapSplayTreePtr  Pointer to the heapsplaytree
/// \param node  Pointer to the heapsplaytree node
///    to be deleted
void HeapSplayDelete(HeapSplayTree *heapSplayTreePtr, Node *node);

/// To insert a node into the Splaytree
///
/// \param node  Pointer to the splaytree node
/// \param splayNodePtr  Pointer to the splayNode
///    to be inserted
void SplayTreeInsert(Node *node, SplayNode *splayNodePtr);

// API       :: SCHED_SplayTreeExtractMin
// PURPOSE   :: To extract a a node from the Splaytree
// PARAMETERS ::
// + node       : Node*    : Pointer to the splaytree node
//                           to be extracted
// RETURN    :: SplayNode* : Pointer to extracted Splaynode
SplayNode* SplayTreeExtractMin(Node *node);


// API       :: SplayAllocateNode
// PURPOSE   :: To allocate a node from the SplaytreeFreeList
// PARAMETERS ::
// + node       : Node*    : Pointer to the splaytree node
//                           to be extracted
// RETURN    :: SplayNode* : Pointer to extracted Splaynode
inline
SplayNode* SplayAllocateNode(Node *){

    SplayNode *splayNodePtr =
        (SplayNode *) MEM_malloc( sizeof(SplayNode) );
    assert(splayNodePtr != NULL);
    memset(splayNodePtr, 0, sizeof(SplayNode));

    return splayNodePtr;
}


// API       :: SplayFreeNode
// PURPOSE   :: To free a node from the SplaytreeFreeList
// PARAMETERS ::
// + partitionData      : PartitionData*    : Pointer to the partition data
//
// + splayNodePtr       : SplayNode*        : Pointer to splay node
// RETURN    :: void  :
inline
void SplayFreeNode(PartitionData *, SplayNode *splayNodePtr){

    MEM_free(splayNodePtr);
    splayNodePtr = NULL;
}


#endif /* _SPLAYTREE_H_ */

