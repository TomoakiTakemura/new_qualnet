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

#include <errno.h>
#include "api.h"
#include "external.h"
#include "external_socket.h"
#include "external_util.h"
#include "proxy_sim-interlock.h"


#ifdef _WIN32
#include "qualnet_mutex.h"
#include "winsock.h"

// recv() is supposed to return an ssize_t, but on Windows (even
// 64-bit Windows) ssize_t is missing and recv() returns an int.
typedef int ssize_t;
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#ifndef s6_addr32
#define s6_addr32 s6_addr
#endif
//#include <netdb.h>
#include "qualnet_mutex.h"
#endif

#include<stdio.h>

// Parameters to control proxy's resource consumption
#define MULTI_GUI_DEFAULT_PORT 4100
#define PROXY_SIM_MAX_SLEEP_INTERVAL    (500*MICRO_SECOND)
#define PROXY_SIM_MIN_SLEEP_INTERVAL    (1*MICRO_SECOND)
#define PROXY_SIM_DEFAULT_SLEEP_INTERVAL (500*MICRO_SECOND)
#define PROXY_SIM_SLEEP_INTERVAL_STEP    (1*MICRO_SECOND)

bool g_requestedEndSimulation = false;

/// \brief Proxy_ParseAvg
///
/// This function is to parse command-line arguments
///             Proxy takes only two below arguments, and ignores others
///             (1) "-interactive hostName portNumber" for GUI socket connection
///             (2) configuration file name
///
///
/// \param argc  Number of command-line arguments
/// \param argv  Pointer to string of arguments
/// \param configFileName  Name of configuration file
///
/// \return true if we can parse two arguments of interest, false otherwise
bool Proxy_ParseArgv( int argc,
                     char **argv,
                     char* configFileName);

/// \brief Function to connect Proxy to master simulator
///
/// \param nodeInput  Pointer to data structure that contains all
///                   configuration information
//
/// \param multigui  Flag indicating whether proxy is launched with
///                   multigui command line option or not
/// \param exataMcAddrStr  IP address of machine running exata
/// \param multiguiPort  Port opened by exata for multigui interface
///
/// \return if sucessfully conencted, return a pointer to socket data
///  structure (EXTERNAL_Socket*), otherwise, return a null pointer
EXTERNAL_Socket* Proxy_ConnectToMultiGuiIntf(NodeInput* nodeInput,
                                             bool multigui,
                                             char* exataMcAddrStr,
                                             int multiguiPort);

/// \brief Function to send data received by proxy from
///        secondary GUI to master simulator
///
/// \param socketConn  Pointer to socket structure for
//                     master simulator
/// \param command  Data from secondary GUI to be sent to
///                 master simulator
void Proxy_SendToMultiGuiIntf(EXTERNAL_Socket* socketConn,
                           GuiCommand command);

/// \brief Function to receive master simulator data
///
/// \param socketConn  Pointer to socket structure for master simulator
/// \param time  Time value from master simulation
void Proxy_ReceiveFromMultiGuiIntf(EXTERNAL_Socket* socket, clocktype* time);

/// \brief Function to check socket connection validity
///
/// \param socketConn  Pointer to socket structure created
///                    for connecting master simulator
///
/// \return Returns socket connection validity
bool isConnectionValid(EXTERNAL_Socket* socketConn);

ProxySimInterlock proxySimInterlock;
bool receivedFirstStepCmd = false;

/*
    High-level structure
    --------------      ---------------------      ---------      -----------------
    |   GUI      | <--> |  Master Simulator | <--> | Proxy | <--> | Secondary GUI |
    --------------      ---------------------      ---------      -----------------
*/

