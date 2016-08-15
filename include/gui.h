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

/// \defgroup Package_GUI GUI

/// \file
/// \ingroup Package_GUI
/// 
/// This file describes data structures and functions for
/// interfacing with the QualNet GUI and the other
/// graphical tools.

#ifndef GUI_H
#define GUI_H

#include <string>

#include "main.h"
#include "coordinates.h"
#include "external.h"
#include "dynamic.h"

/*------------------------------------------------------------------
 * Types to be used for animation.  These are used by protocol
 * developers to add animation to their protocols.
 *------------------------------------------------------------------*/

#ifdef _WIN32
    #define SOCKET_HANDLE unsigned int      // The socket
#else /* unix/linux */
   #define SOCKET_HANDLE unsigned int      // The socket
#endif


/// Layer in protocol stack.  Allows animation filtering.
enum GuiLayers {
    GUI_MOBILITY_LAYER,
    GUI_CHANNEL_LAYER,
    GUI_PHY_LAYER,
    GUI_MAC_LAYER,
    GUI_NETWORK_LAYER,
    GUI_TRANSPORT_LAYER,
    GUI_APP_LAYER,
    GUI_ROUTING_LAYER,
    GUI_ANY_LAYER
};

/// Semantic events to be animated.
enum GuiEvents {
    GUI_INITIALIZE                = 0,
    GUI_INIT_NODE                 = 1,
    GUI_MOVE_NODE                 = 2,
    GUI_SET_NODE_TYPE             = 3,
    GUI_ADD_LINK                  = 4,
    GUI_DELETE_LINK               = 5,
    GUI_BROADCAST                 = 6,
    GUI_MULTICAST                 = 7,
    GUI_UNICAST                   = 8,
    GUI_RECEIVE                   = 9,
    GUI_DROP                      = 10,
    GUI_COLLISION                 = 11,
    GUI_CREATE_SUBNET             = 12,
    GUI_ADD_APP                   = 13,
    GUI_DELETE_APP                = 14,
    GUI_CREATE_HIERARCHY          = 15,
    GUI_SET_ORIENTATION           = 16,
    GUI_SET_NODE_ICON             = 17,
    GUI_SET_NODE_LABEL            = 18,
    GUI_SET_NODE_RANGE            = 19,
    GUI_QUEUE_CREATE              = 20,
    GUI_QUEUE_ADD                 = 21,
    GUI_QUEUE_REMOVE              = 22,
    GUI_QUEUE_DROP                = 23,
    GUI_SET_PATTERN               = 24,
    GUI_ENDBROADCAST              = 25,
    GUI_SET_PATTERNANGLE          = 26,
    GUI_SET_INTERFACE_ADDRESS     = 27,
    GUI_SET_SUBNET_MASK           = 28,
    GUI_SET_INTERFACE_NAME        = 29,
    GUI_SET_PATHLOSS_DATA         = 30,
    GUI_SET_DISTANCE_DATA         = 31,
    GUI_SET_MAPPING_NODE_IDS      = 32,
    GUI_SET_NUMNODES              = 33,
    GUI_PATHLOSS_FILE_INITIALIZED = 34,
    GUI_INIT_WIRELESS             = 35,
    GUI_CREATE_WEATHER_PATTERN    = 36,
    GUI_MOVE_WEATHER_PATTERN      = 37,
    GUI_MOVE_HIERARCHY            = 38,
    GUI_DYNAMIC_AddObject         = 39, // DYNAMIC_API
    GUI_DYNAMIC_RemoveObject      = 40, // DYNAMIC_API
    GUI_DYNAMIC_AddLink           = 41, // DYNAMIC_API
    GUI_DYNAMIC_ObjectValue       = 42, // DYNAMIC_API
    GUI_DYNAMIC_ExecuteResult     = 43, // DYNAMIC_API
    GUI_DYNAMIC_ObjectPermissions = 44, // DYNAMIC_API
    GUI_VISUALIZATION_OBJECT      = 46,
    GUI_ACTIVATE_INTERFACE        = 47, // activate interface
    GUI_DEACTIVATE_INTERFACE      = 48, // deactivate interface
    // EXata specific events are in range 50-100
    GUI_SET_EXTERNAL_NODE         = 50,
    GUI_RESET_EXTERNAL_NODE       = 51,
    GUI_SET_REALTIME_INDICATOR_STATUS = 52,
    GUI_ADDRESS_CHANGE            = 53,
    GUI_NODE_SIDE_CHANGE          = 54, // reserved for use by NDT
    GUI_SET_EXTERNAL_NODE_MAPPING         = 55,
    GUI_RESET_EXTERNAL_NODE_MAPPING       = 56,
    // The maximum number of GUI events.  This enum value must
    // always be last.
    GUI_MAX_EVENTS
};

/// Statistics events recognized by Animator.
enum GuiStatisticsEvents {
    GUI_DEFINE_METRIC   = 0,
    GUI_SEND_REAL       = 1,
    GUI_SEND_INTEGER    = 2,
    GUI_SEND_UNSIGNED   = 3
};

/// Types of statistical metrics.
enum GuiMetrics {
    GUI_CUMULATIVE_METRIC = 0,
    GUI_AVERAGE_METRIC    = 1
};

/// The numeric data types supported for dynamic statistics.
enum GuiDataTypes {
    GUI_INTEGER_TYPE   = 0,
    GUI_DOUBLE_TYPE    = 1,
    GUI_UNSIGNED_TYPE  = 2
};

