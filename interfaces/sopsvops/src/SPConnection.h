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

#ifndef _SPCONNECTION_H_
#define _SPCONNECTION_H_

#include <pthread.h>
#include <string>
#include <vector>
#include "gui.h"
#include "ClientIDGenerator.h"

// Forward-declare WinSock.h definition as a struct
struct sockaddr_in;

#define MAX_SHMEM_NAME_LEN 32

/// \brief Contains information about one Scenario Player connection.
/// When the connection type is TCP, the connection object represents 
/// a connection with a unique socket. 
///
/// When using UDP, the port is not connected. The object contains the 
/// sockaddr_in client address for the sake of keeping track of the client
/// even though there is no connect() call.
//
class SPConnection
{
public:
    SPConnection(int _sock)
    {
        m_cntd = true;
        m_ctrl = false;
        m_errno = 0;
        m_name[0] = '\0';
        m_sock = _sock;
        m_subs = false;
        m_protocol = GetSopsProtocol();
        m_clientId = ClientIdGenerator::generateClientId(VOPS_CLIENT);
        m_clientAddr = NULL;
        m_sendingAllProperties = false;
    }

    const sockaddr_in* getClientAddr() {return m_clientAddr;}
    int getLastErrno() {return m_errno;}
    int getSock() {return m_sock;}
    bool isConnected() {return m_cntd;}
    bool isSendingAllProperties() {return m_sendingAllProperties;}
    bool isSubscribed() {return m_subs;}

    ~SPConnection();

    // FUNCTION   :: disconnect
    // PURPOSE    :: Shuts down the connection.
    // PARAMETERS :: none.
    void disconnect();

    /// Process received data. 
    /// \param buf  address of the received data
    ///             Must have room for len+1 bytes
    /// \param len  number of bytes to process
    // RETURNS    :: void
    void proc(char* buf, int len);

    /// Perform a non-blocking read from the connection
    ///               using either the socket or shared memory if
    ///               designated for the connection. Process complete
    ///               RPC commands
    /// NOTE       :: Called from the EXTERNAL_Interface receive function.
    /// \return number of bytes on success; -1 on error or disconnect
    int recv();

    /// Sends data to the connection using either the socket
    ///               or shared memory if designated for the connection.
    /// \param buf  address of the output data
    /// \param len  number of bytes to send
    /// \return number of bytes on success; -1 on error
    int send(const void *buf, int len);
    
    /// Send the scenario name to the connection
    void sendScenarioInfo(std::string scenarioName);

    void setConnected(bool cntd) {m_cntd = cntd;}
    void setClientAddr(sockaddr_in& clientAddr);
    void setSendingAllProperties(bool sending) {
        m_sendingAllProperties = sending;}
    void setSubscribed(bool subs) {m_subs = subs;}
    void sendClientID();

private:
    bool  m_cntd;  // Connected flag
    bool  m_ctrl;  // Accept control RPC calls (play, pause, ...)
    int   m_errno; // Last errno
    char  m_name[MAX_SHMEM_NAME_LEN];  // Shared memory name
    std::string m_RpcCmd; // Holds incomplete commands waiting for more data
    int   m_sock;  // TCP/UDP socket - also key value for the class
    bool  m_subs;  // Has subscribed to properties
    SopsProtocol m_protocol;  // TCP, UDP, or Shared Memory
    sockaddr_in *m_clientAddr; // client addr info for UDP sendto()
    std::string m_clientId; // Holds clientID for this conection
    volatile bool m_sendingAllProperties; // true during sendAllProperties 
};


/// Manages a set of SPConnection objects.
class SPConnectionManager
{
public:
    // Singleton
    static SPConnectionManager *getInstance();

    /// Adds an SPConnection object to the manager.
    /// \param spc  connection object pointer
    void add(SPConnection *spc);

    /// FFinds a connection matching Client Addr. 
    /// \param clientAddr sockaddr_in struct for the connection
    /// \return NULL if not found, else pointer to the object
    //               to the connection.
    SPConnection* find(sockaddr_in& clientAddr);

    /// Removes an SPConnection object from the manager. 
    /// \param dropit iterator referencing the connection to drop
    /// \return iterator pointing to the next connection in sequence
    std::vector<SPConnection*>::iterator
    drop(std::vector<SPConnection*>::iterator dropit);
    /// Called by the Scenario-Player EXTERNAL_Interface.
    ///               Calls the receive function for each SP connection.
    //
    void recv();

    /// Sends data to each Scenario Player connection that
    ///               indicates it has subscribed.
    /// \param buf  address of the output data
    /// \param len  number of bytes to send
    void send(const void *buf, int len);

    /// \return  Number of SPConnections
    int size() {return m_ncons;}

    /// Sets up a new subscriber.
    /// spc address of subscribing connection
    void subscribe(SPConnection *spc);

    /// Removes a subscriber.
    /// \param spc  address of subscribed connection
    void unsubscribe(SPConnection *spc);

    /// Closes all the connections and causes the program to exit.
    void shutdown();

    int getNumberOfSubscribers() {return m_nsubs;}
    pthread_mutex_t* getSendMutex() {return &m_sendMutex;}

private:

    SPConnectionManager();

    std::vector<SPConnection*>  m_connections;
    int                         m_ncons;  // Number of connections
    int                         m_nsubs;  // Number of subscribed connections
    pthread_rwlock_t            m_rwlock;
    pthread_mutex_t             m_sendMutex;
    static SPConnectionManager *m_single;
    bool                        m_subbed; // Ever been subscribed?
    int                         m_udpSock; // UDP socket bound to server port
};

#endif // ifndef _SPCONNECTION_H_
