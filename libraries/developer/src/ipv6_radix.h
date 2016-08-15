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

//
// Ported from FreeBSD 4.7
// This file contains function for radix search mechanisim
//

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//  This product includes software developed by the University of
//  California, Berkeley and its contributors.
// 4. Neither the name of the University nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//

#ifndef _RADIX_H_
#define _RADIX_H_



/// leaf contains normal route
#define RNF_NORMAL  1       // leaf contains normal route


/// leaf is root leaf for tree
#define RNF_ROOT    2       // leaf is root leaf for tree


/// This node is alive (for rtfree)
#define RNF_ACTIVE  4       // This node is alive (for rtfree)


/// Ignore this entry (for if down)
#define RNF_IGNORE  8       // Ignore this entry (for if down)



/// Local route
#define RT_LOCAL   0


/// Remote route
#define RT_REMOTE  2



/// Automatic configured.
#define AUTOCONF   1

/// Radix node structure.
typedef struct radix_node_
{
    int childlink;
    unsigned char rn_flags;
    void* nextNode[2];
    struct radix_node_* parent;
}radix_node;

// Structure type definition for a radix search trie.


/// Structure type definition for a radix search trie.
typedef struct radix_node_head_
{
    int maxKeyLen;
    radix_node* root;
    unsigned char* (* getValueFnPtr)(void *);
    int totalNodes;
    radix_node** stack;
    int* path_info;
}radix_node_head;

#ifndef MIN
#define MIN(a,b) a>b?b:a
#endif



/// Radix node leaf structure
typedef struct rn_leaf_
{
    in6_addr key;
    unsigned int keyPrefixLen;
    unsigned char rn_flags;

    //unsigned int linkLayerAddr;
    MacHWAddress linkLayerAddr;

    unsigned char flags;
    int interfaceIndex;
    int type;
    int rn_option;
    unsigned char ln_state;
    clocktype expire;
    in6_addr gateway;
    unsigned metric;
    radix_node* parent;
}rn_leaf;

/// NDP cache for reverse lookup based on link layer address
/// as key.
typedef struct rn_reverse_ndp_cache
{
    rn_leaf* ndp_cache_entry;

    rn_reverse_ndp_cache* next;
} rn_rev_ndplookup;

// Function Declarations


/// Radix Node allocation function
///
///
/// \return radix_node*
radix_node* radixNodeAlloc();



/// Radix leaf allocation function.
///
///
/// \return pointer to radix node leaf structure
rn_leaf*
Rn_LeafAlloc();



/// Radix Tree initialization.
///
/// \param ipv6  Ipv6 data pointer of
///    :               node.
///
void
rn_init(struct ipv6_data_struct* ipv6);



/// It matches particular key in the radix tree.
///
/// \param keyNode  Void pointer to radix key node.
/// \param tree  pointer to radix tree head.
///
/// \return void*
void*
rn_match(void* keyNode, radix_node_head* tree,Node* node);



/// It finds particular key in the radix tree.
///
/// \param keyNode  Void pointer to radix key node.
/// \param tree  Pointer to radix tree head.
///
/// \return void*
void *rst_find(void* keyNode, radix_node_head* tree);



/// It searches particular key in the radix tree.
///
/// \param keyNode  Void pointer to radix key node
/// \param tree  Readix tree head.
///
/// \return void*
void*
rn_search(void* keyNode, radix_node_head* tree);



/// Inserts an item into the radix search tree
/// pointed to by tree , according the the value
/// its key.  The key of an item in the radix
/// search trie must be unique among items in the
/// tree.  If an item with the same key already exists
/// in the tree, a pointer to that item is returned.
/// Otherwise, NULL is returned, indicating insertion
/// was successful.
///
/// \param keyNode  Void pointer to radix key node
/// \param tree  Pointer to radix tree head.
///
/// \return void*
void* rn_insert(Node* node, void* keyNode, radix_node_head* tree);

/// Get the data of radix node
///
///    +radix_node*         : radix_node* : Pointer to the Radix Node
///
/// \return Pointer to Radix Node Leaf structure
rn_leaf* GetData(radix_node*);

/// Radix tree traversal.
///
/// \param node  Pointer to Node.
/// \param radixNode  Void pointer to radix node.
/// \param printNextHopInfoFlag  prints next hop informations flag.
///
void radix_traversal(Node* node, radix_node* radixNode, int printNextHopInfoFlag);


/// Deletes an item from the radix-tree. Items are
/// virtually deleted by setting its flag as RNF_IGNORE.
///
/// \param keyNode  Void pointer to radix node.
/// \param tree  points the head of radix-tree .
///
/// \return void *
void *rn_delete(void* keyNode, radix_node_head* tree);

// API                  : radix_empty
/// Function to print empty radix tree
///
/// \param radixNode  Void pointer to radix node.
/// \param nrpType  Routing protocol type.
///    RETURN               : None
///    ---------------------------------------------------------------------------
void radix_empty(radix_node* radixNode, NetworkRoutingProtocolType nrpType);

//// FUNCTION   :: radix_GetNextHopFromLinkLayerAddress
//// LAYER      :: NETWORK
//// PURPOSE    :: Get Ipv6 Next Hop Address from Link Layer Address.
//// PARAMETERS ::
////  +node           : Node*         : Pointer to Node.
////  +nextHopAddr    : in6_addr*     : Pinter to Next Hop Address.
////  +llAddr         : NodeAddress   : Link Layer address.
//// RETURN     :: void : NULL.


BOOL
radix_GetNextHopFromLinkLayerAddress(
   Node* node,
    in6_addr* nextHopAddr,
    MacHWAddress* linkLayerAddr);

void rn_insert_reverse_lookup(Node* node, rn_leaf* route);


void rn_delete_reverse_lookup(Node* node, MacHWAddress& linkLayerAddress);

void rn_delete_reverse_lookup(Node* node, NodeAddress linkLayerAddress);

#endif