/// Animation effects that can be assigned to an event.
enum GuiEffects {
    GUI_DEFAULT_EFFECT     = 0,
    GUI_NO_EFFECT          = 1,
    GUI_CHANGE_ICON        = 2,
    GUI_CHANGE_POSITION    = 3,
    GUI_CHANGE_NAME        = 4,
    GUI_CHANGE_ORIENTATION = 5,
    GUI_DRAW_ARROW         = 6,
    GUI_ERASE_ARROW        = 7,
    GUI_FLASH_ARROW        = 8,
    GUI_DRAW_LINK          = 9,
    GUI_ERASE_LINK         = 10,
    GUI_FLASH_LINK         = 11,
    GUI_OMNI_BROADCAST     = 12,
    GUI_DIRECTED_BROADCAST = 13,
    GUI_DRAW_SUBNET        = 14,
    GUI_WIRED_BROADCAST    = 15,
    GUI_FLASH_X            = 16,
    GUI_CIRCLE_NODE        = 17,
    GUI_SET_COLOR          = 18,
    GUI_ADD_LABEL          = 19
};

/// Colors that can be assigned to Animator effects.
enum GuiColors {
    GUI_BLACK      = 0,
    GUI_BLUE       = 1,
    GUI_CYAN       = 2,
    GUI_DARK_GRAY  = 3,
    GUI_GRAY       = 4,
    GUI_GREEN      = 5,
    GUI_LIGHT_GRAY = 6,
    GUI_MAGENTA    = 7,
    GUI_ORANGE     = 8,
    GUI_PINK       = 9,
    GUI_RED        = 10,
    GUI_WHITE      = 11,
    GUI_YELLOW     = 12,
    GUI_RECEIVE_OTHER = 13
};

/// Types of subnets recognized by the Animator.
enum GuiSubnetTypes {
    GUI_WIRED_SUBNET      = 0,
    GUI_WIRELESS_SUBNET   = 1,
    GUI_SATELLITE_NETWORK = 2
};

/// Commands for displaying visualization objects
enum GuiVisObjCommands {
    GUI_ADD_FILTER        = 0,
    GUI_DRAW_LINE         = 1,
    GUI_DRAW_SHAPE        = 2,
    GUI_DRAW_TEXT         = 3,
    GUI_DELETE_OBJECTS    = 4,
    GUI_DRAW_FLOW_LINE    = 5,
    GUI_CLEAR_DETAILS     = 6,
    GUI_ADD_SHAPE_LEGEND  = 7,
    GUI_ADD_LINE_LEGEND   = 8
};

/// Shape selections for GUI_DRAW_SHAPE command
enum GuiVisShapes {
    GUI_SHAPE_CIRCLE      = 0,
    GUI_SHAPE_TRIANGLE    = 1,
    GUI_SHAPE_SQUARE      = 2
};

/// Selection for the method of sending properties
enum SopsProtocol
{
    SOPS_PROTOCOL_default,
    SOPS_PROTOCOL_TCP,
    SOPS_PROTOCOL_UDP,
    SOPS_PROTOCOL_Max
};

#include "metricData.h"

/// 
/// The default interval before waiting for the Animator handshake/STEP.
#define GUI_DEFAULT_STEP 1000000000

/// Icon used in case none is specified for a node.
#define GUI_DEFAULT_ICON "<no-icon>"

/// By default, there are 8 layers, but users may add more
#define MAX_LAYERS  12

/*------------------------------------------------------------------
 * The type fields of the various animation API functions are used
 * to distinguish between different types of signals/nodes/links.
 * For example, the user may wish to distinguish between RTS and CTS
 * signals.  The default value for these calls is 0.  The user should
 * define additional values beginning with 1.
 *------------------------------------------------------------------*/

/// Default value to use for data types.
#define GUI_DEFAULT_DATA_TYPE 0

/// Default value to use for data types.
#define GUI_EMULATION_DATA_TYPE 1

/// Default value to use for link types.
#define GUI_DEFAULT_LINK_TYPE 0

/// Default value to use for node types.
#define GUI_DEFAULT_NODE_TYPE 0

/// Default interface for GUI commands.
#define GUI_DEFAULT_INTERFACE 0

/// Used to distinguish wireless and wired links.
#define GUI_WIRELESS_LINK_TYPE 1

/// Used to distinguish ATM links from other types.
#define GUI_ATM_LINK_TYPE 2

/// Used by Stats Manager
#define GUI_COVERAGE_LINK_TYPE 3

/*------------------------------------------------------------------
 * GUI functions.
 *------------------------------------------------------------------*/
/// 
/// Called from GUI_EXTERNAL_ReceiveCommand() if command type
/// is GUI_USER_DEFINED. Created so that GUI Human In the loop 
/// commands can also be given through a file, instead of giving it 
/// through the GUI. Will serve for good unit testing of GUI HITL commands
///
/// \param args  the command itself
/// \param partition  the partition pointer
///
void GUI_HandleHITLInput(const char* args, PartitionData* partition);

/// 
/// Initializes the GUI in order to start the animation.
/// The terrain map should give the path (either absolute, or
/// relative to QUALNET_HOME) of an file to represent the
/// terrain.
///
/// \param nodeInput  configuration file
/// \param numNodes  the number of nodes in the simulation
/// \param coordinateSystem  LATLONALT or CARTESIAN
/// \param origin  Southwest corner
/// \param dimensions  Northeast corner, or size
/// \param maxClock  length of the simulation
///
extern void GUI_Initialize(NodeInput*  nodeInput,
                           int         numNodes,
                           int         coordinateSystem,
                           Coordinates origin,
                           Coordinates dimensions,
                           clocktype   maxClock);
/// 
/// This function will allow the protocol designer to specify the
/// effect to use to display certain events.
///
/// \param event  the type of event for the new effect
/// \param layer  the protocol layer
/// \param type  special key to distinguish similar events
/// \param effect  the effect to use
/// \param color  optional color for the effect
///
extern void GUI_SetEffect(GuiEvents  event,
                          GuiLayers  layer,
                          int        type,
                          GuiEffects effect,
                          GuiColors  color);

/// 
/// Provides the initial location and orientation of the node, the
/// transmission range (for wireless nodes), a node type, and optional
/// icon and label.
///
/// \param node  the node
/// \param nodeInput  configuration file
/// \param time  the current simulation time
///
extern void GUI_InitNode(Node*      node,
                         NodeInput* nodeInput,
                         clocktype  time);

