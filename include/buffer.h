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

/// \defgroup Package_BUFFER BUFFER

/// \file
/// \ingroup Package_BUFFER
/// This file describes data structures and functions to implement buffers.

#ifndef _BUFFER_H_
#define _BUFFER_H_

/// Returns the current size of the buffer
#define BUFFER_GetCurrentSize(x)             (x)->currentSize

/// Returns maximum allowable size of the buffer
#define BUFFER_GetMaxSize(x)                 (x)->maxSize

/// Returns a pointer to the data in the buffer
#define BUFFER_GetData(x)                   (x)->data

/// Returns the intial size of the buffer
#define BUFFER_GetAnticipatedSize(x)             (x)->anticipatedSize

/// Sets current size of the buffer
#define BUFFER_SetCurrentSize(x,y)              (x)->currentSize = y

/// Get free space available in the buffer
#define BUFFER_GetFreeSpace(x)                  (x)->maxSize - (x)->currentSize

/// Returns top of the buffer
#define BUFFER_ReturnTop(x)               &((x)->data[(x)->currentSize])

/// Returns the no. of blocks in the buffer
#define BUFFER_NumberOfBlocks(X)        (X)->blocksAssembled

/// structure for the data buffer
typedef struct {
    int   maxSize;                // Maximum allowable size at current instance
    char* data;                   // Actual buffer
    int   currentSize;            //
    int   anticipatedSize;
} DataBuffer;

/// Format for the Reassembly buffer
typedef struct {
    int   maxSize;
    char* data;
    int   currentSize;
    int   anticipatedSize;
    int   blocksAssembled;
} ReassemblyBuffer;

/// structure for the packet buffer
typedef struct {
    char* data;
    char* buffer;
    int   currentSize;
    int   maxSize;
    BOOL  allowOverflow;
} PacketBuffer;

/// Initializing  Data buffer. Keeping in mind that buffer
/// will be initialized once and the guess for initial size is a good one.
/// For all the other manipulation of the buffer will try to allocate in the
/// initial if not asked to increase size and this size will remain until
/// end of program for re-using unless the buffer is closed completely.
///
/// \param buffer  buffer to be intialized
/// \param size  intial size of the buffer
inline void
BUFFER_InitializeDataBuffer(DataBuffer *buffer, int size)
{
    buffer->maxSize = size;
    if (size)
    {
        buffer->data = (char *) MEM_malloc(size);
        memset(buffer->data, 0, size);
    }
    else
    {
        buffer->data = NULL;
    }
    buffer->currentSize = 0;
    buffer->anticipatedSize = size;
}

/// Adding memory space to the buffer
///
/// \param buffer  buffer to which to add space
/// \param size  size to be added
inline void
BUFFER_AddSpaceToDataBuffer(DataBuffer *buffer, int size)
{
    char* newBuf;
    ERROR_Assert(buffer, "Can't add space as buffer is NULL");

    newBuf = (char *) MEM_malloc(buffer->maxSize + size);
    buffer->maxSize = (buffer->maxSize + size);
    memset(newBuf, 0, buffer->maxSize);
    memcpy(newBuf, buffer->data, buffer->currentSize);
    MEM_free(buffer->data);
    buffer->data = newBuf;
}

/// clear data from the buffer(already used portion of buffer
/// Not any unused portion unless u clear till end)
///
/// \param buffer  buffer from which data is cleared
/// \param startLocation  starting location
/// \param size  intial size of the buffer
/// \param destroy  destroy or not
inline void
BUFFER_ClearDataFromDataBuffer(DataBuffer *buffer, char *startLocation,
                               int size, BOOL destroy)
{

    int index = (int)(startLocation - buffer->data);
    ERROR_Assert(index >= 0 && index < buffer->currentSize,
        "The location is out of valid Data Boundary");
    ERROR_Assert((index + size - 1 < buffer->maxSize),
        "Can't clear: position + size is greater than size of data portion");

    if (size)
    {
        memmove(&buffer->data[index], &buffer->data[index + size], buffer->
            currentSize - index - size);

        buffer->currentSize -= size;
    }
    else
    {
        buffer->currentSize = index;
    }

    if (destroy)
    {
       // Remove the space keeping the initial anticipated size
       if (buffer->currentSize < buffer->anticipatedSize)
       {
           char *newData = (char*) MEM_malloc(buffer->
               anticipatedSize);

           memset(newData, 0, buffer->anticipatedSize);
           memcpy(newData, buffer->data, buffer->currentSize);
           MEM_free(buffer->data);
           buffer->data = newData;
           buffer->maxSize = buffer->anticipatedSize;
       }
       else
       {
           char *newData = (char*) MEM_malloc(buffer->currentSize);
           memcpy(newData, buffer->data, buffer->currentSize);
           MEM_free(buffer->data);
           buffer->data = newData;
           buffer->maxSize = buffer->currentSize;
       }
   }
   else
   {
       memset(&buffer->data[buffer->currentSize], 0, buffer->maxSize -
           buffer->currentSize);
   }
}

