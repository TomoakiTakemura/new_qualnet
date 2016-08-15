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

/// \defgroup Package_EXTERNAL_SOCKET EXTERNAL_SOCKET

/// \file
/// \ingroup Package_EXTERNAL_SOCKET
/// This file describes utilities for managing socket connections to external programs.

#ifndef _EXTERNAL_SOCKET_H_
#define _EXTERNAL_SOCKET_H_

#include <string>

#ifndef NO_SOCKET_THREADING
#ifdef _WIN32
#include "pthread.h"
#else
#include <pthread.h>
#endif /* _WIN32 */
#endif /* NO_SOCKET_THREADING*/

#include <string>

//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------

/// The default size of a VarArray
#define EXTERNAL_DEFAULT_VAR_ARRAY_SIZE 512

/// The thread buffer size
#define THREADED_BUFFER_SIZE 2000000

/// A listing of error types that could occur.
enum EXTERNAL_SocketErrorType
{
    EXTERNAL_NoSocketError = 0,     // No error
    EXTERNAL_SocketError = 1,       // Miscellaneous socket error
    EXTERNAL_DataNotSent = 2,       // The data was unable to be sent
    EXTERNAL_InvalidSocket = 3,     // Invalid socket stucture
};

//---------------------------------------------------------------------------
// Data Structures
//---------------------------------------------------------------------------

/// A variable sized array.  This structure is primarily used
/// to assemble data to be sent on a socket connection.
struct EXTERNAL_VarArray
{
    unsigned int maxSize;   // The current maximum size of the VarArray
    unsigned int size;      // The size of the data currently being used
    char *data;             // The data in the VarArray
};

#ifdef _WIN64
    typedef Int64 SocHandle; //To support SOCKET type UINT_PTR in
                             //64bit WINDOWS
#else
#ifdef _WIN32
    typedef int SocHandle;       // The socket
#else /* unix/linux */
    typedef int SocHandle;       // The socket
#endif
#endif

/// The socket data structure
struct EXTERNAL_Socket
{
    bool isOpen;        // TRUE if the socket is open
    SocHandle socketFd; //To support SOCKET type UINT_PTR in 64 bit WINDOWS

    bool blocking;

    // Variables used for threaded IO
    bool threaded;
    bool error;
#ifndef NO_SOCKET_THREADING
    volatile char buffer[THREADED_BUFFER_SIZE];
    volatile int head;
    volatile int tail;
    volatile int size;
    pthread_t reciever;
    pthread_mutex_t* mutex;
    pthread_cond_t* notFull;
    pthread_cond_t* notEmpty;

    volatile char sendBuffer[THREADED_BUFFER_SIZE];
    volatile int sendHead;
    volatile int sendTail;
    volatile int sendSize;
    pthread_t sender;
    pthread_mutex_t* sendMutex;
    pthread_cond_t* sendNotFull;
    pthread_cond_t* sendNotEmpty;
#endif /* NO_SOCKET_THREADING*/
};

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

/// This function will initialize a VarArray and allocate memory
/// for the array.  When the array is finished being used, call
/// EXTERNAL_VarArrayFree to free the memory.
///
/// \param array  Pointer to the uninitialized VarArray
/// \param size  The initial size of the array in bytes .  Defaults
///    to EXTERNAL_DEFAULT_VAR_ARRAY_SIZE
///
void EXTERNAL_VarArrayInit(
    EXTERNAL_VarArray *array,
    unsigned int size = EXTERNAL_DEFAULT_VAR_ARRAY_SIZE);

/// This function will increase the maximum size of the VarArray
/// so that it can contain at least "size" bytes.
///
/// \param array  Pointer to the VarArray
/// \param size  The new minimum size of the VarArray
///
void EXTERNAL_VarArrayAccomodateSize(
    EXTERNAL_VarArray *array,
    unsigned int size);

/// This function will add data to the end of the VarArray.
/// The size of the VarArray will be increased if necessary.
///
/// \param array  Pointer to the VarArray
/// \param data  Pointer to the data to add
/// \param size  The size of the data to add
///
void EXTERNAL_VarArrayAppendData(
    EXTERNAL_VarArray *array,
    char *data,
    unsigned int size);

/// This function will add a string to the end of the VarArray
/// including the terminating NULL character.  This function
/// ASSUMES that the previous data in the VarArray is also a
/// string -- ie, several bytes of data terminated with a NULL
/// character.  If this is not the case then the function
/// EXTERNAL_VarArrayAppendData should be used instead.
///
/// \param array  Pointer to the VarArray
/// \param string  The string
///
void EXTERNAL_VarArrayConcatString(
    EXTERNAL_VarArray *array,
    const char *string);

