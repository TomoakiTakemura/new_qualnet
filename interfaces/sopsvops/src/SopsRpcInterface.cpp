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
// Winsock headers and library directives

// The macro below prevents a re-definition error of in6_addr_struct in
// include/main.h.
#define _NETINET_IN_H

#include <Winsock2.h>
#include <WS2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#define err_no (WSAGetLastError())

#else
// Linux type of socket connection definitions
#include <sys/socket.h>
#include <netinet/in.h>
#define SOCKET int
#define err_no errno
#endif // WIN32

#include "SopsRpcInterface.h"
#include "SPConnection.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <sys/types.h> 
#include "gui.h"

#ifdef USE_MPI
//==== MPI ====
// When using MPI, Sops running in the master process communicates with
// Vops. Sops in the other processes use MPI_Send to forward property
// updates to the master process. 
#include <mpi.h>
#include <parallel_mpi.h>
#endif

#ifdef SOPS_DEBUG
extern void SopsLog(const char *printstr, ...);
#endif

//---------------------------------------------------------------------------
// SPReceive is the EXTERNAL_Function for the SPInterface receive
//---------------------------------------------------------------------------
void SPReceive(EXTERNAL_Interface *iface)
{
#ifdef USE_MPI
    if (!SopsPropertyManager::getInstance()->getInterface()->m_isMasterProcess)
        return;
#endif
    // Call the connection manager function to check for data on all
    // SP connections.
    SPConnectionManager::getInstance()->recv();
}

//---------------------------------------------------------------------------
// SPSimulationHorizon is the EXTERNAL_Function for advancing the 
// SPInterface simulation horizon 
//---------------------------------------------------------------------------
void SPSimulationHorizon(EXTERNAL_Interface *iface)
{
    PartitionData* partitionData = iface->partition;
    clocktype nextEventTime = GetNextInternalEventTime(partitionData);
    if (!partitionData->wallClock->isPaused())
    {
        iface->horizon = nextEventTime + 10 * MILLI_SECOND;
    }
}

//---------------------------------------------------------------------------
// TCP
//---------------------------------------------------------------------------
static pthread_t theListenerThread;
// This is the thread for the TCP implementation. It will listen for a client
// to connect. When one does it will accept and create an SPConnection to 
// handle it, then loop back to accept more connections.
void *listenerThread(void *p)
{
    SopsRpcInterface *sri = SopsPropertyManager::getInstance()->getInterface();
    struct sockaddr_in cli_addr;
    unsigned int       clilen = sizeof(cli_addr);
    SOCKET             connectFd;
    SOCKET             listenFd;

#ifdef _WIN32
    WSADATA            wsaData;
#endif

#ifdef USE_MPI
    ERROR_Assert(sri->m_isMasterProcess, 
        "MPI listener thread running in non-master process");
#endif

    // Create a socket and listen for a connection.
#ifdef _WIN32
    listenFd = INVALID_SOCKET;
    // When on Windows, init Winsock 2.
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        assert(false);
    }
#endif 

    // Set up socket
    listenFd = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _WIN32
    if (listenFd == INVALID_SOCKET)
    {
        char errmsg_buf[256]; 
        strerror_s(errmsg_buf, 255, err_no);
        WSACleanup();
        assert(false);
    }
#else
    assert(listenFd >= 0);
#endif

#ifdef SOPS_DEBUG
    SopsLog("SopsRpcInterface::listenerThread Listen socket = %d\n", listenFd);
#endif

    // Create address
    memset(&cli_addr, 0, sizeof(sockaddr_in));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = INADDR_ANY;
    cli_addr.sin_port = htons(sri->m_portno);

    // Set RESUSEADDR socket option to allow rebind despite the socket
    // being in TIME_WAIT.
    int optval = 1;
    setsockopt (listenFd, SOL_SOCKET, SO_REUSEADDR, 
        (const char *) &optval, sizeof(optval));

    // Bind address to the socket.
    // ::bind disambiguates it for the MSVC10 compiler due to confusion
    // between the socket bind defined in the global namespace (and used 
    // here) and the bind template defined in std namespace.
    int err = ::bind(listenFd, (sockaddr*) &cli_addr, sizeof(sockaddr));
