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
#include <signal.h>
#include <stdio.h>
#include <string.h>

// receive buffer size
#define SOCK_MAX_BUFFER_SIZE  4098

#define HUMAN_IN_THE_LOOP_DEMO 1
#define HITL 1

#ifdef _WIN32
#include <winsock.h>

// recv() is supposed to return an ssize_t, but on Windows (even
// 64-bit Windows) ssize_t is missing and recv() returns an int.
typedef int ssize_t;
#else
#include <netdb.h>
#include <unistd.h>
#endif

#include "gui.h"
#include "partition.h"

#define DEBUG             0

//#define SOCKET unsigned int

#include <sys/types.h>
#include <time.h>

unsigned int GUI_guiSocket       = 0;
BOOL         GUI_guiSocketOpened = FALSE;

#ifdef _WIN32

/*------------------------------------------------------
 * GUI_ConnectToGUI
 *------------------------------------------------------*/
void GUI_ConnectToGUI(char*          hostname,
                      unsigned short portnum) {
    LPHOSTENT   lpHostEntry;
    SOCKADDR_IN saServer;
    SOCKET_HANDLE      theSocket;
    WORD        wVersionRequested = MAKEWORD(1,1);
    WSADATA     wsaData;
    int         returnValue;

    //
    // Initialize WinSock and check the version
    //
    returnValue = WSAStartup(wVersionRequested, &wsaData);
    if (wsaData.wVersion != wVersionRequested) {
        ERROR_ReportWarning("\n Wrong version\n");
        GUI_guiSocketOpened = FALSE;
        return;
    }

    //
    // Find the server
    //
    lpHostEntry = gethostbyname(hostname);
    if (lpHostEntry == NULL) {
        ERROR_ReportWarning("gethostbyname()");
        GUI_guiSocketOpened = FALSE;
        return;
    }

    //
    // Create a TCP/IP datagram socket
    //
    theSocket = (SOCKET_HANDLE)socket(AF_INET,          // Address family
                                      SOCK_STREAM,      // Socket type
                                      IPPROTO_TCP);     // Protocol
    if (theSocket == INVALID_SOCKET) {
        ERROR_ReportWarning("failed to create the socket\n");
        GUI_guiSocketOpened = FALSE;
        return;
    }

    //
    // Fill in the address structure for the server
    //
    saServer.sin_family = AF_INET;
    saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);

    // ^ Server's address
    saServer.sin_port = htons(portnum);

    returnValue = connect(theSocket,
                          (LPSOCKADDR)&saServer,
                          sizeof(struct sockaddr));
    if (returnValue != SOCKET_ERROR) {
        GUI_guiSocket       = theSocket;
        GUI_guiSocketOpened = TRUE;
        return;
    }
    else {
        ERROR_ReportWarning("failed to connect to the socket\n");
        GUI_guiSocketOpened = FALSE;
        return;
    }
}

/*------------------------------------------------------
 * GUI_DisconnectFromGUI
 *------------------------------------------------------*/
void GUI_DisconnectFromGUI(SOCKET_HANDLE socket, bool sendFinishedReply) {
    if (socket != 0 && GUI_guiSocketOpened) {
        if (sendFinishedReply) {
            std::string replyText = " ";
            GUI_SendReply(socket, GUI_CreateReply(GUI_FINISHED, &replyText));
        }

        if (DEBUG) {
            printf ("windows gui disconnect sent GUI_FINISHED\n");
            fflush(stdout);
        }

        // Allow one second to elapse until the socket is closed
        EXTERNAL_Sleep(1 * SECOND);
        GUI_guiSocketOpened = FALSE;
        closesocket(socket);

        //
        // Release WinSock
        //
        WSACleanup();
    }
}

#else // unix

/*------------------------------------------------------
 * GUI_ConnectToGUI
 *------------------------------------------------------*/
void GUI_ConnectToGUI(char*          hostname,
                      unsigned short portnum) {
    struct sockaddr_in sa;
    struct hostent*    hp;

    int localSocket;

    if ((hp = gethostbyname(hostname)) == NULL) { /* do we know the host's */
        errno = ECONNREFUSED;                       /* address? */
        printf("couldn't get hostinfo\n");
        return;
    }

    memset(&sa,0,sizeof(sa));
    memcpy((char *)&sa.sin_addr,hp->h_addr,hp->h_length);     /* set address */
    sa.sin_family= hp->h_addrtype;
    sa.sin_port= htons((unsigned short)portnum);

    if ((localSocket = socket(hp->h_addrtype,SOCK_STREAM,0)) < 0) {
        printf("couldn't create socket\n");
        GUI_guiSocketOpened = FALSE;
        return;
    }

    if (connect(localSocket,(struct sockaddr *)&sa, sizeof(sa)) < 0) {
        printf("couldn't connect to server\n");
        close(localSocket);
        GUI_guiSocketOpened = FALSE;
        return;
    }

    // If the GUI crashes, the stdout pipe will be broken.  Make sure we
    // can still shut down cleanly if this happens.
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &sa, NULL);
    }

    GUI_guiSocket       = localSocket;
    GUI_guiSocketOpened = TRUE;
}