/// This function will free all memory allocated to the VarArray
///
/// \param array  Pointer to the VarArray
///
void EXTERNAL_VarArrayFree(EXTERNAL_VarArray *array);

/// Convert data from host byte order to network byte order
///
/// \param ptr  Pointer to the data
/// \param size  Size of the data
///
void EXTERNAL_hton(void* ptr, unsigned int size);

/// Convert data from network byte order to host byte order
///
/// \param ptr  Pointer to the data
/// \param size  Size of the data
///
void EXTERNAL_ntoh(void* ptr, unsigned int size);

/// 
///
/// \param ptr  Pointer to the data
/// \param size  Size of the data (in bytes)
///
void EXTERNAL_swapBitfield(void* ptr, unsigned int size);

/// Initialize a socket.  Must be called before all other socket
/// API calls on the individual socket.
///
/// \param socket  Pointer to the socket
///
void EXTERNAL_SocketInit(
    EXTERNAL_Socket *s,
    bool blocking = TRUE,
    bool threaded = FALSE);

/// Initialize a UDP socket.  Must be called before all other socket
/// API calls on the individual socket.
///
/// \param socket  Pointer to the socket
///
/// \return Error type
//
void EXTERNAL_SocketInitUDP(
    EXTERNAL_Socket *s,
    int MYPORT,
    bool blocking = TRUE,
    bool threaded = FALSE);

/// Check if a socket connection is valid: The socket is open
/// and no errors have occurred.
///
/// \param socket  Pointer to the socket
///
/// \return true if valid, FALSE if closed or errors
bool EXTERNAL_SocketValid(EXTERNAL_Socket *socket);

/// Listen and accept a connections on a socket.  This function
/// is a wrapper for EXTERNAL_SocketInitListen and
/// EXTERNAL_SocketAccept.
/// 
///
/// \param listenSocket  Pointer to the socket to listen on
/// \param port  The port to listen on
/// \param connectSocket  Pointer to the socket that will
///    receive the established connection
///
/// \return EXTERNAL_NoSocketError if
/// successful, different error if not successful which could
/// be due to a number of reasons.
EXTERNAL_SocketErrorType EXTERNAL_SocketListen(
    EXTERNAL_Socket *listenSocket,
    int port,
    EXTERNAL_Socket *connectSocket);

/// Initialize an input socket and have it listen on the given
/// port.  Call EXTERNAL_SocketAccept to accept connections on
/// the socket.  Call EXTERNAL_SocketDataAvailable to see if
/// there is an incoming connection that has not been accepted.
/// 
///
/// \param listenSocket  Pointer to the socket to listen on
/// \param port  The port to listen on
///
/// \return EXTERNAL_NoSocketError if
/// successful, different error if not successful which could
/// be due to a number of reasons.
EXTERNAL_SocketErrorType EXTERNAL_SocketInitListen(
    EXTERNAL_Socket* listenSocket,
    int port,
    bool threaded = FALSE,
    bool reuseAddr = FALSE);

/// Accept a connection on a listening socket.  This operation
/// may block if there is no incoming connection, use
/// EXTERNAL_SocketDataAvailable to check if there is an
/// incoming connection.
/// 
///
/// \param listenSocket  Pointer to the socket to listen on.
///    Must have previously called
///    EXTERNAL_SocketInitListen
/// \param connectSocket  Pointer to the newly created socket
///
/// \return EXTERNAL_NoSocketError if
/// successful, different error if not successful which could
/// be due to a number of reasons.
EXTERNAL_SocketErrorType EXTERNAL_SocketAccept(
    EXTERNAL_Socket* listenSocket,
    EXTERNAL_Socket* connectSocket);

/// Test if a socket has readable data.  For a listening socket
/// this will test for an incoming connection.  For a data
/// socket this will test if there is incoming data.
/// 
///
/// \param s  Pointer to the socket
/// \param available  TRUE if data is available
///
/// \return EXTERNAL_NoSocketError if
/// successful, different error if not successful which could
/// be due to a number of reasons.
EXTERNAL_SocketErrorType EXTERNAL_SocketDataAvailable(
    EXTERNAL_Socket *s,
    bool* available);

/// Connect to a listening socket.  The socket is set to
/// non-blocking mode.
///
/// \param socket  Pointer to the socket
/// \param address  String represent the address to connect to
/// \param port  The port to connect to
/// \param maxAttempts  Number of times to attempt connecting before an
///    error is returned
///
/// \return EXTERNAL_NoSocketError if
/// successful, different error if not successful which could
/// be due to a number of reasons.
EXTERNAL_SocketErrorType EXTERNAL_SocketConnect(
    EXTERNAL_Socket *socket,
    char *address,
    int port,
    int maxAttempts);