/// To Destroy a buffer
///
/// \param buffer  buffer to be destroyed
 inline void
 BUFFER_DestroyDataBuffer(DataBuffer* buffer)
 {
     if (buffer->data != NULL)
     {
        MEM_free(buffer->data);
        buffer->currentSize = 0;
        buffer->maxSize = 0;
        buffer->anticipatedSize = 0;
     }
 }

/// Add data to databuffer
///
/// \param buffer  buffer to which data is added
/// \param data  pointer to data that is added
/// \param size  initial size of the buffer
inline void
 BUFFER_AddDataToDataBuffer(
     DataBuffer *buffer,
     char *data,
     int size)
 {
     if (BUFFER_GetFreeSpace(buffer) < size)
     {
         BUFFER_AddSpaceToDataBuffer(buffer, buffer->anticipatedSize);
     }
     memcpy(BUFFER_ReturnTop(buffer), data, size);
     buffer->currentSize = buffer->currentSize + size;
 }

/// To remove data from the data buffer
///
/// \param buffer  buffer from which data is to be removed
/// \param startLocation  starting location from whcih data is removed
/// \param size  size of the buffer
inline void
BUFFER_RemoveDataFromDataBuffer(
    DataBuffer *buffer,
    char *startLocation,
    int size)
{
    int index = (int)(startLocation - buffer->data);
    int prevChunkNumber;
    int currentChunkNumber;
    ERROR_Assert(index >= 0 && index < buffer->currentSize,
        "The location is out of valid Data Boundary");
    ERROR_Assert((index + size - 1 < buffer->maxSize),
        "Can't clear: position + size is greater than size of data portion");

    prevChunkNumber = buffer->currentSize / buffer->anticipatedSize + 1;

    if (size)
    {
        memmove(&buffer->data[index], &buffer->data[index + size], buffer->
            currentSize - index - size);

        buffer->currentSize -= size;
    }
    else
    {
        buffer->currentSize = index;
    }

    currentChunkNumber = buffer->currentSize / buffer->anticipatedSize
                                 + 1;


    if (prevChunkNumber != currentChunkNumber)
    {
        int newSize = currentChunkNumber * buffer->anticipatedSize;
        char *newData = (char*) MEM_malloc(newSize);

        memset(newData, 0, newSize);
        memcpy(newData, buffer->data, buffer->currentSize);
        MEM_free(buffer->data);
        buffer->data = newData;
        buffer->maxSize = buffer->currentSize;
    }
}

/// Initialize  Reassembly buffer
///
/// \param buffer  ReassemblyBuffer to be initialized
/// \param size  maximum allowable size of the buffer
inline void
InitializeReassemblyBuffer(ReassemblyBuffer *buffer, int size)
{
    assert(size > 0);

    buffer->maxSize = size;
    buffer->data = (char *) MEM_malloc(buffer->maxSize);
    memset(buffer->data, 0, size);
    buffer->currentSize = 0;
    buffer->blocksAssembled = 0;
    buffer->anticipatedSize = 0;
}

/// Appending data to the reassembly buffer
///
/// \param buffer  Pointer to ReassemblyBuffer
/// \param data  data to be added
/// \param size  size of the data
/// \param allowOverflow  To allow overflow or not
inline void
BUFFER_AddDataToAssemblyBuffer(ReassemblyBuffer *buffer, char *data, int size, BOOL allowOverflow)
{

    char *newBuf;

    assert((data) && (buffer));

    if ((buffer->currentSize + size) >= buffer->maxSize)
    {
        if (!allowOverflow)
        {
            assert((buffer->currentSize + size) < buffer->maxSize);
        }
        else
        {
           newBuf = (char *)MEM_malloc(buffer->maxSize + size);
           memcpy(newBuf, buffer->data, buffer->maxSize);
           MEM_free(buffer->data);
           buffer->maxSize = (buffer->maxSize + size);
           buffer->data = newBuf;
        }
    }
    assert((buffer->currentSize + size) < buffer->maxSize);
    memcpy(&(buffer->data[buffer->currentSize]), data, size);
    buffer->currentSize = (buffer->currentSize + size);
    (buffer->blocksAssembled)++;
}

/// clear the buffer
///
/// \param buffer  Pointer to ReassemblyBuffer
/// \param max  the maximum size you want to set, if setSize is TRUE
/// \param setSize  TRUE, if the buffer max-size is to be re-set
inline void
BUFFER_ClearAssemblyBuffer(ReassemblyBuffer *buffer, int max, BOOL setSize)
{
    if (setSize)
    {
        buffer->maxSize = max;
        MEM_free(buffer->data);
        buffer->data = (char *) MEM_malloc(buffer->maxSize);
    }
    memset(buffer->data, 0, buffer->maxSize);
    buffer->currentSize = 0;
    buffer->blocksAssembled = 0;
}

/// To set the anticipated size of the assemblyBuffer
///
/// \param buffer  Pointer to ReassemblyBuffer
/// \param size  size to be set
inline void
BUFFER_SetAnticipatedSizeForAssemblyBuffer(ReassemblyBuffer *buffer, int size)
{
    assert(size <= buffer->maxSize);
    buffer->anticipatedSize = size;
}