/// 
/// Provides the initial location and orientation of the node, the
/// transmission range (for wireless nodes), a node type, and optional
/// icon and label.
///
/// \param node  the node
/// \param interfaceIndex  the interface to initialize
///
extern void GUI_InitWirelessInterface(Node* node,
                                      int interfaceIndex);

/// 
/// Sets the IP address associated with one of the node's interfaces.
///
/// \param nodeInput  configuration file
///
extern void GUI_InitializeInterfaces(const NodeInput* nodeInput);

/// 
/// Sets the IP address associated with one of the node's interfaces.
///
/// \param nodeID  the node's ID
/// \param interfaceAddress  new IP address
/// \param interfaceIndex  interface Address to change
/// \param time  the current simulation time
///
extern void GUI_SetInterfaceAddress(NodeId      nodeID,
                                    NodeAddress interfaceAddress,
                                    int         interfaceIndex,
                                    clocktype   time);

/// 
/// Sets the Subnet mask associated with one of the node's interfaces.
///
/// \param nodeID  the node's ID
/// \param interfaceAddress  new Subnet mask
/// \param interfaceIndex  Subnet mask to change
/// \param time  the current simulation time
///
extern void GUI_SetSubnetMask(NodeId      nodeID,
                              NodeAddress subnetMask,
                              int         interfaceIndex,
                              clocktype   time);
/// 
/// Sets the Interface name associated with one of the node's interfaces.
///
/// \param nodeID  the node's ID
/// \param interfaceAddress  new Interface name
/// \param interfaceIndex  interface Name to change
/// \param time  the current simulation time
///
extern void GUI_SetInterfaceName(NodeId      nodeID,
                                 char*       interfaceName,
                                 int         interfaceIndex,
                                 clocktype   time);

/// Moves the node to a new position.
///
/// \param nodeID  the node's ID
/// \param position  the new position
/// \param time  the current simulation time
///
extern void GUI_MoveNode(NodeId      nodeID,
                         Coordinates position,
                         clocktype   time);

/// Changes the orientation of a node.
///
/// \param nodeID  the node's ID
/// \param orientation  the new orientation
/// \param time  the current simulation time
///
extern void GUI_SetNodeOrientation(NodeId      nodeID,
                                   Orientation orientation,
                                   clocktype   time);

/// Changes the icon associated with a node.
///
/// \param nodeID  the node's ID
/// \param iconFile  the path to the image file, may be the
///    absolute path or relative to QUALNET_HOME
/// \param time  the current simulation time
///
extern void GUI_SetNodeIcon(NodeId      nodeID,
                            const char*       iconFile,
                            clocktype   time);

extern void GUI_SetExternalNode(NodeId      nodeID,
                            int type,
                            clocktype   time);

extern void GUI_ResetExternalNode(NodeId      nodeID,
                            int type,
                            clocktype   time);

/// \brief Function used to send create mapping event with virtual and 
/// physical ip address info to the connected guis.
///
/// \param nodeID  the node's ID
/// \param virtualAddr  IP address of the simulation node to be mapped
/// \param physicalAddr IP address of the real machine to be mapped
/// \param time Current simulation time
extern void GUI_SetExternalNodeMapping(NodeId      nodeID,
                            char* virtualAddr,
                            char* physicalAddr,
                            clocktype   time);

/// \brief Function used to send delete mapping event with virtual and 
/// physical ip address info to the connected guis.
///
/// \param nodeID  the node's ID
/// \param virtualAddr  IP address of the simulation node to be mapped
/// \param physicalAddr IP address of the real machine to be mapped
/// \param time Current simulation time
extern void GUI_ResetExternalNodeMapping(NodeId      nodeID,
                            char* virtualAddr,
                            char* physicalAddr,
                            clocktype   time);

/// Changes the label (the node name) of a node.
///
/// \param nodeID  the node's ID
/// \param label  a string to label the node
/// \param time  the current simulation time
///
extern void GUI_SetNodeLabel(NodeId      nodeID,
                             char*       label,
                             clocktype   time);

/// Changes the transmission range of a node
///
/// \param nodeID  the node's ID
/// \param interfaceIndex  which of the node's interfaces to use
/// \param range  the new transmission range in meters
/// \param time  the current simulation time
///
extern void GUI_SetNodeRange(NodeId      nodeID,
                             int         interfaceIndex,
                             double      range,
                             clocktype   time);

/// Changes the (symbolic) type of a node
///
/// \param nodeID  the node's ID
/// \param type  user defined type, used with GUI_SetEffect
/// \param time  the current simulation time
///
extern void GUI_SetNodeType(NodeId      nodeID,
                            int         type,
                            clocktype   time);

/// 
/// Sets the antenna pattern to one of a previously specified
/// antenna pattern file.
///
/// \param node  the node pointer
/// \param interfaceIndex  which of the node's interfaces to use
/// \param index  index into the node's antenna pattern file
/// \param time  the current simulation time
///
extern void GUI_SetPatternIndex(Node*      node,
                                int         interfaceIndex,
                                int         index,
                                clocktype   time);

/// 
/// For steerable antennas, it sets the pattern to use, and also
/// an angle relative to the node's current orientation.
///
/// \param node*  the node pointer
/// \param interfaceIndex  which of the node's interfaces to use
/// \param index  index into the node's antenna pattern file
/// \param angleInDegrees  angle to rotate the pattern
/// \param time  the current simulation time
///
extern void GUI_SetPatternAndAngle(Node*    node,
                                   int       interfaceIndex,
                                   int       index,
                                   int       angleInDegrees,
                                   int       elevationAngle,
                                   clocktype time);