#ifdef _WIN32
    assert(err != SOCKET_ERROR);
#else
    if (err < 0)
    {
        ERROR_ReportErrorArgs("Could not bind: %s", strerror(errno));
    }
#endif

    // Listen for a connection
    err = listen(listenFd, 2);
#ifdef _WIN32
    if (err == SOCKET_ERROR)
    {
        char errmsg_buf[256]; 
        strerror_s(errmsg_buf, 255, err_no);
        WSACleanup();
        assert(false);
    }
#else
    if (err < 0)
    {
        ERROR_ReportErrorArgs("Could not listen: %s", strerror(errno));
    }
#endif

    // Listen/service connection loop. 
    while (true)
    {
        // Accept connections
#ifdef _WIN32
        connectFd = accept(listenFd, NULL, NULL);
        if (connectFd == INVALID_SOCKET)
        {
            char errString[MAX_STRING_LENGTH];
            sprintf(errString, "Could not connect: %d", err_no);
            ERROR_ReportWarning(errString);
            continue;
        }

        // Make the connection socket non-blocking
        u_long iMode=1;
        ioctlsocket(connectFd, FIONBIO, &iMode); 
#else
        connectFd = accept(listenFd, NULL, NULL);
        if (connectFd < 0)
        {
            char errString[MAX_STRING_LENGTH];
            sprintf(errString, "Could not connect: %s", strerror(errno));
            ERROR_ReportWarning(errString);
            continue;
        }
#endif

#ifdef SOPS_DEBUG
        SopsLog("SopsRpcInterface::listenerThread connected on socket %d\n", 
            connectFd);
#endif

        // Connected to a client - create a new SPConnection and add
        // it to the connection manager.
        SPConnection *spc = new SPConnection(connectFd);
        spc->sendClientID();
        SPConnectionManager::getInstance()->add(spc);
    }
    return NULL;
}


// Initializes the interface to wait for a connection from vops.

SopsRpcInterface::SopsRpcInterface(
        unsigned short portno,
        SopsPropertyManager *propertyManager)
{
    m_portno = portno;
    m_propertyManager = propertyManager;

    m_connectionState = 2; // begin in paused state
    m_lastErrno = 0;

#ifdef USE_MPI
    // When using MPI, delay kicking off the listener thread until 
    // setIsMasterProcess(true) is called.
    m_isMasterProcess = false;
#else  
    // Kick off the listener thread
    pthread_create(&theListenerThread, NULL, &listenerThread, NULL); 
#endif
}

#ifdef USE_MPI
void SopsRpcInterface::setIsMasterProcess(bool isMaster)
{
    m_isMasterProcess = isMaster;
    if (isMaster)
    {
        pthread_create(&theListenerThread, NULL, &listenerThread, NULL);
    }
    else
    {
        // Have non-master process behave as if play command received.
        m_propertyManager->getPartition()->wallClock->resume();
    }
}
#endif

#ifdef USE_MPI
void SopsRpcInterface::processMpiCommand(char *cmd)
{
    char* etx = strchr(cmd, ETX);

    if (*(cmd + 1) != '(' || etx == NULL)
    {
        printf("processMpiCommand received garbled RPC: %s\n", cmd);
#ifdef SOPS_DEBUG
        SopsLog("processMpiCommand received garbled RPC: %s\n", cmd);
#endif
        return;
    }

    *etx = '\0';

#ifdef SOPS_DEBUG
    SopsLog("SOPS received MPI: %s\n", cmd);
#endif

    switch (*cmd)
    {
        case DeleteProperty:
        {
            m_propertyManager->deleteProperty(cmd + 2);
            break;
        }
        case SetProperty:
        {
            char* eq = strchr((char*)cmd, '=');
            if (eq)
            {
                *eq = '\0';
                m_propertyManager->setProperty(cmd + 2, eq + 1);
                break;
            }
        }
        // Fall thru if '=' not found

        default:
        {
            printf("processMpiCommand unexpected command: %s\n", cmd);
            fflush(stdout);
#ifdef SOPS_DEBUG
            SopsLog("processMpiCommand unexpected command: %s\n", cmd);
#endif  
            break;
        }
    }
}
#endif // USE_MPI