/// \brief Main API for proxy
///
/// Main API for executing proxy funtionalities that include
///            (1) parsing arguments
///            (2) connect to secondary GUI
///            (3) conenct to master simulator
///            (4) loop of
///                (i) reading data from master simulator, process them and
///                    send to seconadry GUI
///                (ii) reading data from secondary GUI, process them and
///                    send to master simulator
///
/// \param argc  Number of command-line arguments
/// \param argv  Pointer to string of arguments
int main(int argc, char **argv)
{
    char        timeString[MAX_CLOCK_STRING_LENGTH];
    NodeInput   nodeInput;
    EXTERNAL_Socket* socketConn;
    EXTERNAL_Socket* guiSocket;
    bool guiDataAvail;

    bool guiOK;

    clocktype newTime;
    clocktype time = 0;
    char    configFileName [MAX_STRING_LENGTH];

    clocktype sleepInterval = PROXY_SIM_DEFAULT_SLEEP_INTERVAL;

    int thisArg = 1;
    bool enableMultguiFrmCmdLine = false;
    int multiguiPort = MULTI_GUI_DEFAULT_PORT;

    char exataAddrStr[MAX_STRING_LENGTH] = {0};
    while (thisArg < argc)
    {
        if (!strcmp(argv[thisArg], "-with-multi-gui")) {
            enableMultguiFrmCmdLine = true;
            if (argc < thisArg + 3) {
                ERROR_ReportError(
                    "Not enough arguments to -with_multi_gui.\n"
                    "Correct usage:\t -with_multi_gui EXATA_MACHINE_IP PORT_NUMBER.\n");
            }
            strcpy(exataAddrStr, argv[thisArg+1]);
            multiguiPort = atoi(argv[thisArg+2]);
            break;
        }
        thisArg++;
    }

    if (!Proxy_ParseArgv(argc, argv, configFileName))
    {
        ERROR_ReportWarning("Proxy can't start because of "
            "either not being connected to Gui or not having configuration file\n");
        //return 0;
    }

    guiSocket = (EXTERNAL_Socket*)MEM_malloc(sizeof(EXTERNAL_Socket));

    // Need to change the license check in CheckForLicenseError() below
    // if threaded(3rd parameter) gets changed to TRUE
    EXTERNAL_SocketInit(guiSocket, TRUE, FALSE);

    //GUI_guiSocket is a gobal variable defined in "gui.cpp"
    guiSocket->socketFd = GUI_guiSocket;

    guiSocket->isOpen = TRUE;

    IO_InitializeNodeInput(&nodeInput, false);
    IO_ReadFile(&nodeInput, configFileName);

    BOOL multiGuiInterface = FALSE;
    BOOL found = FALSE;

    if (!enableMultguiFrmCmdLine)
    {
        IO_ReadBool(ANY_NODEID,
                    ANY_ADDRESS,
                    &nodeInput,
                    "MULTI-GUI-INTERFACE",
                    &found,
                    &multiGuiInterface);
    }
    if ((! found || ! multiGuiInterface) && !enableMultguiFrmCmdLine)
    {
        GUI_DisconnectFromGUI(GUI_guiSocket,TRUE);
        ERROR_ReportWarning("MULTI-GUI-INTERFACE is disabled ...");
        return 0;
    }

    socketConn = Proxy_ConnectToMultiGuiIntf(&nodeInput,
                                             enableMultguiFrmCmdLine,
                                             exataAddrStr,
                                             multiguiPort);

    GuiReply    reply;

    // Sleep to make sure that we get to know whether GUI closed the connection
    EXTERNAL_Sleep(10 * MILLI_SECOND);

    if (isConnectionValid(socketConn))
    {
        // Let GUI know that we are good to go
        reply.type = GUI_STEPPED;
        ctoa (time, timeString);
        reply.args.append(timeString);
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else
    {
        // Propagate the error to GUI
        reply.type = GUI_ERROR;
        std::string error("No more GUI licenses could be found.");
        reply.args.append(error);
        GUI_SendReply(GUI_guiSocket, reply);
        GUI_DisconnectFromGUI(GUI_guiSocket, false);
        ERROR_ReportError(error.c_str());
        return 0;
    }

    proxySimInterlock.start();

    guiOK = false;
    while (1)
    {
        EXTERNAL_SocketDataAvailable(guiSocket, &guiDataAvail);
        if (guiDataAvail)
        {
            GuiCommand command;

            command = GUI_ReceiveCommand(GUI_guiSocket);

            if (command.type == GUI_STOP)
            {
                GUI_DisconnectFromGUI(GUI_guiSocket,TRUE);
                return 0;
            }

            if (command.type == GUI_STEP)
            {
                guiOK = true;
                if (receivedFirstStepCmd == false)
                {
                    Proxy_SendToMultiGuiIntf(socketConn, command);
                    receivedFirstStepCmd = true;
                }
            }
            else
            {
                Proxy_SendToMultiGuiIntf(socketConn,command);
            }

            sleepInterval = 0;
        }

        bool simulatorDataAvail;
        EXTERNAL_SocketDataAvailable(socketConn,  &simulatorDataAvail);
        if (simulatorDataAvail)
        {
            Proxy_ReceiveFromMultiGuiIntf(socketConn, &newTime);
            sleepInterval = 0;
        }

        if (!guiDataAvail && !simulatorDataAvail)
        {
            if (sleepInterval < PROXY_SIM_MAX_SLEEP_INTERVAL)
            {
                sleepInterval += PROXY_SIM_SLEEP_INTERVAL_STEP;
            }
        }

        if (guiOK)
        {
            proxySimInterlock.setReceivedStepCmdFlag(true);
            guiOK = false;
        }

        if (EXTERNAL_SocketValid(socketConn) == false)
        {
            // If connection to master simulator is lost, disconnect from secondary Gui and stop proxy
            GUI_DisconnectFromGUI(GUI_guiSocket,TRUE);
            return 0;
        }

        EXTERNAL_Sleep(sleepInterval);
    }

    return 0;
}

/// \brief Proxy_ParseAvg
///
/// this function is to parse command-line arguments
///             Proxy takes only two below arguments, and ignores others
///             (1) "-interactive hostName portNumber" for GUI socket connection
///             (2) configuration file name
///
///
/// \param argc  Number of command-line arguments
/// \param argv  Pointer to string of arguments
/// \param configFileName  Name of configuration file
///
/// \return true if we can parse two arguments of interest, false otherwise
bool Proxy_ParseArgv( int argc,
                     char **argv,
                     char* configFileName)
{
    int thisArg = 1;
    bool isConfigFileSet = false;
    bool isGuiConnected = false;

    while (thisArg < argc)
    {
        if (!strcmp(argv[thisArg], "-interactive")) {
            char hostname[64];
            int  portNumber;

            // if this arg is -interactive, then next two must be hostname and port
            if (argc < thisArg + 3)
            {
                ERROR_ReportError(
                    "Not enough arguments to -interactive.\n"
                    "Correct Usage:\t -interactive hostName portNumber.\n");
            }

            strcpy(hostname, argv[thisArg+1]);
            portNumber = atoi(argv[thisArg+2]);

            // Connect to secondary GUI
            GUI_ConnectToGUI(hostname, (unsigned short) portNumber);
            isGuiConnected = true;

            thisArg += 3;
        }
        else if (!isConfigFileSet && argv[thisArg][0] != '-')
        {
            // Config file name
            strncpy(configFileName, argv[thisArg], MAX_STRING_LENGTH);
            configFileName[MAX_STRING_LENGTH - 1] = '\0';
            isConfigFileSet = true;

            thisArg++;
        }
        else
        {
            thisArg++;
        }

    }// End of while (thisArg < argc)

    return (isGuiConnected & isConfigFileSet);
}

/// \brief Function to connect Proxy to master simulator
///
/// \param nodeInput  Pointer to data structure that contains all
///                   configuration information
//
/// \param multigui  Flag indicating whether proxy is launched with
///                   multigui command line option or not
/// \param exataMcAddrStr  IP address of machine running exata
/// \param multiguiPort  Port opened by exata for multigui interface
///
/// \return if sucessfully conencted, return a pointer to socket data structure
///  (EXTERNAL_Socket*), otherwise, return a null pointer
EXTERNAL_Socket* Proxy_ConnectToMultiGuiIntf(NodeInput* nodeInput,
                                            bool multigui,
                                            char* exataMcAddrStr,
                                            int multiguiPort)
{
    EXTERNAL_Socket* socketConn;
    EXTERNAL_SocketErrorType error;


    char exataAddrStr[MAX_STRING_LENGTH] = {0};
    int portNumber = multiguiPort;
    if (multigui)
    {
        strcpy(exataAddrStr, exataMcAddrStr);
    }
    else
    {
        BOOL retVal;

        // Reading simulator host address
        IO_ReadString(ANY_NODEID,
                  ANY_ADDRESS,
                  nodeInput,
                  "EXATA-MACHINE-ADDRESS",
                  &retVal,
                  exataAddrStr);

        if (!retVal)
        {
            // If not specified, take default address i.e. loopback address
            sprintf(exataAddrStr,"127.0.0.1");
        }

        IO_ReadInt( ANY_NODEID,
                    ANY_ADDRESS,
                    nodeInput,
                    "MULTI-GUI-INTERFACE-PORT",
                    &retVal,
                    &portNumber);
        if (retVal)
        {
            // Do nothing, port will be added below
        }
        else
        {
            portNumber = MULTI_GUI_DEFAULT_PORT;
        }
    }

    // Initialize a data socket
    socketConn = (EXTERNAL_Socket*)MEM_malloc(sizeof(EXTERNAL_Socket));
    EXTERNAL_SocketInit(socketConn, FALSE, FALSE);
    error = EXTERNAL_SocketConnect(socketConn,
                            exataAddrStr,
                            portNumber,
                            1000);
    if (error == EXTERNAL_NoSocketError)
    {
        // No error
        return socketConn;
    }

    char errorStr[MAX_STRING_LENGTH*2];
    sprintf(errorStr,
            "Can't connect to EXata machine (IP address %s) on port %d\n",
            exataAddrStr,
            portNumber);
    ERROR_ReportError(errorStr);
    return NULL;
}

/// \brief Function to send data received by proxy from
///        secondary GUI to master simulator
///
/// \param socketConn  Pointer to socket structure for
//                     master simulator
/// \param command  Data from secondary GUI to be sent to
///                 master simulator
void Proxy_SendToMultiGuiIntf(EXTERNAL_Socket* socketConn,
                           GuiCommand command)
{
    EXTERNAL_SocketErrorType error;
    char buffer[BIG_STRING_LENGTH];
    unsigned int numChar;

    // Only allow HITL command and GUI_STEP command
    if ((command.type == GUI_USER_DEFINED ||
        command.type == GUI_STEP)
        && (command.args).size() > 0)
    {
        numChar = sprintf(buffer,
                          "%d %s\n",
                          command.type,
                          (command.args).c_str());

        error = EXTERNAL_SocketSend(socketConn,
                                    buffer,
                                    numChar,
                                    FALSE);

        if (error != EXTERNAL_NoSocketError )
        {
            printf("Can not send data through socket \n");
        }
    }
}

/// \brief Function to send simulator data to secondary GUI
///
/// \param strFromMaster  Pointer to string to be sent
/// \param size  Size of string
void Proxy_SendToSecondaryGui(const char* strFromMaster, size_t size)
{
    std::string outString;
    outString.append(strFromMaster,size);

    const char *outPos = outString.c_str();
    size_t size_left = outString.size();
    while (size_left > 0)
    {
        ssize_t returnValue =
            send(GUI_guiSocket, outPos, size_left, 0);

#ifdef _WIN32
        if (returnValue == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                if (size_left != (int)size)
                {
                    EXTERNAL_Sleep(1 * MICRO_SECOND);
                    continue;
                }
                else
                {
                    return;
                }
            }
            else
            {
                char err[MAX_STRING_LENGTH];
                sprintf(err, "Could not send err = %d", WSAGetLastError());
                ERROR_ReportWarning(err);
                return;
            }
        }
#else /* unix/linux */
        if (returnValue == -1)
        {
            // This send would result in a block.
            // Return EXTERNAL_DataNotSent.
            if (errno == EAGAIN)
            {
                if (size_left != (int)size)
                {
                    EXTERNAL_Sleep(1 * MICRO_SECOND);
                    continue;
                }
                else
                {
                    return;
                }
            }

            ERROR_ReportWarningArgs("Could not send, err = %s",
                strerror(errno));
            return;
        }
#endif
        else
        {
            outPos += returnValue;
            size_left -= returnValue;
        }
    }
}