/// 
/// Adds a link (one hop on a route) between two nodes.  In a wired
/// topology, this could be a static route; in wireless, a dynamic one.
///
/// \param sourceID  the source node for the link
/// \param destID  the destination node
/// \param layer  the protocol layer associated w/ the link
/// \param type  a user-defined type for the link
/// \param subnetAddress  subnet address for network links
/// \param numHostBits  subnet size for network links
/// \param time  the current simulation time
///
extern void GUI_AddLink(NodeId      sourceID,
                        NodeId      destID,
                        GuiLayers   layer,
                        int         type,
                        NodeAddress subnetAddress,
                        int         numHostBits,
                        clocktype   time);

/// 
/// Adds an IPv6 link (one hop on a route) between two nodes.  In a wired
/// topology, this could be a static route; in wireless, a dynamic one.
///
/// \param sourceID  the source node for the link
/// \param destID  the destination node
/// \param layer  the protocol layer associated w/ the link
/// \param type  a user-defined type for the link
/// \param tla  TLA field of IPv6 address
/// \param nla  NLA field of IPv6 address
/// \param sla  SLA field of IPv6 address
/// \param time  the current simulation time
///
extern void GUI_AddLink(NodeId    sourceID,
                        NodeId    destID,
                        GuiLayers layer,
                        int       type,
                        int       tla,
                        int       nla,
                        int       sla,
                        clocktype time);

/// 
/// Adds an IPv6 link (one hop on a route) between two nodes.  In a wired
/// topology, this could be a static route; in wireless, a dynamic one.
///
/// \param sourceID  the source node for the link
/// \param destID  the destination node
/// \param layer  the protocol layer associated w/ the link
/// \param type  a user-defined type for the link
/// \param ip6_addr  IPv6 address
/// \param unsigned int  IPv6 address prefix length
/// \param time  the current simulation time
///
extern void GUI_AddLink(NodeId       sourceID,
                        NodeId       destID,
                        GuiLayers    layer,
                        int          type,
                        char*        ip6Addr,
                        unsigned int IPv6subnetPrefixLen,
                        clocktype time);

/// Removes link of a specific type.
///
/// \param sourceID  the source node for the link
/// \param destID  the destination node
/// \param layer  the protocol layer associated w/ the link
/// \param type  type of link being deleted
/// \param time  the current simulation time
///
void GUI_DeleteLink(NodeId      sourceID,
                    NodeId      destID,
                    GuiLayers   layer,
                    int         type,
                    clocktype   time);

/// Removes the aforementioned link, no matter the "type."
///
/// \param sourceID  the source node for the link
/// \param destID  the destination node
/// \param layer  the protocol layer associated w/ the link
/// \param time  the current simulation time
///
extern void GUI_DeleteLink(NodeId      sourceID,
                           NodeId      destID,
                           GuiLayers   layer,
                           clocktype   time);

/// Indicates a broadcast.
///
/// \param nodeID  the node's ID
/// \param layer  the protocol layer associated w/ event
/// \param type  a user-defined type for the link
/// \param interfaceIndex  which of the node's interfaces to use
/// \param time  the current simulation time
///
extern void GUI_Broadcast(NodeId      nodeID,
                          GuiLayers   layer,
                          int         type,
                          int         interfaceIndex,
                          clocktype   time);

/// Indicates the end of a broadcast.
///
/// \param nodeID  the node's ID
/// \param layer  the protocol layer associated w/ event
/// \param type  a user-defined type for the link
/// \param interfaceIndex  which of the node's interfaces to use
/// \param time  the current simulation time
///
extern void GUI_EndBroadcast(NodeId      nodeID,
                             GuiLayers   layer,
                             int         type,
                             int         interfaceIndex,
                             clocktype   time);

/// 
/// Indicates a multicast. (Probably need to add a destination address.)
///
/// \param nodeID  the node's ID
/// \param layer  the protocol layer associated w/ event
/// \param type  a user-defined type for the link
/// \param interfaceIndex  which of the node's interfaces to use
/// \param time  the current simulation time
///
extern void GUI_Multicast(NodeId      nodeID,
                          GuiLayers   layer,
                          int         type,
                          int         interfaceIndex,
                          clocktype   time);

/// 
/// Sends a unicast packet/frame/signal to a destination.
/// Will probably be drawn as a temporary line between source and
/// destination, followed by a signal (at the receiver) indicating
/// success or failure.
///
/// \param sourceID  the source node
/// \param destID  the destination node
/// \param layer  protocol layer associated w/ the event
/// \param type  a user-defined type
/// \param sendingInterfaceIndex  sender's interface to use
/// \param receivingInterfaceIndex  receiver's interface to use
/// \param time  the current simulation time
///
extern void GUI_Unicast(NodeId      sourceID,
                        NodeId      destID,
                        GuiLayers   layer,
                        int         type,
                        int         sendingInterfaceIndex,
                        int         receivingInterfaceIndex,
                        clocktype   time);

/// Shows a successful receipt at a destination.
///
/// \param sourceID  the source node
/// \param destID  the destination node
/// \param layer  protocol layer associated w/ the event
/// \param type  a user-defined type
/// \param sendingInterfaceIndex  sender's interface to use
/// \param receivingInterfaceIndex  receiver's interface to use
/// \param time  the current simulation time
///
extern void GUI_Receive(NodeId      sourceID,
                        NodeId      destID,
                        GuiLayers   layer,
                        int         type,
                        int         sendingInterfaceIndex  ,
                        int         receivingInterfaceIndex,
                        clocktype   time);

/// Shows a packet/frame/signal being dropped by a node.
///
/// \param sourceID  the source node
/// \param destID  the destination node
/// \param layer  protocol layer associated w/ the event
/// \param type  a user-defined type
/// \param sendingInterfaceIndex  sender's interface to use
/// \param receivingInterfaceIndex  receiver's interface to use
/// \param time  the current simulation time
///
extern void GUI_Drop(NodeId      sourceID,
                     NodeId      destID,
                     GuiLayers   layer,
                     int         type,
                     int         sendingInterfaceIndex  ,
                     int         receivingInterfaceIndex,
                     clocktype   time);

