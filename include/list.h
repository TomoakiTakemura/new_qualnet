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


/// \defgroup Package_LIST LIST

/// \file
/// \ingroup Package_LIST
/// This file describes the data structures and functions used in the
/// implementation of lists.


#ifndef _QUALNET_LIST_H_
#define _QUALNET_LIST_H_


/// Structure for each item of a generic container list
template<typename DATA>
struct tListItem
{
    DATA data;
    clocktype timeStamp;
    tListItem<DATA> *prev;
    tListItem<DATA> *next;
};

//
//
//
//       DEFINES A LIST THAT HOLDS POINTERS
//
//
///////////////////////////////////////////////////////////////////

typedef tListItem<void*> ListItem;

/// A list that stores different types of structures.
typedef struct LinkedList_
{
    int size;
    ListItem *first;       // First item in list.
    ListItem *last;        // Last item in list.
} LinkedList;

/// Initialize the list
///
/// \param node  Node that contains the list
/// \param list  Pointer to list pointer
///
void ListInit(Node *node, LinkedList **list);

/// Check if list is empty
///
/// \param node  Node that contains the list
/// \param list  Pointer to the list
///
/// \return If empty, TRUE, non-empty, FALSE
BOOL ListIsEmpty(Node *node, LinkedList *list);

/// Get the size of the list
///
/// \param node  Pointer to the node containing the list
/// \param list  Pointer to the list
///
/// \return Size of the list
int ListGetSize(Node *node, LinkedList *list);

/// Insert an item at the end of the list
///
/// \param node  Pointer to the node containing the list
/// \param list  Pointer to the list
/// \param timeStamp  Time the item was last inserted.
/// \param data  item to be inserted
///
void ListInsert(Node *node,
                LinkedList *list,
                clocktype timeStamp,
                void *data);

/// Find an item from the list
///
/// \param node  Pointer to the node containing the list
/// \param list  Pointer to the list
/// \param byteSkip  How many bytes skip to get the key item
///    from the listItem.
/// \param key  The key that an item is idendified.
/// \param size  Size of the key element in byte
///
/// \return Item found, NULL if not found
void* FindItem(Node *node,
               LinkedList *list,
               int byteSkip,
               char* key,
               int size);

/// Find an item from the list
///
/// \param node  Pointer to the node containing the list
/// \param list  Pointer to the list
/// \param byteSkip  How many bytes skip to get the key item
///    from the listItem.
/// \param key  The key that an item is idendified.
/// \param size  Size of the key element in byte
///
/// \return Item found, NULL if not found
void* FindItem(Node *node,
               LinkedList *list,
               int byteSkip,
               char* key,
               int size,
               ListItem** item);

/// Remove an item from the list
///
/// \param node  Pointer to the node containing the list
/// \param list  Pointer to the list to remove item from
/// \param listItem  item to be removed
/// \param freeItem  Whether to free the item
/// \param isMsg  Whether is this item a message? If it is
///    message, we need to call MESSAGE_Free for it.
///
void ListGet(Node *node,
             LinkedList *list,
             ListItem *listItem,
             BOOL freeItem,
             BOOL isMsg);

/// Free the entire list
///
/// \param node  Pointer to the node containing the list
/// \param list  Pointer to the list to be freed
/// \param isMsg  Does the list contain Messages? If so, we
///    need to call MESSAGE_Free.
///
void ListFree(Node *node, LinkedList *list, BOOL isMsg);

//
//
//
//       DEFINES A LIST THAT HOLDS INTEGERS
//
//
///////////////////////////////////////////////////////////////////

typedef tListItem<int> IntListItem;

/// A list that stores integers.
typedef struct IntList_
{
    int size;
    IntListItem *first;       // First item in list.
    IntListItem *last;        // Last item in list.
} IntList;

/// Initialize the list
///
/// \param node  Node that contains the list
/// \param list  Pointer to list pointer
///
void IntListInit(Node *node, IntList **list);

/// Check if list is empty
///
/// \param node  Node that contains the list
/// \param list  Pointer to the list
///
/// \return If empty, TRUE, non-empty, FALSE
BOOL IntListIsEmpty(Node *node, IntList *list);

/// Get the size of the list
///
/// \param node  Pointer to the node containing the list
/// \param list  Pointer to the list
///
/// \return Size of the list
int IntListGetSize(Node *node, IntList *list);

/// Insert an item at the end of the list
///
/// \param node  Pointer to the node containing the list
/// \param list  Pointer to the list
/// \param timeStamp  Time the item was last inserted.
/// \param data  item to be inserted
///
void IntListInsert(Node *node,
                IntList *list,
                clocktype timeStamp,
                int data);

/// Remove an item from the list
///
/// \param node  Pointer to the node containing the list
/// \param list  Pointer to the list to remove item from
/// \param listItem  item to be removed
/// \param freeItem  Whether to free the item
/// \param isMsg  Whether is this item a message? If it is
///    message, we need to call MESSAGE_Free for it.
///
void IntListGet(Node *node,
             IntList *list,
             IntListItem *listItem,
             BOOL freeItem,
             BOOL isMsg);

/// Free the entire list
///
/// \param node  Pointer to the node containing the list
/// \param list  Pointer to the list to be freed
/// \param isMsg  Does the list contain Messages? If so, we
///    need to call MESSAGE_Free.
///
void IntListFree(Node *node, IntList *list, BOOL isMsg);

#endif // _QUALNET_LIST_H_