/// \brief Function to receive master simulator data
///
/// \param socketConn  Pointer to socket structure for master simulator
/// \param time  Time value from master simulation
void Proxy_ReceiveFromMultiGuiIntf(EXTERNAL_Socket* socket, clocktype* time)
{
    char buffer[MAX_STRING_LENGTH*10];
    int lengthRead;
    size_t  lengthProcessed;
    char* lineEnd;
    std::string commandLine;
    bool foundNewline = false;

    bool socketDataAvail;
    EXTERNAL_SocketDataAvailable(socket,  &socketDataAvail);
    memset(buffer, 0, MAX_STRING_LENGTH*10);
    if (socketDataAvail)
    {
        // Try to read from the socket until we get the end of line
        while (!foundNewline)
        {
             memset(buffer, 0, MAX_STRING_LENGTH*10);
            lengthRead = recv(socket->socketFd,      // Connected socket
                              buffer,                // Receive buffer
                              MAX_STRING_LENGTH*10,  // Size of receive buffer
                              MSG_PEEK);             // Flags

            if (lengthRead == 0)
            {
                // close socket
                EXTERNAL_SocketClose(socket);
                return;
            }
#ifdef _WIN32
            if (lengthRead < 0 && WSAGetLastError() == WSAEWOULDBLOCK)
            {
                // Try it again.
                EXTERNAL_Sleep(1*MICRO_SECOND);
                continue;
            }
#else /* unix/linux */
            if (lengthRead < 0 && errno == EAGAIN)
            {
                // Try it again.
                EXTERNAL_Sleep(1*MICRO_SECOND);
                continue;
            }
#endif
            if (lengthRead < 0)
            {
                char err[MAX_STRING_LENGTH];
#ifdef _WIN32
                sprintf(err, "Simulator receive error:%d and length:%d\n", WSAGetLastError(), lengthRead);
#else /* unix/linux */
                sprintf(err, "Simulator receive error:%d and length:%d\n", errno, lengthRead);
#endif
                ERROR_ReportWarning(err);
                EXTERNAL_SocketClose(socket);
                return;
            }

            lineEnd = (char*) memchr(buffer, '\n', lengthRead);

            if (lineEnd != NULL)
            {
                foundNewline = true;
                lengthProcessed = lineEnd - buffer + 1;
                commandLine.append(buffer, lengthProcessed);
            }
            else
            {
                lengthProcessed = lengthRead;

                // append everything read so far
                commandLine.append(buffer, lengthRead);
            }

            // flush processed data from the TCP input buffer
            recv(socket->socketFd, buffer, lengthProcessed, 0);
        }

        char inStr[MAX_STRING_LENGTH*10] = {0};
        unsigned int size;
        char type[MAX_STRING_LENGTH] = {0};
        char eventType[MAX_STRING_LENGTH] = {0};

        size = sprintf(inStr,"%s",commandLine.c_str());
        sscanf(inStr,"%s%s",type, eventType);

        // Temp way to process what we read from the socket
        if (strcmp(type,"Cyber:")==0)
        {
            printf("%s",inStr);
            fflush(stdout);
        }
        else
        {
            GuiReplies guiType = (GuiReplies)atoi(type);
            GuiEvents guiEvent = (GuiEvents)atoi(eventType);
            if (guiType == GUI_STEPPED)
            {
                printf("GUI_STEPPED-%s\n", inStr);
                unsigned int num;
                num = sscanf(inStr,"%s %ld",type, time);
            }
            if (!receivedFirstStepCmd && guiType != GUI_UID)
            {
                return;
            }

            if ((guiType >=1 && guiType <=16) || guiType == GUI_FINISHED)
            {
                std::string cmd;
                cmd.append(inStr,size);
                proxySimInterlock.startSendingDataToGui();
                if ((guiType != GUI_ANIMATION_COMMAND) ||
                    (guiType == GUI_ANIMATION_COMMAND &&
                    (guiEvent == GUI_DEACTIVATE_INTERFACE ||
                    guiEvent == GUI_ACTIVATE_INTERFACE ||
                    guiEvent == GUI_MOVE_NODE||
                    guiEvent == GUI_SET_ORIENTATION ||
                    guiEvent == GUI_ADD_LINK ||
                    guiEvent == GUI_DELETE_LINK ||
                    guiEvent == GUI_RESET_EXTERNAL_NODE ||
                    guiEvent == GUI_SET_EXTERNAL_NODE ||
                    guiEvent == GUI_SET_EXTERNAL_NODE_MAPPING ||
                    guiEvent == GUI_RESET_EXTERNAL_NODE_MAPPING ||
                    guiEvent == GUI_NODE_SIDE_CHANGE)))
                {
                    proxySimInterlock.push_back(cmd);
                }
                else if (!proxySimInterlock.dropData())
                {
                    proxySimInterlock.push_back(cmd);
                }
                else
                {
                    // drop data
                }
            }

            if (guiType == GUI_FINISHED)
            {
                EXTERNAL_SocketClose(socket);
                return;
            }
        }
    } // End of if (socketDataAvail)
}