/// Shows a node detecting a collision.
///
/// \param nodeID  the node's ID
/// \param layer  the protocol layer associated w/ event
/// \param time  the current simulation time
///
extern void GUI_Collision(NodeId      nodeID,
                          GuiLayers   layer,
                          clocktype   time);

/// Creates a subnet.  Normally done at startup.
///
/// \param type  GUI_WIRED/WIRELESS/SATELLITE_NETWORK
/// \param subnetAddress  base address for the subnet
/// \param numHostBits  number of host bits for subnet mask
/// \param nodeList  the rest of the .config file SUBNET line
/// \param time  the current simulation time
///
extern void GUI_CreateSubnet(GuiSubnetTypes type,
                             NodeAddress    subnetAddress,
                             int            numHostBits,
                             const char*    nodeList,
                             clocktype      time);

/// Creates a IPv6 subnet.  Normally done at startup.
///
/// \param type  GUI_WIRED/WIRELESS/SATELLITE_NETWORK
/// \param IPv6subnetAddress  base address for the subnet
/// \param IPv6subnetPrefixLen  number of network bits present
/// \param nodeList  the rest of the .config file SUBNET line
/// \param time  the current simulation time
///
void GUI_CreateSubnet(GuiSubnetTypes type,
                      char* ip6Addr,
                      unsigned int   IPv6subnetPrefixLen,
                      const char*    nodeList,
                      clocktype      time);

/// Creates a IPv6 subnet.  Normally done at startup.
///
/// \param type  GUI_WIRED/WIRELESS/SATELLITE_NETWORK
/// \param ip6_addr  IPv6 address
/// \param unsigned int  IPv6 address prefix length
/// \param nodeList  the rest of the .config file SUBNET line
/// \param time  the current simulation time
///
void GUI_CreateSubnet(GuiSubnetTypes type,
                      in6_addr       IPv6subnetAddress,
                      unsigned int   IPv6subnetPrefixLen,
                      const char*    nodeList,
                      clocktype      time);


/// 
/// Since the GUI supports hierarchical design, this function informs
/// the GUI of the contents of a hierarchical component.
///
/// \param componentID  an identifier for the hierarchy
/// \param nodeList  the rest of the .config file COMPONENT line
///
extern void GUI_CreateHierarchy(int   componentID,
                                char* nodeList);

/// Moves the center point of a hierarchy to a new position.
///
/// \param hierarchyId  the hierarchy's ID
/// \param centerCoordinates  the new position
/// \param orientation  the new orientation
/// \param time  the current simulation time
///
extern void GUI_MoveHierarchy(int         hierarchyID,
                              Coordinates centerCoordinates,
                              Orientation orientation,
                              clocktype   time);

/// 
/// Sends the input line describing a weather pattern to the GUI.
///
/// \param patternID  the weather pattern ID
/// \param inputLine  the .weather file line
///
extern void GUI_CreateWeatherPattern(int   patternID,
                                     char* inputLine);


/// Moves the first point of a weather pattern to a new position.
///
/// \param patternID  the weather pattern ID
/// \param coordinates  the new position
/// \param time  the current simulation time
///
extern void GUI_MoveWeatherPattern(int         patternID,
                                   Coordinates coordinates,
                                   clocktype   time);

/// 
/// Shows label beside the client and the server as app link is setup.
///
/// \param sourceID  the source node
/// \param destID  the destination node
/// \param appName  the application name, e.g. "CBR"
/// \param uniqueId  unique label for this application session
/// \param time  the current simulation time
///
extern void GUI_AddApplication(NodeId      sourceID,
                               NodeId      destID,
                               char*       appName,
                               int         uniqueId,
                               clocktype   time);

/// Deletes the labels shown by AddApplication.
///
/// \param sourceID  the source node
/// \param destID  the destination node
/// \param appName  the application name, e.g. "CBR"
/// \param uniqueId  unique label for this application session
/// \param time  the current simulation time
///
extern void GUI_DeleteApplication(NodeId      sourceID,
                                  NodeId      destID,
                                  char*       appName,
                                  int         uniqueId,
                                  clocktype   time);

/// Creates a queue for a node, interface and priority.
///
/// \param nodeID  the node's ID
/// \param layer  protocol layer associated w/ the event
/// \param interfaceIndex  associated interface of node
/// \param priority  priority of queue
/// \param queueSize  maximum size in bytes
/// \param time  the current simulation time
///
extern void GUI_AddInterfaceQueue(NodeId       nodeID,
                                  GuiLayers    layer,
                                  int          interfaceIndex,
                                  unsigned int priority,
                                  int          queueSize,
                                  clocktype    currentTime);

/// 
/// Inserting one packet to a queue for a node, interface and priority
///
/// \param nodeID  the node's ID
/// \param layer  protocol layer associated w/ the event
/// \param interfaceIndex  associated interface of node
/// \param priority  priority of queue
/// \param packetSize  size of packet
/// \param time  the current simulation time
///
extern void GUI_QueueInsertPacket(NodeId       nodeID,
                                  GuiLayers    layer,
                                  int          interfaceIndex,
                                  unsigned int priority,
                                  int          packetSize,
                                  clocktype    currentTime);

/// 
/// Dropping one packet from a queue for a node, interface and priority.
///
/// \param nodeID  the node's ID
/// \param layer  protocol layer associated w/ the event
/// \param interfaceIndex  associated interface of node
/// \param priority  priority of queue
/// \param time  the current simulation time
///
extern void GUI_QueueDropPacket(NodeId       nodeID,
                                GuiLayers    layer,
                                int          interfaceIndex,
                                unsigned int priority,
                                clocktype    currentTime);