/// Common function to process commands however they were received.
//
void SopsRpcInterface::processRpcCommand(const char *cmd, SPConnection *spc)
{
    if (*(cmd + 1) != '(')
    {
        printf("SopsRpcInterface received garbled RPC: %s\n", cmd);
#ifdef SOPS_DEBUG
        SopsLog("SopsRpcInterface received garbled RPC: %s\n", cmd);
#endif
        return;
    }

#ifdef SOPS_DEBUG
    SopsLog("SOPS received RPC: %s\n", cmd);
#endif

    switch (*cmd)
    {
        case Exit:
           // unsubscribe results in dropping the connection.
            SPConnectionManager::getInstance()->unsubscribe(spc);
            // If all known clients have sent exit commands, done.
            if (SPConnectionManager::getInstance()->size() < 1)
            {
                // Exit command from the last connected VOPS client
                // will result in an exit.
                exit();
            }
            break;

        case Hitl:
            if (m_propertyManager->getPartition() != NULL)
            {
                GUI_HandleHITLInput(cmd + 2, m_propertyManager->getPartition());
            }
            else
            {
                printf("Partition undefined -- Unable to process HITL command. "
                       "Did build include addon/kernel?\n");
            }
            break;

        case Play:
            if (m_connectionState == 2)
            {
                if (m_propertyManager->getPartition() != NULL 
                    && m_propertyManager->getPartition()->wallClock != NULL)
                {  
                    // unpause simulation
                    m_propertyManager->getPartition()->wallClock->resume();
#ifdef SOPS_DEBUG
                    SopsLog("SopsRpcInterface : Unpaused\n");
#endif
                    fflush(stdout);
                    m_connectionState = 1;
                    m_propertyManager->setProperty("/remoteControl/playEnabled", "1");
                    m_propertyManager->setProperty("/remoteControl/pauseEnabled", "0");
                }
            }
            break;

        case Pause:
            if (m_connectionState == 1)
            {
                if (m_propertyManager->getPartition() != NULL 
                    && m_propertyManager->getPartition()->wallClock != NULL)
                {  
                    // pause simulation
                    m_propertyManager->getPartition()->wallClock->pause();
#ifdef SOPS_DEBUG
                    SopsLog("\nSopsRpcInterface : Paused\n");
#endif
                    m_connectionState = 2;
                    m_propertyManager->setProperty("/remoteControl/pauseEnabled", "1");
                    m_propertyManager->setProperty("/remoteControl/playEnabled", "0");
                }
            }
            break;

        case Query:
            {
                std::string qkey(cmd + 2);
                SopsProperty* qprop = m_propertyManager->getProperty(qkey);
                if (qprop)
                {
                    // Send only to the connection that requested the property
                    std::string reply(1, SetProperty);
                    reply += "(" + qprop->getKey() + "=" 
                        + qprop->getValue() + ETX;
#ifdef SOPS_DEBUG
                    SopsLog("Query: key=%s value=%s response=%s\n", 
                        cmd + 2, qprop->getValue().c_str(), 
                        reply.c_str());
#endif
                    // Wait to send if sending all properties.
                    while (spc->isSendingAllProperties()) {};

                    spc->send(reply.c_str(), reply.size());
                }
            }
            break;


        case Subscribe:
            SPConnectionManager::getInstance()->subscribe(spc);
            break;

        case Unsubscribe:
            SPConnectionManager::getInstance()->unsubscribe(spc);
            break;

        case RequestScenarioInfo:
            spc->sendScenarioInfo(SopsPropertyManager::getInstance()->getScenarioName());
            break; 

        default:
            printf("Unrecognized RPC call received from VOPS: %s)\n", cmd);
            fflush(stdout);
#ifdef SOPS_DEBUG
            SopsLog("Unrecognized RPC call received from VOPS: %s)\n", cmd);
#endif
    }
}