/// \brief Function to check socket connection validity
///
/// \param socketConn  Pointer to socket structure created
///                    for connecting master simulator
///
/// \return Returns socket connection validity
bool isConnectionValid(EXTERNAL_Socket* socketConn)
{
    bool simulatorDataAvail;
    bool ret = true;
    // Check to propagate the license error to GUI in case of error
    if (EXTERNAL_SocketDataAvailable(socketConn, &simulatorDataAvail) == EXTERNAL_NoSocketError)
    {
        if (simulatorDataAvail)
        {
            char buffer[MAX_STRING_LENGTH];
            int lengthRead = recv(socketConn->socketFd,  // Connected socket
                                  buffer,                // Receive buffer
                                  MAX_STRING_LENGTH,     // Size of receive buffer
                                  MSG_PEEK);
            // True if Simulator closed the connection
            if (lengthRead == 0)
            {
                EXTERNAL_SocketClose(socketConn);
                MEM_free(socketConn);
                socketConn = NULL;
                ret = false;
            }
        }
    }
    else
    {
        EXTERNAL_SocketClose(socketConn);
        MEM_free(socketConn);
        socketConn = NULL;
        ret = false;
    }
    return ret;
}

/// \brief Function to send step command to secondary GUI
void Proxy_SendSteppedCmd()
{
    GuiReply    reply;

    reply.type = GUI_STEPPED;
    reply.args.append("0");
    GUI_SendReply(GUI_guiSocket, reply);
}