/// 
/// Dequeuing one packet from a queue for a node, interface and priority
///
/// \param nodeID  the node's ID
/// \param layer  protocol layer associated w/ the event
/// \param interfaceIndex  associated interface of node
/// \param priority  priority of queue
/// \param packetSize  size of packet
/// \param time  the current simulation time
///
extern void GUI_QueueDequeuePacket(NodeId       nodeID,
                                   GuiLayers    layer,
                                   int          interfaceIndex,
                                   unsigned int priority,
                                   int          packetSize,
                                   clocktype    currentTime);

/// 
/// This function defines a metric by giving it a name and a
/// description.  The system will assign a number to this data
/// item.  Future references to the data should use the number
/// rather than the name.  The link ID will be used to associate
/// a metric with a particular application link, or MAC interface, etc.
///
/// \param name  the name of the metric
/// \param nodeID  the node's ID
/// \param layer  protocol layer associated w/ the event
/// \param linkID  e.g., an application session ID
/// \param datatype  real/unsigned/integer
/// \param metrictype  cumulative/average, etc.
///
/// \return an identifier associated the the metric name and layer
extern int GUI_DefineMetric(const char*        name,
                            NodeId       nodeID,
                            GuiLayers    layer,
                            int          linkID,
                            GuiDataTypes datatype,
                            GuiMetrics   metrictype);

/// Sends data for an integer metric.
///
/// \param nodeID  the node's ID
/// \param metricID  the value returned by DefineMetric
/// \param value  the current value of the metric
/// \param time  the current simulation time
///
extern void GUI_SendIntegerData(NodeId      nodeID,
                                int         metricID,
                                int         value,
                                clocktype   time);

/// Sends data for an unsigned metric.
///
/// \param nodeID  the node's ID
/// \param metricID  the value returned by DefineMetric
/// \param value  the current value of the metric
/// \param time  the current simulation time
///
extern void GUI_SendUnsignedData(NodeId      nodeID,
                                 int         metricID,
                                 unsigned    value,
                                 clocktype   time);

/// Sends data for a floating point metric.
///
/// \param nodeID  the node's ID
/// \param metricID  the value returned by DefineMetric
/// \param value  the current value of the metric
/// \param time  the current simulation time
///
extern void GUI_SendRealData(NodeId      nodeID,
                             int         metricID,
                             double      value,
                             clocktype   time);


/*------------------------------------------------------------------
 * The following declarations are used for interactive control of
 * the Simulator via the Animator.  Reserved for kernel use.
 *------------------------------------------------------------------*/

/// Maximum length for a single interchange with Animator.
#define GUI_MAX_COMMAND_LENGTH 1024

/// Coded commands sent from Animator to Simulator.
enum GuiCommands {
    GUI_STEP                    = 0,
    GUI_SET_COMM_INTERVAL       = 1,
    GUI_ENABLE_LAYER            = 2,
    GUI_DISABLE_LAYER           = 3,
    GUI_ENABLE_NODE             = 4,
    GUI_DISABLE_NODE            = 5,
    GUI_SET_STAT_INTERVAL       = 6,
    GUI_ENABLE_METRIC           = 7,
    GUI_DISABLE_METRIC          = 8,
    GUI_PATHLOSSTABLE           = 9,
    GUI_DYNAMIC_ReadAsString    = 10, // DYNAMIC_API
    GUI_DYNAMIC_WriteAsString   = 11, // DYNAMIC_API
    GUI_DYNAMIC_ExecuteAsString = 12, // DYNAMIC_API
    GUI_DYNAMIC_Listen          = 13, // DYNAMIC_API
    GUI_DYNAMIC_Unlisten        = 14, // DYNAMIC_API
    GUI_STATS_MANAGER_COMMAND   = 15, // Stats Manager
    GUI_ENABLE_EVENT            = 16,
    GUI_DISABLE_EVENT           = 17,
    GUI_ENABLE_VISOBJ_FILTER    = 18,
    GUI_DISABLE_VISOBJ_FILTER   = 19,
    GUI_ENABLE_VISOBJ_DETAILS   = 20,
    GUI_DISABLE_VISOBJ_DETAILS  = 21,
    GUI_USER_DEFINED            = 100,
    GUI_STOP                    = 1000,
    GUI_PAUSE                   = 1001,
    GUI_RESUME                  = 1002,
    GUI_SET_ANIMATION_FILTER_FREQUENCY = 1003,
    GUI_UNRECOGNIZED            = 5000
};

/// \brief enums for the HITL reponse type
typedef enum {
    GUI_HITL_STARTED              = 0,
    GUI_HITL_STOPPED              = 1,
    GUI_HITL_SUCCESS              = 2,
    GUI_HITL_ERROR                = 3,
    GUI_HITL_OUTPUT               = 4
}GuiHITLResponse;

/// Coded commands sent from Simulator to Animator.
enum GuiReplies {
    GUI_STEPPED                 = 0,
    GUI_DATA                    = 1,
    GUI_ANIMATION_COMMAND       = 2,
    GUI_ASSERTION               = 3,
    GUI_ERROR                   = 4,
    GUI_WARNING                 = 5,
    GUI_SET_EFFECT              = 6,
    GUI_STATISTICS_COMMAND      = 7,
    GUI_PATHLOSS_TABLE          = 9,
    GUI_DYNAMIC_COMMAND         = 10, // DYNAMIC_API
    GUI_STATS_MANAGER_REPLY     = 11, // Stats Manager
    GUI_FINALIZATION_COMMAND    = 12, // Finalization Command
    GUI_UID                     = 13,
    GUI_HITL_RESPONSE           = 14,
    GUI_FINISHED                = 1000
};

/// Structure containing command received from Animator.
struct GuiCommand {
    GuiCommands type;
    std::string args;
};

/// Structure containing message sent to Animator.
struct GuiReply {
    GuiReplies type;
    std::string args;
};