/// Send data on a connected socket.  Since the socket is
/// non-blocking, it is possible that the send would result in a
/// block: If the "block" parameter is FALSE, then
/// EXTERNAL_DataNotSent is returned, and no data is sent.
///
/// \param socket  Pointer to the socket
/// \param data  Pointer to the data
/// \param size  Size of the data
/// \param block  If this call may block.  Defaults to TRUE.
///
/// \return EXTERNAL_NoSocketError if
/// successful, different error if not successful which could
/// be due to a number of reasons.
EXTERNAL_SocketErrorType EXTERNAL_SocketSend(
    EXTERNAL_Socket *s,
    const char *data,
    unsigned int size,
    bool block = TRUE);

/// This is a wrapper for the above overloaded function.
///
/// \param socket  Pointer to the socket
/// \param data  Pointer to the VarArray to send
/// \param block  If this call may block.  Defaults to TRUE.
///
/// \return EXTERNAL_NoSocketError if
/// successful, different error if not successful which could
/// be due to a number of reasons.
EXTERNAL_SocketErrorType EXTERNAL_SocketSend(
    EXTERNAL_Socket *s,
    EXTERNAL_VarArray *data,
    bool block = TRUE);


EXTERNAL_SocketErrorType EXTERNAL_SocketSendTo(
    int sockFd,
    char *data,
    unsigned int size,
    unsigned int port,
    unsigned int hitlIp,
    bool block = TRUE);

/// Receive data on a connected socket.  Since the socket is
/// non-blocking, it is possible that the send would result in a
/// block: If the "block" parameter is FALSE, the "receiveSize"
/// parameter will be set to the amount of data received before
/// the blocking operation.  This amount could be any value
/// between 0 and size - 1.
///
/// \param socket  Pointer to the socket
/// \param data  Pointer to the destination
/// \param size  The amount of data to receive in bytes
/// \param size  The number of bytes received.  This could be less
///    than the specified size if an error occurred or if the block
///    parameter is FALSE.
/// \param block  TRUE if the call can block, FALSE if non-blocking.
///    Defaults to TRUE.
///
/// \return EXTERNAL_NoSocketError if
/// successful, different error if not successful which could
/// be due to a number of reasons.
EXTERNAL_SocketErrorType EXTERNAL_SocketRecv(
    EXTERNAL_Socket *s,
    char *data,
    unsigned int size,
    unsigned int *receiveSize,
    bool block = TRUE);

/// Receive data on a UDP socket.  Since the socket is
/// non-blocking, it is possible that the send would result in a
/// block: If the "block" parameter is FALSE, the "receiveSize"
/// parameter will be set to the amount of data received before
/// the blocking operation.  This amount could be any value
/// between 0 and size - 1.
///
/// \param socket  Pointer to the socket
/// \param data  Pointer to the destination
/// \param size  The amount of data to receive in bytes
/// \param size  The number of bytes received.  This could be less
///    than the specified size if an error occurred or if the block
///    parameter is FALSE.
/// \param ip  The IP address it was received from
/// \param port  The port it was received from
/// \param block  TRUE if the call can block, FALSE if non-blocking.
///    Defaults to TRUE.
///
/// \return EXTERNAL_NoSocketError if
/// successful, different error if not successful which could
/// be due to a number of reasons.
EXTERNAL_SocketErrorType EXTERNAL_SocketRecvFrom(
    EXTERNAL_Socket *s,
    char *data,
    unsigned int size,
    unsigned int *receiveSize,
    unsigned int *ip,
    unsigned int *port,
    bool block);

/// Receive data on a connected socket.  Continues reading until
/// a '\n' character is found.  This function always blocks.
///
/// \param socket  Pointer to the socket
/// \param data : std:  The read data
///
/// \return EXTERNAL_NoSocketError if
/// successful, different error if not successful which could
/// be due to a number of reasons.
EXTERNAL_SocketErrorType EXTERNAL_SocketRecvLine(
    EXTERNAL_Socket *s,
    std::string* data);

/// Close a socket.  Must be called for each socket that is
/// listening or connected.
///
/// \param socket  Pointer to the socket
///
/// \return EXTERNAL_NoSocketError if
/// successful, different error if not successful which could
/// be due to a number of reasons.
EXTERNAL_SocketErrorType EXTERNAL_SocketClose(EXTERNAL_Socket *s);

#endif /* _EXTERNAL_SOCKET_H_ */