// Call the setProperty function in Vops
//
//    + key       : string     : Property key 
// \param value  Property value
void SopsRpcInterface::setProperty(
        const std::string& key, const std::string& value)
{
    char setCmd[2048];
    int cmdLen = 
        sprintf(setCmd, "S(%s=%s%c", key.c_str(), value.c_str(), ETX);
    ERROR_Assert(cmdLen < 2048, "setCmd buffer overflow");

#ifdef USE_MPI
    // If using MPI, non-master processes send cmd to master. 
    if (!m_isMasterProcess)
    {
        MPI_Request mpiRequest;
        MPI_Status mpiStatus;
        int flag = 0;

        // MPI_Send is a blocking send. Do not use it because the processes
        // will deadlock when they go to process stats->Aggregate() during
        // what is called the "Sops/Vops Event" that is done once a second. Here
        // MPI_Isend does a non-blocking send and MPI_Test will set the flag
        // parameter to 1 when the process can continue running.
        MPI_Isend((void*)setCmd,
                 cmdLen,
                 MPI_BYTE,
                 0, // partition zero
                 MPI_QUALNET_SOPS_RPC,
                 MPI_COMM_WORLD,
                 &mpiRequest);
        // Wait for send to complete
        while (!flag)
        {
            MPI_Test(&mpiRequest, &flag, &mpiStatus);
        }
        return;
    }
#endif

    if (SPConnectionManager::getInstance()->getNumberOfSubscribers() < 1)
    {
        m_lastErrno = ENOLINK;
        return;
    }

    SPConnectionManager::getInstance()->send(setCmd, cmdLen);
    m_lastErrno = err_no;

#ifdef SOPS_DEBUG
    SopsLog(
        "SopsRpcInterface::setProperty(%s, %s)\n", key.c_str(), value.c_str());
#endif
}

bool SopsRpcInterface::deleteProperty(const std::string& key)
{
    std::string delCmd;
    delCmd = "D(" + key + ETX;

#ifdef USE_MPI
    // When using MPI, only the master process manages the connection. 
    // Non-master processes use MPI_Send
    if (!m_isMasterProcess)
    {
        MPI_Request mpiRequest;
        MPI_Status mpiStatus;
        int flag = 0;
        // See the comment on MPI_Send vs. MPI_Isend in setProperty above.
        MPI_Isend((void*)delCmd.c_str(),
                 delCmd.size(),
                 MPI_BYTE,
                 0, // partition zero
                 MPI_QUALNET_SOPS_RPC,
                 MPI_COMM_WORLD,
                 &mpiRequest);
        while (!flag)
        {
            MPI_Test(&mpiRequest, &flag, &mpiStatus);
        }
        return true;
    }
#endif

    SPConnectionManager::getInstance()->send(delCmd.c_str(), (int)delCmd.size());


#ifdef SOPS_DEBUG
    SopsLog("VopsRpcInterface::deleteProperty(%s)\n",key.c_str());
#endif

    return true; // The interface doesn't get return values, so always set true.
}

void SopsRpcInterface::exit()
{
    static char exitCmd[4];
    int cmdLen = sprintf(exitCmd, "X(%c", ETX);

#ifdef SOPS_DEBUG
    SopsLog("SopsRpcInterface::exit(%s)\n",exitCmd);
#endif
    
    SPConnectionManager::getInstance()->send(exitCmd, cmdLen);
    SPConnectionManager::getInstance()->shutdown(); // calls ::exit(0)
}