#ifdef D_LISTENING_ENABLED
/// Class used when GUI is running with Dynamic API.  Reports
/// newly created objects to GUI.
class GuiDynamicObjectCallback : public D_ListenerCallback
{
    private:
        EXTERNAL_Interface* m_Iface;
        GuiEvents m_EventType;
        
    public:
        GuiDynamicObjectCallback(
            EXTERNAL_Interface* iface,
            GuiEvents eventType);

        void operator () (const std::string& newValue);
};

class GuiDynamicObjectValueCallback : public D_ListenerCallback
{
    private:
        EXTERNAL_Interface* m_Iface;
        std::string m_Path;
        
    public:
        GuiDynamicObjectValueCallback(
            EXTERNAL_Interface* iface,
            const std::string& path);

        void operator () (const std::string& newValue);
};
#endif // D_LISTNEING_ENABLED


/// Returns true if the GUI was activated on the command line.
///
///
/// \return True if the GUI is enabled.
bool GUI_isAnimateOrInteractive ();

/// Creates a connection to the GUI
///
/// \param argc  number of command line parameters
/// \param argv  command line parameters
/// \param nodeInput  the contents of the .config file
/// \param thisPartitionId  the ID of this partition
///
bool GUI_EXTERNAL_Bootstrap(int        argc,
                            char*      argv [],
                            NodeInput* nodeInput,
                            int        thisPartitionId);

/// Registers the GUI as an external interface
///
/// \param partitionData  the partition to register with
/// \param list  the list to add oneself to
///
void GUI_EXTERNAL_Registration(EXTERNAL_Interface* iface,
                               NodeInput* nodeInput);

/// Function used to replace newline characters in a string being
/// sent to the GUI.
///
/// \param replyType  the type of reply
/// \param msg       : std:  the reply message
///
extern GuiReply GUI_CreateReply(GuiReplies   replyType,
                                std::string* msg);

extern void GUI_ConnectToGUI(char*          hostname,
                             unsigned short port);
extern void GUI_DisconnectFromGUI(SOCKET_HANDLE socket,
                                  bool sendFinishedReply = true);

extern void GUI_WaitForCommands(PartitionData* partitionData,
                                clocktype*     nextStep,
                                clocktype      nextEvent,
                                clocktype      simulationEndTime);
extern GuiCommand GUI_ReceiveCommand(SOCKET_HANDLE socket);
extern void GUI_SendReply(SOCKET_HANDLE socket,
                          GuiReply     reply);
#ifdef MULTI_GUI_INTERFACE
extern void MultiGUI_SendReply(GuiReply reply, Int64 connectionId = -1);
#endif // MULTI_GUI_INTERFACE

extern void GUI_SetLayerFilter(const char* args, // layer ID, applies to animation
                               BOOL  offOrOn);
extern void GUI_SetNodeFilter(const char* args, // node ID, applies to animation
                              BOOL  offOrOn);
extern void GUI_SetMetricFilter(const char* args, // metric ID and node ID, for stats
                                BOOL offOrOn);

extern BOOL GUI_NodeIsEnabledForAnimation(NodeId nodeID);
extern BOOL GUI_NodeIsEnabledForStatistics(NodeId nodeID);
extern BOOL GUI_LayerIsEnabledForStatistics(GuiLayers layer);
void GeneratePathLossTable(Node *node, int channelInd);

void GUI_SendStatsManagerReply(GuiReply reply);

/// Sends activation/deactivation status of interface
/// \param  nodeID  NodeId for which the status needs to be sent
/// \param  nodeStatus  whether activation or deactivation of node is done
/// \param  interfaceIndex index of interface which needs to be activated/deactivated
///         (default value = -1 when the entire node needs to be activated/deactivated)
/// \param  optionalMessage Any optional message (default value = NULL)
///
void GUI_SendInterfaceActivateDeactivateStatus(
                        NodeId       nodeID,
                        GuiEvents    nodeStatus,
                        Int32        interfaceIndex = ANY_INTERFACE,
                        char*        optionalMessage = NULL);

/// \brief Adds a filter
/// Filter format = <filter>|<button - optional>|<node - optional> <br>
///  <filter> - create tab in Visualizaton Controls->Animation Filters <br>
///  <button> - create button in <filter> tab <br>
///  <node> - create menu item for <button> menu <br>
void GUI_AddFilter(
    const char* filter,
    const char* toolTip = "",
    bool        active = false,
    clocktype   time = 0);

void GUI_DeleteObjects(const char* id, clocktype time);

void GUI_DrawLine(
    NodeId          src,
    NodeId          dst,
    const char*     color,
    const char*     id,
    const char*     label,
    float           thickness,
    unsigned short  pattern,
    int             factor,
    bool            srcArrow,
    bool            dstArrow,
    clocktype       time);

void GUI_DrawFlowLine(
    NodeId      src,
    NodeId      dst,
    const char* color,
    const char* id,
    const char* label,
    float       thickness,
    int         pattern,
    int         factor,
    int         ttl,
    clocktype   time);

void GUI_DrawShape(
    GuiVisShapes    shape,
    NodeId          node,
    double          scale,
    const char*     color,
    const char*     id,
    clocktype       time);

void GUI_DrawText(
    NodeId      node,
    const char* text,
    const char* id,
    int         interfaceIndex,
    int         order,
    clocktype   time);

void GUI_AddShapeLegend(
    GuiVisShapes  shape,
    const char*   color,
    const char*   legendText,
    clocktype     time);

void GUI_AddLineLegend(
    const char*     color,
    float           thickness,
    unsigned short  pattern,
    int             factor,
    bool            srcArrow,
    bool            dstArrow,
    const char*     legendText,
    clocktype       time);

void GUI_AppHopByHopFlow(Node* node, Message* msg, NodeAddress previousHop);

void GUI_CreateAppHopByHopFlowFilter(
    UInt32      sessionId,
    NodeId      source,
    const char* srcString,
    const char* destString,
    const char* appName);

