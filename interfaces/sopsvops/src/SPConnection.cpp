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
// use in compliance with the license agreement as part of the Qualnet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <errno.h>
#include <sys/socket.h>
#endif

#include <stdio.h>
#include <gui.h>
#include <qualnet_error.h>
#include <SPConnection.h>
#include <SopsProperty.h>
#include <SopsRpcInterface.h>

#ifdef SOPS_DEBUG
extern void SopsLog(const char *printstr, ...);
#endif

// Receive data from the connection. This is a non-blocking read, so it
// may receive zero bytes. If anything is received, complete commands are
// processed by the SopsRpcInterface::processRpcCommand. Partial commands
// are saved in m_RpcCmd until recv is called again.
//     -1:   error or disconnect
//     >= 0: number of bytes received.
int SPConnection::recv()
{
    SopsRpcInterface *sri = SopsPropertyManager::getInstance()->getInterface();
    int              nBytes;
    char             buf[BUFFER_SIZE];
    int              error_no = 0;
    // In Windows the port is set to non-blocking via ioctlsocket, so no
    // additional flags are needed. In Linux, the MSG_DONTWAIT flag calls
    // for non-blocking i/o.
#ifdef _WIN32
    const int        flags = 0;
#else // Linux
    const int        flags = MSG_DONTWAIT;
#endif

    ERROR_Assert(m_protocol == SOPS_PROTOCOL_TCP, 
                 "SPConnection::recv only supports TCP");

    nBytes = ::recv(m_sock, buf, BUFFER_SIZE - 1, flags);
    // Saving the error number before it
    // gets overwritten by SopsLog()
    if (nBytes < 0)
    {
        // An error occured or the socket was disconnected. 
#ifdef _WIN32
        error_no = WSAGetLastError();
#else
        error_no = errno;
#endif
    }
#ifdef SOPS_EXTRA_DEBUG
    SopsLog("SPConnection::recv(sock=%d) nBytes=%d\n",
        m_sock, nBytes);
#endif
    if (nBytes == 0)
    {
        // The remote has gracefully shutdown the connection.
        // Return -1 so that this SPConnection will be dropped.
        return -1;
    }

    if (nBytes < 0)
    {
        // An error occured or the socket was disconnected. 
#ifdef _WIN32
        if (error_no == WSAEWOULDBLOCK )
        {
            // non-blocking read when no data ready
            m_errno = error_no;
            return 0;
        }

        if (error_no != WSAECONNRESET)
        {
            // Report error except for disconnection
#ifdef SOPS_DEBUG
            SopsLog(
#else
            fprintf(stderr,
#endif
                "SPConnection::recv "
                "Winsock recv returned errno %d on socket %d\n",
                error_no, m_sock);
        }
#else
        // Check the error status on socket when error_no is 0
        if (error_no == 0)
        {
            int errCode = -1;
            int errCodelen = sizeof(errCode);
            int retval = getsockopt(m_sock,
                                    SOL_SOCKET,
                                    SO_ERROR,
                                    (char *)&errCode,
                                    (socklen_t *)&errCodelen);
            if (retval < 0)
            {
#ifdef SOPS_DEBUG
                SopsLog(
#else
                fprintf(stderr,
#endif
                    "SPConnection::recv TCP connection error, errno=%d, errstr=%s\n",
                        errCode, strerror(errCode));
            }
            else // No error
            {
                m_errno = error_no;
                return 0;
            }
        }
        if (error_no == EWOULDBLOCK || error_no == EAGAIN)
        {
            // non-blocking read found no data ready
            m_errno = error_no;
            return 0;
        }

        if (error_no != ECONNRESET && error_no != EPIPE)
        {
#ifdef SOPS_DEBUG
            SopsLog(
#else
            fprintf(stderr,
#endif
                "SPConnection::recv TCP connection error, errno=%d",
                error_no);
        }            
#endif
        m_errno = error_no;
        return nBytes;
    }

    // Process the received data
    sri->m_lastErrno = 0;
    proc(buf, nBytes);
    return nBytes;
}

// Process data received from VOPS
void SPConnection::proc(char* buf, int len)
{
    char* pCmdEnd;
    char* pCmdStart = buf;

    buf[len] = '\0';

#ifdef SOPS_EXTRA_DEBUG
    SopsLog("SPConnection::proc len=%d buf=%s\n", len, buf);
#endif

    while (*pCmdStart != '\0') // Loop while there is something
    {
        pCmdEnd = strchr(pCmdStart, ETX);
        if (pCmdEnd == NULL)
        {
            // There is no end of command in the buff, so save the incomplete 
            // command in the std::string for storage size management. Future 
            // recv's will read the rest of the command.
            m_RpcCmd += pCmdStart;

            // Return for now - future recv's will complete the command.
            return;
        }

        // There is an end of command character in the buffer.
        // Check for leftover from last buffer.
        *pCmdEnd = '\0'; // Change ETX to terminator.
        if (m_RpcCmd.size() > 0)
        {
            // Append the end of the command to the saved part.
            m_RpcCmd += pCmdStart;

            // Process the completed command
            SopsPropertyManager::getInstance()->getInterface()->
                processRpcCommand(m_RpcCmd.c_str(), this);
            m_RpcCmd.clear();
        }
        else
        {
            // At this point the entire command is in the buffer, 
            // so using the slower std::string is not needed. 
            SopsPropertyManager::getInstance()->getInterface()->
                processRpcCommand(pCmdStart, this);
        }

        // Point to the start of the next comnmand in the buffer,
        // or a nil character if the buffer is done.
        pCmdStart = pCmdEnd + 1;
    }
}


// Send data from the connection and return status:
//     -1 on error or disconnect
//    >=0 number of bytes sent on success
int SPConnection::send(const void *buf, int len)
{
    int error_no = 0;
    // Send data over the SOPS/VOPS interface.
    // Note that if the receiver is not ready,
    // send() may fail with error no. WSAEWOULDBLOCK,
    // since we're using a non-blocking socket.
    // In such cases, we must try again.
    int res = -1;
    pthread_mutex_t* sendMutex = SPConnectionManager::getInstance()->getSendMutex();
    while (res < 0)
    {
        pthread_mutex_lock(sendMutex);
        if (m_protocol == SOPS_PROTOCOL_UDP)
        {
            res = ::sendto(m_sock, (const char*)buf, len, 0,
                (sockaddr*)m_clientAddr, sizeof(sockaddr_in));
        }
        else
        {
            res = ::send(m_sock, (const char*)buf, len, 0);
        }
        pthread_mutex_unlock(sendMutex);

        if (res < 0)
        {
            // An error occured or the socket was disconnected. 
#ifdef _WIN32
            error_no = WSAGetLastError();
                
            // Report error if other than a disconnect or
            // receiver not ready
            if (error_no == WSAECONNRESET)
            {
                m_errno = error_no;
                return -1;
            }
            if (error_no != WSAEWOULDBLOCK)
            {
#ifdef SOPS_DEBUG
                SopsLog(
#else
                fprintf(stderr,
#endif
                    "SPConnection::send "
                    "Winsock send returned errno %d on socket %d\n",
                    error_no, m_sock);
                m_errno = error_no;
                return -1;
            }
#else
            error_no = errno;
            if (error_no == ECONNRESET || error_no == EPIPE)
            {
                m_errno = error_no;
                return -1;
            }
            else
            {
#ifdef SOPS_DEBUG
                SopsLog(
#else
                fprintf(stderr,
#endif
                    "SPConnection::send TCP connection error - errno=%d",
                    error_no);
                    
            }
            m_errno = error_no;
#endif
        }
    }
#ifdef SOPS_EXTRA_DEBUG
    SopsLog("SPConnection::send(sock=%d) buf=%s\n", m_sock, buf);
#endif
    return res;
}


void SPConnection::disconnect()
{
    if (m_cntd)
    {
        m_cntd = false;
#ifdef _WIN32
        ::shutdown(m_sock, SD_BOTH);
#else
        ::shutdown(m_sock, SHUT_RDWR);
#endif
    }
}

void SPConnection::setClientAddr(sockaddr_in& clientAddr)
{
    if (m_clientAddr)
    {
        delete m_clientAddr;
    }
    m_clientAddr = new sockaddr_in;
    memcpy(m_clientAddr, &clientAddr, sizeof(sockaddr_in));
}

// Destructor -- close the socket.
SPConnection::~SPConnection()
{
    disconnect();
    if (m_clientAddr)
    {
        delete m_clientAddr;
        m_clientAddr = NULL;
    }
}


// Singleton 
SPConnectionManager* SPConnectionManager::m_single = NULL;

SPConnectionManager *SPConnectionManager::getInstance()
{
    if (m_single == NULL)
    {
        m_single = new SPConnectionManager;
    }
    return m_single;
}

SPConnectionManager::SPConnectionManager()
{
    pthread_rwlock_init(&m_rwlock, NULL);
    pthread_mutex_init(&m_sendMutex, NULL);
    m_subbed = false;
    m_ncons = 0;
    m_nsubs = 0;
    m_udpSock = -1;

    // If using UDP, create a socket and bind it to receive from any sender.
    if (GetSopsProtocol() == SOPS_PROTOCOL_UDP)
    {
#ifdef _WIN32
        // When on Windows, init Winsock 2.
        WSADATA wsaData;
        m_udpSock = INVALID_SOCKET;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        {
            assert(false);
        }
#endif 
        m_udpSock = socket(AF_INET, SOCK_DGRAM, 0);
        assert(m_udpSock >= 0); // TBD change to user-friendly error handling

#ifdef _WIN32
        // In Windows, set the socket non-blocking. On linux the I/O will be flagged.
        u_long iMode=1;
        ioctlsocket(m_udpSock, FIONBIO, &iMode); 
#endif
        // Create a sockaddr to read from any address at the given port
        sockaddr_in recvAddr;
        memset((char *)&recvAddr, 0, sizeof(recvAddr));
        recvAddr.sin_family = AF_INET;
        recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        recvAddr.sin_port = htons(SopsPort());

        // Bind the sockaddr to the UDP socket
        int res = ::bind(m_udpSock, (sockaddr *)&recvAddr, sizeof(recvAddr));
        assert(res >= 0); // TBD change to user-friendly error handling
    }
}

// Add a connection to be managed
void SPConnectionManager::add(SPConnection *spc)
{
    pthread_rwlock_wrlock(&m_rwlock);
    m_connections.push_back(spc);
    pthread_rwlock_unlock(&m_rwlock);
    m_ncons++;
    
#ifdef SOPS_DEBUG
    SopsLog("SPConnectionManager::add(sock=%d)\n", spc->getSock());
#endif
}


// Drop a managed connection using an iterator and return the next 
// connection from the iterator sequence.
std::vector<SPConnection*>::iterator 
SPConnectionManager::drop(std::vector<SPConnection*>::iterator dropit)
{
#ifdef SOPS_DEBUG
    SopsLog("SPConnectionManager::drop(sock=%d) subbed=%s nconnections=%d\n", 
        (*dropit)->getSock(), (m_subbed ? "true" : "false"), m_ncons);
#endif

    // The std::vector erase operation was not behaving well with
    // multiple threads, resulting in a number of problems. For now, 
    // connections are disconnected and set so isConnected() returns
    // false, but are not removed from the vector using erase(). 
    // TBD use a thread-safe container.

    SPConnection *dropspc = *dropit;
    if (dropspc->isConnected())
    {
        dropspc->disconnect(); // Closes the connection
        m_ncons--;
        // Account for dropped subscribers.
        if (dropspc->isSubscribed())
        {
            m_nsubs--;
        }
    }

#ifdef SOPS_DEBUG
    SopsLog("Remaining subscribers: %d\n", m_nsubs);
#endif

    dropit++;
    return dropit;
}


// For UDP -- locate a client
SPConnection* SPConnectionManager::find(sockaddr_in& clientAddr)
{
    std::vector<SPConnection*>::iterator it = m_connections.begin();
    for (; it != m_connections.end(); it++)
    {
        const sockaddr_in* addr = (*it)->getClientAddr();
        if (addr == NULL)
        {
            continue;
        }
#ifdef _WIN32
        if (addr->sin_addr.S_un.S_addr == clientAddr.sin_addr.S_un.S_addr &&
            addr->sin_port == clientAddr.sin_port)
#else
        if (addr->sin_addr.s_addr == clientAddr.sin_addr.s_addr &&
            addr->sin_port == clientAddr.sin_port)
#endif
        {
            return *it;
        }
    }
    // Not found
    return NULL;
}

// Receive using a non-blocking read from each Scenario Player connection.
void SPConnectionManager::recv()
{
    SPConnection *spc;

    if (GetSopsProtocol() == SOPS_PROTOCOL_UDP)
    {
        // For UDP, one read is done from any sender. Although the socket is 
        // connectionless, SPConnection objects are used to keep track of all
        // the clients.
        int  nBytes;
        char buf[BUFFER_SIZE];
        int  error_no = 0;
        sockaddr_in clientAddr;

#ifdef _WIN32
        int flags = 0;
        int addrlen = sizeof(clientAddr);
#else
        int flags = MSG_DONTWAIT;
        socklen_t addrlen = sizeof(clientAddr);
#endif

        nBytes = ::recvfrom(m_udpSock, buf, BUFFER_SIZE - 1, flags, 
            (struct sockaddr *)&clientAddr, &addrlen);

        if (nBytes == 0)
        {
            // UDP is connectionless; no need to handle closed connections.
            return;
        }

        if (nBytes < 0)
        {
            // An error occured
#ifdef _WIN32
            error_no = WSAGetLastError();
            if (error_no == WSAEWOULDBLOCK )
            {
                // non-blocking read when no data ready
                return;
            }

#ifdef SOPS_DEBUG
            SopsLog(
#else
            fprintf(stderr,
#endif
                "SPConnectionManager::recv "
                "Winsock recvfrom returned errno %d on socket %d\n",
                error_no, m_udpSock);
#else
            error_no = errno;
            if (error_no == EWOULDBLOCK )
            {
                // non-blocking read when no data ready
                return;
            }

#ifdef SOPS_DEBUG
            SopsLog(
#else
            fprintf(stderr,
#endif
                "SPConnectionManager::recv error, errno=%d, errstr=%s\n",
                 error_no, strerror(error_no));
#endif // _WIN32
            return;
        }

        // Is this an existing client?
        SPConnection* spc = find(clientAddr);
        if (!spc)
        {
            // First time receiving from this client. All clients in UDP use the 
            // same socket.
            spc = new SPConnection(m_udpSock);
            add(spc);
            // UDP is done connectionless, but certain processing needs to get 
            // faked-out that this client is connected.
            spc->setConnected(true);
            spc->setClientAddr(clientAddr);
        }
        spc->proc(buf, nBytes);
        return;
    }

    // TCP
    int res;

    pthread_rwlock_rdlock(&m_rwlock); 
    std::vector<SPConnection*>::iterator it = m_connections.begin();
    while (it != m_connections.end())
    {
        spc = *it;
        // check for connected and subscribed
        if (spc->isConnected())
        {
            res = spc->recv();
            if (res < 0)
            {
                // Detected an error or disconnecton.
                // drop return next iterator
                it = drop(it);
                continue;
            }
        }

        // Get next connection
        if (it != m_connections.end()) it++;
    }
    pthread_rwlock_unlock(&m_rwlock); 
}

// Send data to each subscribed Scenario Player connection.
void SPConnectionManager::send(const void *buf, int len)
{
    int res = 0;

    // Loop thru each connection and repeat the send.
    // TBD -- consider multicasting, multicast with assured delivery, etc.
    // TBD -- Also consider implementing a service/daemon for multi connections.

    pthread_rwlock_rdlock(&m_rwlock);
    for (std::vector<SPConnection*>::iterator it = m_connections.begin();
         it != m_connections.end(); it++)
    {
        // check for connected and subscribed
        if ((*it)->isConnected() && (*it)->isSubscribed())
        {
            res = (*it)->send(buf, len);
            // For thread-safety, connections will only be dropped 
            // when recv detects a disconnection
        }
    }
    pthread_rwlock_unlock(&m_rwlock);
}

// Send info about the current scenario to this SP connection
void SPConnection::sendScenarioInfo(std::string scenarioName)
{
    std::string cmd(1, RequestScenarioInfo);
    cmd += "(" + scenarioName + ETX;
    this->send(cmd.c_str(), cmd.size());
}
// Subscribe a connection
void SPConnectionManager::subscribe(SPConnection *spc)
{
    // Must be connected. May send multiple subscribe commands to request
    // resending all properties.
    if (spc->isConnected())
    {
        // Enable sending future property changes
        if (!spc->isSubscribed())
        {
            spc->setSubscribed(true);
            m_subbed = true;
            m_nsubs++;
        }

        if (!spc->isSendingAllProperties())
        {
            // Send all the properties to one conection
            spc->setSendingAllProperties(true);
            SopsPropertyManager::getInstance()->sendAll(spc);
            spc->setSendingAllProperties(false);
        }
    }
}

// Remove a connection
void SPConnectionManager::unsubscribe(SPConnection *spc)
{
    // Find the connection and drop it
    for (std::vector<SPConnection*>::iterator it = m_connections.begin();
         it != m_connections.end();
         ++it)
    {
        if (*it == spc)
        {
            drop(it);
            break;
        }
    }
}

// Close all connections -- program will exit.
void SPConnectionManager::shutdown()
{
#ifdef SOPS_DEBUG
    SopsLog("SPConnectionManager::shutdown()\n");
#endif
    PARTITION_RequestEndSimulation();
    // Making sure that all the pauses have been undone
    // so that g_requestEndSimulation can trigger the wrapping up
    while (SopsPropertyManager::getInstance()->getPartition()->wallClock->isPaused())
    {
        SopsPropertyManager::getInstance()->getPartition()->wallClock->resume();
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

/// \brief to send UUID to client
void SPConnection::sendClientID()
{
    std::string uidCmd = "U(" + m_clientId + ETX;

#ifdef SOPS_DEBUG
    SopsLog("SopsRpcInterface::UID(%s)\n", uidCmd);
#endif // SOPS_DEBUG

    send(uidCmd.c_str(), uidCmd.length());
}