/// To allocate packet buffer
///
/// \param initialSize  intial size of buffer
/// \param anticipatedHeaderMax  expected max header size
/// \param allowOverflow  if overflow is allowed
/// \param dataPtr  pointer to data array
///
/// \return Pointer to packetbuffer
inline PacketBuffer *
BUFFER_AllocatePacketBuffer(
    int initialSize,
    int anticipatedHeaderMax,
    BOOL allowOverflow,
    char **dataPtr)
{
    PacketBuffer *buffer = (PacketBuffer*) MEM_malloc(sizeof(PacketBuffer));
    buffer->maxSize = initialSize + anticipatedHeaderMax;
    buffer->buffer = (char*) MEM_malloc(buffer->maxSize);
    buffer->currentSize = initialSize;
    buffer->allowOverflow = allowOverflow;
    buffer->data = (buffer->buffer + anticipatedHeaderMax);

    if (dataPtr)
    *dataPtr = buffer->data;
    return buffer;
}

/// To allocate buffer with Intial header
///
/// \param initialSize  intial buffer size
/// \param initialHeaderSize  initial header size
/// \param anticipatedHeaderMax  expected max header size
/// \param allowOverflow  if overflow is allowed
/// \param dataPtr  pointer to array
/// \param headerPtr  pointer to array
///
/// \return Pointer to packetbuffer
inline PacketBuffer *
BUFFER_AllocatePacketBufferWithInitialHeader(
    int initialSize,
    int initialHeaderSize,
    int anticipatedHeaderMax,
    BOOL allowOverflow,
    char **dataPtr,
    char **headerPtr)
{
    PacketBuffer *buffer = BUFFER_AllocatePacketBuffer(initialSize + initialHeaderSize,
                                               anticipatedHeaderMax,
                                               allowOverflow,
                                               headerPtr);

    *dataPtr = *headerPtr + initialHeaderSize;

    return buffer;
}

/// To add header to buffer
///
/// \param buffer  Pointer to PacketBuffer
/// \param headerSize  size of header
/// \param headerPtr  Pointer to an array of strings
inline void
BUFFER_AddHeaderToPacketBuffer(
    PacketBuffer *buffer,
    int headerSize,
    char **headerPtr)
{
    assert(buffer);
    if (headerSize > (buffer->data - buffer->buffer))
    {
        int newHeaderSize =
            (int)(headerSize - (buffer->data - buffer->buffer));
        int remainingCurSize =
            (int)(buffer->maxSize - (buffer->data - buffer->buffer));
        char *newArray = (char*) MEM_malloc(buffer->maxSize + newHeaderSize);
        char *newTop = newArray + headerSize;

        assert(buffer->allowOverflow);

        memcpy(newTop, buffer->data, remainingCurSize);
        MEM_free(buffer->buffer);

        buffer->buffer = newArray;
        buffer->data = newTop;
        buffer->maxSize += newHeaderSize;
    }

    buffer->currentSize += headerSize;
    buffer->data -= headerSize;
    *headerPtr = buffer->data;
}

/// To remove header from packet buffer
///
/// \param buffer  Pointer to PacketBuffer
/// \param headerSize  size of header
/// \param dataPtr  Pointer to an strings array
inline void
BUFFER_RemoveHeaderFromPacketBuffer(
    PacketBuffer *buffer,
    int headerSize,
    char **dataPtr)
{
    assert(buffer);
    assert(headerSize > buffer->currentSize);

    buffer->currentSize -= headerSize;
    buffer->data += headerSize;
    *dataPtr = buffer->data;
}

/// To clear data from current buffer
///
/// \param buffer  Pointer to PacketBuffer
inline void
BUFFER_ClearPacketBufferData(
    PacketBuffer *buffer)
{
    assert(buffer);

    buffer->data = (buffer->buffer + buffer->maxSize);
    buffer->currentSize = 0;
}

/// Free the packet buffer passed as argument
///
/// \param buffer  Pointer to PacketBuffer
inline void 
BUFFER_FreePacketBuffer(
    PacketBuffer *buffer)
{
    MEM_free(buffer->buffer);
    MEM_free(buffer);
}

/// Add useful contents of source buffer as header to
/// to the destination  buffer
///
/// \param source  Pointer to PacketBuffer
/// \param dest  Pointer to PacketBuffer
inline void 
BUFFER_ConcatenatePacketBuffer(
    const PacketBuffer * source,
    PacketBuffer * dest)
{
    char *headerPtr;

    //get the size of source buffer
    int headerSize = BUFFER_GetCurrentSize(source);

    //allocate the place for header equal to size of source
    //buffer in front of content of destination buffer
    BUFFER_AddHeaderToPacketBuffer(dest, headerSize, &headerPtr);

    //copy the content of source buffer into the header allocated
    memcpy(headerPtr, BUFFER_GetData(source), headerSize);
}

#endif // PACKET_MANIPULATION_H