bool GUI_IsAppHopByHopFlowEnabled();

void GUI_ReadAppHopByHopFlowAnimationEnabledSetting(
    const NodeInput* nodeInput);

void GUI_RealtimeIndicator(const char* rtStatus);

/// Sends finalization status to the GUI when the GUI is waiting for 
///  simulation to finish a command.
/// \param  nodeID  NodeId for which finalization satus needs to be send
/// \param  layer Layer for which finalization satus needs to be send 
///             (default value = GUI_ANY_LAYER)
/// \param  modelName Model for which finalization satus needs to be sent 
///             (default value = 0)
/// \param  subCommand Any sub-command (default value = 0)
/// \param  optionalMessage Any optional message
///             (default value = NULL)
///
void GUI_SendFinalizationStatus(NodeId       nodeID,
                                GuiLayers    layer = GUI_ANY_LAYER,
                                Int32        modelName = 0,
                                Int32        subCommand = 0,
                                char*        optionalMessage = NULL);
/*------------------------------------------------------------------
 * GUI variables.
 *
 * For kernel use only.
 *------------------------------------------------------------------*/

extern clocktype    GUI_statReportTime;
extern clocktype    GUI_statInterval;

extern unsigned int GUI_guiSocket;
extern BOOL         GUI_guiSocketOpened;

extern MetricLayerData g_metricData[];


#ifdef PAS_INTERFACE
void PSI_EnablePSI(PartitionData* partitionData, char* nodeId);
void PSI_EnableDot11(PartitionData* partitionData, BOOL flag);
void PSI_EnableApp(PartitionData* partitionData, BOOL flag);
void PSI_EnableUdp(PartitionData* partitionData, BOOL flag);
void PSI_EnableTcp(PartitionData* partitionData, BOOL flag);
void PSI_EnableRouting(PartitionData* partitionData, BOOL flag);
void PSI_EnableMac(PartitionData* partitionData, BOOL flag);
#endif

/// Gets the status of the Sops/Vops Interface.
/// \return true if the Sops/Vops interface is enabled
bool SopsVopsInterfaceEnabled();


/// Gets the port number for the Sops/Vops Interface.
/// \return Sops/Vops commuication port number
int SopsPort();


/// Returns the protocol for SOPS
/// \return Selected SopsProtocol
SopsProtocol GetSopsProtocol();


/// Sets the SopsVops interface as enabled.
/// \param port - the simulator's SOPS server port
/// \param sopsProtocol - selected protocol: TCP, UDP
void EnableSopsVopsInterface(int port, SopsProtocol sopsProtocol);

/// \brief Sets Exata Net Link properties
/// Updates three properties related to an Exata Net Link: <br>
///     /node/<nodeId>/exata/netLink/<linkId>/previousHopId <br>
///     /node/<nodeId>/exata/netLink/<linkId>/packetsReceived <br>
///     /node/<nodeId>/exata/netLink/linkId/bytesReceived
/// \param node  destination node
/// \param previousHopId  source node ID
/// \param bytesRcvd  number of bytes received
///
void SetSopsPropertyExataNetLink(
    Node*   node,
    int     previousHopId,
    int     bytesRcvd);

/// \brief Sets Exata App Link properties.
/// Updates three properties related to an Exata App Link: <br>
/// /node/<nodeId>/exata/appLink/<linkId>/sourceId <br>
/// /node/<nodeId>/exata/appLink/<linkId>/packetsReceived <br>
/// /node/<nodeId>/exata/appLink/<linkId>/bytesReceived <br>
/// \param  node  destination node
/// \param  sourceId  source node ID
/// \param  bytesRcvd  bytes received
///
void SetSopsPropertyExataAppLink(Node* node, int sourceId, int bytesRcvd);

/// Sends address change event for an interface
/// \param  nodeID  NodeId for which address changes
/// \param  interfaceIndex  Interface index for which address changes
/// \param  addressToSet  address that needs to be set
/// \param  networkType  network type for an interface
///
void GUI_SendAddressChange(
                        NodeId nodeID,
                        Int32 interfaceIndex,
                        Address addressToSet,
                        NetworkType networkType);


/// \brief Sends a unique id to the GUI.
void SendUidToGuiClient();

/// \brief Creates and sends a HITL command response to subscribing client(s).
///
/// \param hid HID of the command for which response is being sent
/// \param responseType Response value
void GUI_CreateHITLResponse(const std::string& hid,
                            GuiHITLResponse responseType,
                            const std::string& responseString = std::string(""));

/// \brief Strips the HID from an input HITL command.
///
/// \param hitlCommand Input HITL command
void GUI_StripHID(char* hitlCommand);

/// \brief Get HID from input HITL command
///
/// \param hitlCommand Input HITL command
///
/// \return If the input string contains " HID " or " hid ", the
/// string following it is returned, else an empty string is returned.
std::string GUI_GetHID(const char* hitlCommand);

/// \brief Get HID and its index from input HITL command
///
/// \param hitlCommand Input HITL command
/// \param index Index of HID
///
/// \return If the input string contains " HID " or " hid ", the
/// string following it and its index is returned, else an empty string
/// and std::string::npos is returned.
std::string GUI_GetHID(const std::string& hitlCommand, std::size_t* index);

#ifdef MULTI_GUI_INTERFACE

/// \brief Function to check if multigui interface is enabled or not
///
/// \return Returns the multigui interface status(enabled/disabled)
BOOL GUI_isMultigui ();

/// \brief Function returns the multigui commuication port
///
/// \return Returns the multigui commuication port 
int getMultiguiPort();


/// \brief Function setting multigui commuication port
///
/// \param  multigui_port - The TCP port the GUI (using multigui interfcae)
///                        communicates over
void setMultiguiPort(int multigui_port);

/// \brief Function enable multigui interface
void setMultiguiEnabled();

#endif // MULTI_GUI_INTERFACE

#endif // GUI_H