/*------------------------------------------------------
 * GUI_DisconnectFromGUI
 *------------------------------------------------------*/
void GUI_DisconnectFromGUI(SOCKET_HANDLE socket, bool sendFinishedReply) {
    if (socket != 0 && GUI_guiSocketOpened) {
        if (sendFinishedReply) {
            std::string replyText = " ";
            GUI_SendReply(socket, GUI_CreateReply(GUI_FINISHED, &replyText));
        }

        if (DEBUG) {
            printf ("unix gui disconnect sent GUI_FINISHED\n");
            fflush(stdout);
        }

        // Allow one second to elapse until the socket is closed
        EXTERNAL_Sleep(1 * SECOND);

        GUI_guiSocketOpened = FALSE;
        close(socket);
    }
}
#endif

/*------------------------------------------------------
 * GUI_ReceiveCommand
 *------------------------------------------------------*/
GuiCommand GUI_ReceiveCommand(SOCKET_HANDLE socket) {
    char buffer[SOCK_MAX_BUFFER_SIZE];
    ssize_t lengthRead;
    size_t  lengthProcessed;
    GuiCommand command;
    char* lineEnd;
    std::string commandLine;
    bool foundNewline = false;

    while (! foundNewline)
    {
        lengthRead = recv(socket,                // Connected socket
                          buffer,                // Receive buffer
                          SOCK_MAX_BUFFER_SIZE,  // Size of receive buffer
                          MSG_PEEK);             // Flags
        if (lengthRead < 0 && errno == EINTR)
        {
            // Try it again.
            continue;
        }
        else if (lengthRead <= 0)
        {
            // Either the socket was closed prematurely (maybe because the
            // GUI crashed) or an unrecognized error occurred -- shut down
            // the simulator.
            GUI_DisconnectFromGUI(socket, false);
            command.type = GUI_STOP;
            return command;
        }

        lineEnd = (char*) memchr(buffer, '\n', lengthRead);
        if (lineEnd != NULL)
        {
            foundNewline = true;
            lengthProcessed = lineEnd - buffer + 1;

            // append everything except the final newline
            commandLine.append(buffer, lineEnd - buffer);
        }
        else
        {
            lengthProcessed = lengthRead;

            // append everything read so far
            commandLine.append(buffer, lengthRead);
        }

        // flush processed data from the TCP input buffer
        recv(socket, buffer, (int)lengthProcessed, 0);
    }

    if (DEBUG)
    {
        printf("Received from GUI: %s\n", commandLine.c_str());
        fflush(stdout);
    }

    // possibly remove \r sent by Windows GUI from end of line
    if (commandLine.length() > 0 &&
        commandLine[commandLine.length() - 1] == '\r')
    {
        commandLine.erase(commandLine.length() - 1);
    }

    if (commandLine.length() < 1)
    {
        // this is an error
        command.type = GUI_UNRECOGNIZED;
    }
    else
    {
        command.type = (GuiCommands) atoi(commandLine.c_str());
        std::string::size_type space_pos = commandLine.find(' ');
        if (space_pos != std::string::npos)
        {
            command.args = commandLine.substr(space_pos + 1);
        }
    }
    return command;
}

/*------------------------------------------------------
 * GUI_SendReply
 *------------------------------------------------------*/
void GUI_SendReply(SOCKET_HANDLE   socket,
                   GuiReply reply) {
    char buffer[GUI_MAX_COMMAND_LENGTH + 10];
    std::string outString;

    sprintf(buffer, "%d ", reply.type);
    outString.append(buffer);
    outString.append(reply.args);
    outString.append("\n");

    const char *outPos = outString.c_str();
    size_t size_left = outString.size();
    while (size_left > 0)
    {
        ssize_t returnValue =
            send(socket, outPos, (int)size_left, 0);
        if (returnValue < 0 && errno == EINTR)
        {
            // try it again
            continue;
        }
        else if (returnValue < 0)
        {
            // unknown error, shut down the simulation gracefully
            PARTITION_RequestEndSimulation();
            return;
        }
        else
        {
            outPos += returnValue;
            size_left -= returnValue;
        }
    }
    if (DEBUG)
    {
        printf("sent over socket: %s\n", outString.c_str());
        fflush(stdout);
    }
}

/*------------------------------------------------------
 * FUNCTION     GUI_CreateReply
 * PURPOSE      Function used to replace newline characters in a string being
 *              sent to the GUI.
 *
 * Parameters:
 *    replyType:  the type of reply
 *    msg:        the reply message
 *------------------------------------------------------*/
GuiReply GUI_CreateReply(GuiReplies replyType,
                         std::string* msgString) {

    GuiReply reply;
    int      m = 0;
    const char* msg = msgString->c_str();

    reply.type = replyType;

    // replace '\n' with "<p>" since the GUI uses '\n' to signify
    // the end of a message.

    while (msg[m] != '\0') {
        if (msg[m] == '\n') {
            reply.args.append("<br>");
        }
        else {
            reply.args += msg[m];
        }
        m++;
    }
    return reply;
}
