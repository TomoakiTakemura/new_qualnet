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

/// \defgroup Package_MAPPING MAPPING

/// \file
/// \ingroup Package_MAPPING
/// This file describes data structures and functions for mapping between node pointers,
/// node identifiers, and node addresses.

#ifndef MAPPING_H
#define MAPPING_H

#include "clock.h"

/// Indicates Invalid Mapping
#define INVALID_MAPPING 0xffffffff

/// max no of addressees assigned to an interface
#define MAX_INTERFACE_ADDRESSES 4


/// 
/// Defines node hash size. Hashes the nodeIds using a mod
/// NODE_HASH_SIZE hash.
#define NODE_HASH_SIZE  32

/// Describes the property of a network.
/// 
typedef union network_property_struct
{
    struct
    {
        NodeAddress u_numHostBits;
        NodeAddress u_subnetMask;
        NodeAddress u_subnetAddress;
    } ipv4;
    struct
    {
        unsigned int u_subnetPrefixLen;
        in6_addr     u_subnetAddress;
    } ipv6;

    // Here we introduce ATM related property. User specify ATM network
    // Address by the following string:
    // 'ATM-LINK ICD-<value>.AID-<value>.PTP-<value> {node ids}'.
    struct
    {
        unsigned int u_icd; // store the ICD value.
        unsigned int u_aid; // store the AID value.
        unsigned int u_ptp; // store the PTP value.
    } atm;

} NetworkProperty;

//
// ENUM         :: NetworkProtocolType
// DESCRIPTION  :: Types of various nodes
//
typedef enum
{
    INVALID_NETWORK_TYPE,
    IPV4_ONLY,
    IPV6_ONLY,
    DUAL_IP,
    ATM_NODE,
    GSM_LAYER3,
    CELLULAR,
    NETWORK_VIRTUAL
}
NetworkProtocolType;


/// Describes the type of address mapping.
typedef
struct
{
    Address address;     // key
    unsigned int nodeId;
    NetworkProperty networkProp;
    int interfaceIndex;
    int oppositeMappingIndex;
}
AddressMappingType;

// Dynamic address

// ENUM        :: AddressState
// Description :: The current state of the address in mapping

typedef enum
{
    PREFERRED = 0, // valid address state
    DEPRECATED_ADDR, // deprecated ipv6 address state whose use is discouraged
    TENTATIVE,     // tentative ipv6 address state whose uniqueness is not 
                   // yet verified
    INVALID,       // invalid address state
} AddressState;

//
// STRUCT :: AddressInfo
// DESCRIPTION :: information of all the addresses assigned to the interface.
//
typedef
struct
{
    Address address;
    NetworkProperty networkProp;
    int oppositeMappingIndex;

    // Dynamic address
    AddressState addressState; // current state of address
    clocktype lifeTime; // preferred life-time if address is preffered
                        // valid life-time if address is deprecated

}
AddressInfo;

/// Describes the type of reverse address mapping.
typedef
struct
{
    unsigned int nodeId;    // key
    int interfaceIndex;
    NetworkProtocolType netProtoType;
    int noOfAddresses;
    AddressInfo addressInfo[MAX_INTERFACE_ADDRESSES];
}
AddressReverseMappingType;


/// 
/// Used to determine what the next address counter should be for each
/// subnet address. This is needed to allow different SUBNET/LINK
/// statements to declare the same subnet address.

typedef struct
{

    //Subnet network type.
    NetworkType networkType;
    // Dynamic address
    NetworkProperty addrQual;
    unsigned int addressCounter;

} SubnetListType;

#define HOST_BITS   networkProp.ipv4.u_numHostBits
#define SUBNET_MASK networkProp.ipv4.u_subnetMask
#define SUBNET_ADDR networkProp.ipv4.u_subnetAddress

// Dynamic address
#define list_subnet_addr addrQual.ipv4.u_subnetAddress
#define list_subnet_mask addrQual.ipv4.u_subnetMask
#define list_TLA addrQual.ipv6.u_tla
#define list_NLA addrQual.ipv6.u_nla
#define list_SLA addrQual.ipv6.u_sla

#define list_IPV6_SUBNET_ADDR addrQual.ipv6.u_subnetAddress
#define list_IPV6_PREFIX_LEN addrQual.ipv6.u_subnetPrefixLen

#define list_ICD addrQual.atm.u_icd
#define list_AID addrQual.atm.u_aid
#define list_PTP addrQual.atm.u_ptp

#define TLA networkProp.ipv6.u_tla
#define NLA networkProp.ipv6.u_nla
#define SLA networkProp.ipv6.u_sla

#define IPV6_SUBNET_ADDR networkProp.ipv6.u_subnetAddress
#define IPV6_PREFIX_LEN networkProp.ipv6.u_subnetPrefixLen

#define IPV4_ADDR    address.interfaceAddr.ipv4
#define IPV6_ADDR    address.interfaceAddr.ipv6
#define NETWORK_TYPE address.networkType
#define IPV6_LIFETIME       lifeTime

/// 
/// Describes the detailed information of Node ID <--> IP address
/// mappings.
typedef
struct
{
    int numMappings;
    int numAllocatedMappings;

    int numReverseMappings;
    int numAllocatedReverseMappings;

    AddressMappingType *mappings;
    AddressReverseMappingType *reverseMappings;

    int numSubnets;
    int numAllocatedSubnets;

    SubnetListType *subnetList;
}
AddressMapType;

typedef struct nodeIdToNodePtr * IdToNodePtrMap;

/// 
/// Describes the nodeId and corresponding nodePtr.
struct nodeIdToNodePtr
{
    NodeAddress nodeId;
    Node* nodePtr;
    IdToNodePtrMap next;
};

//---------------------------------------------------------------------------
// Some address related macros and functions.
//---------------------------------------------------------------------------

/// Multicast Address Scope.
#ifndef MADDR6_SCOPE
#define MADDR6_SCOPE(a) \
        MADDR6_SCP_LINK
#endif

/// Checks whether an address is multicast address.
#define IS_MULTIADDR6(a)    ((a).s6_addr8[0] == 0xff)

/// Set an address with 0 values.
#ifndef CLR_ADDR6
#define CLR_ADDR6(a) \
    do { \
        (a).s6_addr32[0] = 0; \
        (a).s6_addr32[1] = 0; \
        (a).s6_addr32[2] = 0; \
        (a).s6_addr32[3] = 0; \
    } while (0)
#endif

/// Does an address have the value of 0 (Cleared).
#ifndef IS_CLR_ADDR6
#define IS_CLR_ADDR6(a) \
        ((a).s6_addr32[0] == 0 && \
         (a).s6_addr32[1] == 0 && \
         (a).s6_addr32[2] == 0 && \
         (a).s6_addr32[3] == 0 )
#endif

/// Copies from-ipv6 address to to-ipv6 address.
#ifndef COPY_ADDR6
#define COPY_ADDR6(from, to) \
    do { \
        (to).s6_addr32[0] = (from).s6_addr32[0]; \
        (to).s6_addr32[1] = (from).s6_addr32[1]; \
        (to).s6_addr32[2] = (from).s6_addr32[2]; \
        (to).s6_addr32[3] = (from).s6_addr32[3]; \
    } while (0)
#endif

/// Checks if a and b address is same address.
#define SAME_ADDR6(a, b) \
    (((a).s6_addr32[0] == (b).s6_addr32[0]) && \
     ((a).s6_addr32[1] == (b).s6_addr32[1]) && \
     ((a).s6_addr32[2] == (b).s6_addr32[2]) && \
     ((a).s6_addr32[3] == (b).s6_addr32[3]))

/// Checks whether the address is any address or not.
#define IS_ANYADDR6(a) \
    (((a).s6_addr32[0] == 0) && \
     ((a).s6_addr32[1] == 0) && \
     ((a).s6_addr32[2] == 0) && \
     ((a).s6_addr32[3] == 0))

/// Checks whether it is loopback address.
#define IS_LOOPADDR6(a) \
    (((a).s6_addr32[0] == 0) && \
     ((a).s6_addr32[1] == 0) && \
     ((a).s6_addr32[2] == 0) && \
     ((a).s6_addr32[3] == 1))

/// Compaires two addresses.
#define CMP_ADDR6(a, b) Ipv6CompareAddr6(a, b)

/// Checks whether it is ipv4 address.
#ifndef IS_IPV4ADDR6
#define IS_IPV4ADDR6(a) \
    (((a).s6_addr32[0] == 0) && \
     ((a).s6_addr32[1] == 0) && \
     ((a).s6_addr32[2] == 0))
#endif

/// Checks whether it is local address.
#define IS_LOCALADDR6(a)    ((a).s6_addr8[0] == 0xfe)

/// Checks whether it is link local address.
#define IS_LINKLADDR6(a) \
    (IS_LOCALADDR6(a) && ((a).s6_addr8[1] == 0x80))

/// Checks whether it is site local address.
#define IS_SITELADDR6(a) \
    (IS_LOCALADDR6(a) && ((a).s6_addr8[1] == 0xc0))

/// Checks whether IPv4 addresses match.
#define SAME_ADDR4(a, b) ((a) == (b))

/// Checks whether IPv4 address is ANY_DEST.
#define IS_ANYADDR4(a) ((a) == ANY_DEST)

/// Check whether both addresses(i.e. addr1 and addr2)
/// are same.
///
/// \param addr1  Pointer to 1st address
/// \param addr2  Pointer to 2nd address
BOOL Address_IsSameAddress(Address* addr1, Address* addr2);

/// Check whether addr is any address of the same type
///
/// \param addr  Pointer to address
BOOL Address_IsAnyAddress(Address* addr);

/// Check whether addr is a multicast address
///
/// \param addr  Pointer to address
BOOL Address_IsMulticastAddress(Address* addr);


/// Check whether addr is a subnet broadcast address
///
/// \param node  pointer to node
/// \param addr  Pointer to address
BOOL Address_IsSubnetBroadcastAddress(Node *node, Address* addr); 

/// Set addr to any address of the same type as refAddr.
///
/// \param addr  Pointer to address
/// \param refAddr  Pointer to refAddr
void Address_SetToAnyAddress(Address* addr, Address* refAddr);

/// Copy srcAddress to dstAddress
///
/// \param dstAddress  Destination address
/// \param srcAddress  Source address
///
void MAPPING_AddressCopy(Address* dstAddress, Address* srcAddress);

/// Compairs to ipv6 address. if a is greater than
/// b then returns positive, if equals then 0,
/// a is smaller then b then negative.
///
/// \param a  ipv6 address.
/// \param b  ipv6 address.
int Ipv6CompareAddr6(in6_addr a, in6_addr b);

/// Checks whether the address is in the same network.
/// : if in the same network then returns TRUE,
/// otherwise FALSE.
///
/// \param globalAddr  Pointer to ipv6 address.
/// \param tla  Top level ipv6 address.
/// \param vla  Next level ipv6 address.
/// \param sla  Site local ipv6 address.
BOOL
Ipv6IsAddressInNetwork(
    const in6_addr* globalAddr,
    unsigned int tla,
    unsigned int nla,
    unsigned int sla);


/// Checks whether the address is in the same network.
/// : if in the same network then returns TRUE,
/// otherwise FALSE.
///
/// \param globalAddr  Pointer to ipv6 address.
/// \param ipv6SubnetAddr  Pointer to ipv6 subnet address.
/// \param prefixLenth  prefix length of the address.
///
/// \return TRUE if the address is in the
/// same network, FALSE otherwise
BOOL
Ipv6IsAddressInNetwork(
    const in6_addr* globalAddr,
    const in6_addr* ipv6SubnetAddr,
    unsigned int prefixLenth);


/// Checks network parameters (tla, nla, sla)
///
/// \param tla  Top level aggregation.
/// \param nla  Next level aggregation.
/// \param sla  Site level aggregaton.
///
/// \return BOOL
BOOL
Ipv6CheckNetworkParams(
    unsigned int tla,
    unsigned int nla,
    unsigned int sla);

//---------------------------------------------------------------------------
// End of address related code.
//---------------------------------------------------------------------------

/// 
/// Hashes the nodeIds using a mod NODE_HASH_SIZE hash. This
/// is not thread safe.
///
/// \param hash  IdToNodePtrMap pointer
/// \param nodeId  Node id.
/// \param nodePtr  Node poniter
void
MAPPING_HashNodeId(
    IdToNodePtrMap* hash,
    NodeAddress     nodeId,
    Node*           nodePtr);


/// Retrieves the node pointer for nodeId from hash.
///
/// \param hash  IdToNodePtrMap pointer
/// \param nodeId  Node id.
///
/// \return Node pointer for nodeId.
Node*
MAPPING_GetNodePtrFromHash(
    IdToNodePtrMap* hash,
    NodeAddress     nodeId);


/// Allocates memory block of size AddressMapType.
///
///
/// \return Pointer to a new AddressMapType structure.
AddressMapType*
MAPPING_MallocAddressMap();


/// Initializes the AddressMapType structure.
///
/// \param map  A pointer of type AddressMapType.
void
MAPPING_InitAddressMap(AddressMapType *map);


/// Builds the address map
///
/// \param nodeInput  A pointer to const NodeInput.
/// \param nodeIdArrayPtr  A pointer to pointer of NodeAddress
/// \param map  A pointer of type AddressMapType.
void
MAPPING_BuildAddressMap(
    const NodeInput *nodeInput,
    int *numNodes,
    NodeAddress **nodeIdArrayPtr,
    AddressMapType *map);


/// Gives Interface address for a Subnet.
///
/// \param node  A pointer to node being initialized
/// \param nodeId  Node id
/// \param subnetAddress  Subnet address
/// \param numHostBits  Number of host bits
///
/// \return Interface address for the subnet.
NodeAddress
MAPPING_GetInterfaceAddressForSubnet(
    Node *node,
    NodeId nodeId,
    NodeAddress subnetAddress,
    int numHostBits);


/// Gives Interface address for a Subnet.
///
/// \param map  A pointer to address map
/// \param nodeId  Node id
/// \param subnetAddress  Subnet address
/// \param numHostBits  Number of host bits
///
/// \return Interface address for the subnet.
NodeAddress
MAPPING_GetInterfaceAddressForSubnet(
    const AddressMapType* map,
    NodeId nodeId,
    NodeAddress subnetAddress,
    int numHostBits);


/// Gives the Subnet address for an interface.
///
/// \param node  A pointer to node being initialized.
/// \param nodeId  Node id
/// \param interfaceIndex  Interface index
///
/// \return Subnet address for an interface.
NodeAddress
MAPPING_GetSubnetAddressForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex);


/// Gives the Subnet mask for an interface.
///
/// \param node  A pointer to node being initialized.
/// \param nodeId  Node id
/// \param interfaceIndex  Interface index
///
/// \return Subnet mask for an interface.
NodeAddress
MAPPING_GetSubnetMaskForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex);


/// Gives the number of host bits for an interface.
///
/// \param node  A pointer to node being initialized.
/// \param nodeId  Node id
/// \param interfaceIndex  Interface index
///
/// \return The number of host bits for an interface.
int
MAPPING_GetNumHostBitsForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex);


/// Gives the Interface information for an interface.
///
/// \param node  A pointer to node being initialized.
/// \param nodeId  Node id
/// \param interfaceIndex  Interface index
/// \param interfaceAddress  Interface address, int pointer.
/// \param subnetAddress  Subnet address, NodeAddress pointer.
/// \param subnetMask  Subnet mask, NodeAddress pointer.
/// \param numHostBits  Number of host bits, int pointer.
void
MAPPING_GetInterfaceInfoForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex,
    NodeAddress *interfaceAddress,
    NodeAddress *subnetAddress,
    NodeAddress *subnetMask,
    int *numHostBits);


/// Gives the Interface address for an interface.
///
/// \param node  A pointer to the node being initialized.
/// \param nodeId  Node id
/// \param interfaceIndex  Interface index
///
/// \return Interface address for an interface.
NodeAddress
MAPPING_GetInterfaceAddressForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex);

/// Get node interface Address according to the network
/// specific interface index.
/// Overloaded function for ATM compatibility.
///
/// \param netType  Network type of the interface.
/// \param relativeInfInx  Inrerface index related to networkType.
///
/// \return Return Address.
// NOTE         :: If relativeInfInx is not valid, networkType of return
//                 Address is NETWORK_INVALID.
Address
MAPPING_GetInterfaceAddressForInterface(
    Node *node,
    NodeAddress nodeId,
    NetworkType netType,
    int relativeInfInx);


/// Gives Node id from an interface address.
///
/// \param node  A pointer to node being initialized.
/// \param interfaceAddress  Interface address
NodeAddress
MAPPING_GetNodeIdFromInterfaceAddress(
    Node *node,
    NodeAddress interfaceAddress);


/// Gives Node id from an interface address.
/// Overloaded for IPv6
///
/// \param node  A pointer to node being initialized.
/// \param interfaceAddress  Interface address
NodeAddress
MAPPING_GetNodeIdFromInterfaceAddress(
    Node *node,
    Address interfaceAddress);

/// Gives default interface address from a node id.
///
/// \param node  A pointer to node being initialized.
/// \param nodeId  Node id
///
/// \return Default interface address from the node id.
NodeAddress
MAPPING_GetDefaultInterfaceAddressFromNodeId(
    Node *node,
    NodeAddress nodeId);


/// Gives the number of nodes in a subnet.
///
/// \param node  A pointer to node being initialized.
/// \param subnetAddress  Subnet address
///
/// \return Number of nodes in a subnet.
unsigned int
MAPPING_GetNumNodesInSubnet(
    Node *node,
    NodeAddress subnetAddress);


/// Gives the subnet address counter.
///
/// \param map  A pointer to AddressMapType.
/// \param subnetAddress  Subnet address
///
/// \return The subnet address counter.
unsigned int
MAPPING_GetSubnetAddressCounter(
    AddressMapType *map,
    NodeAddress subnetAddress);


/// Updates the subnet address counter.
///
/// \param map  A pointer to AddressMapType.
/// \param subnetAddress  Subnet address
/// \param addressCounter  Address counter
void
MAPPING_UpdateSubnetAddressCounter(
    AddressMapType *map,
    NodeAddress subnetAddress,
    int addressCounter);

/// Gets the node's interface index for the given address.
///
/// \param node  A pointer to node being initialized.
/// \param interfaceAddress  Interface address
///
/// \return The interface index.
int
MAPPING_GetInterfaceIndexFromInterfaceAddress(
    Node *node,
    NodeAddress interfaceAddress);

/// Get node interface Address, generic interfaceIndex and
/// Atm related interfaceIndex from ATM Network information.
///
/// \param index  return atm related interface index of a
///    node.
/// \param genIndex  return generic interface index of a node.
///
/// \return Return valid ATM Address related to Network information
/// if genIndex is not equal to -1.
Address
MAPPING_GetNodeInfoFromAtmNetInfo(
    Node *node,
    int nodeId,
    int* index,
    int* genIndex,
    unsigned int* u_atmVal,
    unsigned char afi = 0x47);

/// For a given destination address find its interface index
///
/// \param node  A pointer to node being initialized.
/// \param nodeId  Node ID
/// \param destAddr  Destination address.
unsigned int
MAPPING_GetInterfaceIdForDestAddress(
    Node *node,
    NodeId nodeId,
    NodeAddress destAddr);

/// For a given nodeId & destination address
/// find the subnet mask for the associated network
///
/// \param node  A pointer to node being initialized.
/// \param nodeId  Node ID
/// \param destAddr  Destination address.
NodeAddress
MAPPING_GetSubnetMaskForDestAddress(
    Node *node,
    NodeId nodeId,
    NodeAddress destAddr);

/// For a given nodeId & InterfaceId
/// find the associated IP-Address
///
/// \param node  The pointer to the node.
/// \param nodeId  Node ID
/// \param intfId  Interface ID.
NodeAddress
MAPPING_GetInterfaceAddrForNodeIdAndIntfId(
    Node *node,
    NodeId nodeId,
    int intfId);

/// Get IPV6 network address counter.
///
/// \param map  The address map.
/// \param subnetAddr  The IPv6 address.
/// \param subnetPrefixLen  The prefix length.
///
/// \return The current counter.
unsigned int
MAPPING_GetIPv6NetworkAddressCounter(
        AddressMapType *map,
        in6_addr subnetAddr,
        unsigned int subnetPrefixLen);

/// Update IPV6 network address counter.
///
/// \param map  The address map.
/// \param subnetAddr  The IPv6 address.
/// \param subnetPrefixLen  The prefix length.
/// \param addressCounter  The new counter value.
void
MAPPING_UpdateIPv6NetworkAddressCounter(
    AddressMapType *map,
    in6_addr subnetAddr,
    unsigned int subnetPrefixLen,
    int addressCounter);

/// Get Num of nodes in IPV6 network.
///
/// \param node  The pointer to the node.
/// \param subnetAddr  The IPv6 address.
/// \param subnetPrefixLen  The prefix length.
unsigned int
MAPPING_GetNumNodesInIPv6Network(
    Node *node,
    in6_addr subnetAddr,
    unsigned int subnetPrefixLen);

/// Get Network version IPv4/IPv6.
///
/// \param addrString  The address string
NetworkType
MAPPING_GetNetworkIPVersion(const char* addrString);

/// Identify network type from addrString.
///
/// \param addrString  The address string
NetworkType
MAPPING_GetNetworkType(const char* addrString);

/// Get IPV6 interface information for a interface.
///
/// \param node  The node.
/// \param nodeId  Node Id
/// \param interfaceIndex  The interface index.
/// \param globalAddr  The global IPv6 address.
/// \param subnetAddr  The subnet IPv6 address.
/// \param subnetPrefixLen  THe subnet prefex length.
void
MAPPING_GetIpv6InterfaceInfoForInterface(
    Node *node,
    NodeId nodeId,
    int interfaceIndex,
    in6_addr* globalAddr,
    in6_addr* subnetAddr,
    unsigned int* subnetPrefixLen);

/// Get IPV6 global address.
///
/// \param node  The node
/// \param nodeId  The node's id
/// \param tla  Top level aggregation
/// \param nla  Next level aggregation
/// \param sla  Site level aggregation
/// \param addr6  The global IPv6 address.
BOOL
MAPPING_GetIpv6GlobalAddress(
    Node *node,
    NodeId nodeId,
    unsigned int tla,
    unsigned int nla,
    unsigned int sla,
    in6_addr *addr6);

/// Get IPV6 global address for a node's nth interface.
///
/// \param node  The node
/// \param nodeId  The node's id
/// \param interfaceIndex  The interface index.
/// \param addr6  The global IPv6 address.
/// \param isDeprecated  Return deprecated address (if valid)
BOOL
MAPPING_GetIpv6GlobalAddressForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex,
    in6_addr *addr6);

/// Create IPv6 Global Unicast Address from tla nla sla:
///
/// \param tla  Top level aggregation
/// \param nla  Next level aggregation
/// \param sla  Site level aggregation
/// \param addressCounter  The address counter.
/// \param globalAddr  The global IPv6 address.
void
MAPPING_CreateIpv6GlobalUnicastAddr(
    unsigned int tla,
    unsigned int nla,
    unsigned int sla,
    int addressCounter,
    in6_addr* globalAddr);

/// Create IPv6 Global Unicast Address.
///
/// \param map  The address map.
/// \param IPv6subnetAddress  The subnet address.
/// \param IPv6subnetPrefixLen  The prefix length.
/// \param addressCounter  The address counter.
/// \param globalAddr  The global IPv6 address.
void
MAPPING_CreateIpv6GlobalUnicastAddr(
    AddressMapType *map,
    in6_addr IPv6sunnetAddress,
    unsigned int IPv6subnetPrefixLen,
    int addressCounter,
    in6_addr* globalAddr);

/// Create IPv6 link local Address.
///
/// \param node  pointer to node structure
/// \param interfaceId  interface Id
/// \param globalAddr  The global IPv6 address.
/// \param linkLocalAddr  The subnet IPv6 address.
/// \param subnetPrefixLen  The subnet prefix length.
void MAPPING_CreateIpv6LinkLocalAddr(
    Node* node,
    Int32 interfaceId,
    in6_addr* globalAddr,
    in6_addr* linkLocalAddr,
    unsigned int subnetPrefixLen);

/// Create IPv6 site local Address.
///
/// \param globalAddr  The global IPv6 address.
/// \param siteLocalAddr  The subnet IPv6 address.
/// \param siteCounter  The counter to use.
/// \param subnetPrefixLen  The subnet prefix length.
void
MAPPING_CreateIpv6SiteLocalAddr(
    in6_addr* globalAddr,
    in6_addr* siteLocalAddr,
    unsigned short siteCounter,
    unsigned int subnetPrefixLen);

/// Create ipv6 multicast address.
///
/// \param globalAddr  The global IPv6 address.
/// \param multicastAddr  The multicast IPv6 address.
void
MAPPING_CreateIpv6MulticastAddr(
        in6_addr* globalAddr,
        in6_addr* multicastAddr);

/// create subnet addr for IPV6 address.
///
/// \param tla  Top level aggregation.
/// \param nla  Next level aggregation.
/// \param sla  Site level aggregation.
/// \param IPv6subnetPrefixLen  The IPv6 prefix length.
/// \param IPv6subnetAddress  The IPv6 subnet address.
void
MAPPING_CreateIpv6SubnetAddr(
    unsigned int tla,
    unsigned int nla,
    unsigned int sla,
    unsigned int* IPv6subnetPrefixLen,
    in6_addr* IPv6sunnetAddress);

/// Get node id from Global Address.
///
/// \param node  The node.
/// \param globalAddr  The global IPv6 address.
NodeId
MAPPING_GetNodeIdFromGlobalAddr(
    Node *node,
    in6_addr globalAddr);

/// Get node id from Link layer Address.
///
/// \param node  The node.
/// \param linkLayerAddr  The link layer address.
NodeId
MAPPING_GetNodeIdFromLinkLayerAddr(
    Node* node,
    NodeAddress linkLayerAddr);

/// Create IPv6 link layer Address.
///
/// \param nodeId  The node's id.
/// \param interfaceId  The interface id.
NodeAddress
MAPPING_CreateIpv6LinkLayerAddr(
    unsigned int nodeId,
    int interfaceId);

/// checks whether the ipv6 address is of this node.
///
/// \param node  The node id.
/// \param nodeId  The node's address.
/// \param globalAddr  The global IPv6 address.
BOOL
MAPPING_IsIpv6AddressOfThisNode(
    Node *node,
    const NodeAddress nodeId,
    in6_addr *globalAddr);

// FUNCTION             : MAPPING_IsNodeInThisIpRange:
// PURPOSE              : checks whether the node is in given range of
/// : Addresses.
///
/// \param node  The node.
/// \param nodeId  The node id.
/// \param startRange  The starting address.
/// \param endRange  The end address.
BOOL
MAPPING_IsNodeInThisIpRange(
    Node *node,
    NodeId nodeId,
    NodeAddress startRange,
    NodeAddress endRange);

/// checks whether the ipv4 address is of this node.
///
/// \param node  The node.
/// \param nodeId  The node id.
/// \param addr  The address.
BOOL
MAPPING_IsIpAddressOfThisNode(
    Node *node,
    const NodeAddress nodeId,
    NodeAddress addr);

/// Get interface address for subnet using ipv6 addr.
///
/// \param node  The node.
/// \param nodeId  The node id.
/// \param ipv6SubnetAddr  The subnet address.
/// \param prefixLenth  The subnet prefix length.
/// \param ipv6InterfaceAddr  The ipv6 interface address.
/// \param interfaceIndex  The interface index.
BOOL
MAPPING_GetInterfaceAddressForSubnet(
    Node *node,
    NodeId nodeId,
    in6_addr* ipv6SubnetAddr,
    unsigned int prefixLenth,
    in6_addr* ipv6InterfaceAddr,
    int* interfaceIndex);

/// Get interface address for subnet using ipv6 addr.
///
/// \param map  The address map.
/// \param nodeId  The node id.
/// \param ipv6SubnetAddr  The subnet address.
/// \param prefixLenth  The subnet prefix length.
/// \param ipv6InterfaceAddr  The ipv6 interface address.
/// \param interfaceIndex  The interface index.
///    RETURN               : BOOL :
BOOL
MAPPING_GetInterfaceAddressForSubnet(
    const AddressMapType* map,
    NodeId nodeId,
    in6_addr* ipv6SubnetAddr,
    unsigned int prefixLenth,
    in6_addr* ipv6InterfaceAddr,
    int* interfaceIndex);

/// Get interface address for subnet using tla nla sla.
///
/// \param node  The node.
/// \param nodeId  The node id.
/// \param tla  Top level aggregation.
/// \param nla  Next level aggregation.
/// \param sla  Site level aggregation.
/// \param ipv6Addr  The ipv6 interface address.
/// \param interfaceIndex  The interface index.
BOOL
MAPPING_GetInterfaceAddressForSubnet(
    Node *node,
    NodeId nodeId,
    unsigned int tla,
    unsigned int nla,
    unsigned int sla,
    in6_addr* ipv6Addr,
    int* interfaceIndex);

// FUNCTION             :; MAPPING_GetInterfaceAddressForSubnet
/// Get interface address for subnet using tla nla sla.
///
/// \param map  The address map.
/// \param nodeId  The node id.
/// \param tla  Top level aggregation.
/// \param nla  Next level aggregation.
/// \param sla  Site level aggregation.
/// \param ipv6Addr  The ipv6 interface address.
/// \param interfaceIndex  The interface index.
BOOL
MAPPING_GetInterfaceAddressForSubnet(
    const AddressMapType* map,
    NodeId nodeId,
    unsigned int tla,
    unsigned int nla,
    unsigned int sla,
    in6_addr* ipv6Addr,
    int* interfaceIndex);

/// Get interface from link layer address.
///
/// \param node  The node.
/// \param linkLayerAddr  The link layer address.
int
MAPPING_GetInterfaceFromLinkLayerAddress(
    Node *node,
    const NodeAddress linkLayerAddr);

/// Get interface index from interface address.
///
/// \param node  The node.
/// \param interfaceAddress  The interface address.
int
MAPPING_GetInterfaceIndexFromInterfaceAddress(
   Node *node,
   Address interfaceAddress);


// FUNCTION             : MAPPING_GetIpv6GlobalAddress:
// PURPOSE              : Get ipv6 global address :
///
/// \param node  The node.
/// \param nodeId  The node id
/// \param subnetAddr  The subnet address.
/// \param prefixLen  The subnet prefix length.
/// \param addr6  The IPv6 address.
BOOL
MAPPING_GetIpv6GlobalAddress(
    Node *node,
    NodeId nodeId,
    in6_addr subnetAddr,
    UInt32 prefixLen,
    in6_addr *addr6);


// FUNCTION             : MAPPING_GetDefaultInterfaceAddressInfoFromNodeId:
// PURPOSE              :Get default interface address based on network type:
///
/// \param node  The node.
/// \param nodeId  The node id.
/// \param networktype  The network type.
///    RETURN               : Address :
Address
MAPPING_GetDefaultInterfaceAddressInfoFromNodeId(
    Node *node,
    NodeAddress nodeId,
    NetworkType networktype = NETWORK_IPV4);

// NAME         :: MAPPING_SetAddress
/// Set a Generic address from different type of address.
/// It may be IPv4, IPv6 or ATM address.
///
/// \param netType  The network type.
/// \param destAddress  The destination address.
/// \param srcAddress  The source address.
void
MAPPING_SetAddress(
    NetworkType netType,
    Address* destAddress,
    void* srcAddress);

void
AddressMap_FindReverseMappingElements(
    const AddressMapType *map,
    const NodeAddress representativeAddress,
    int *numberFoundElements,
    int *indexFound);

void
AddressMap_UpdateIpAddress(
   AddressMapType *map,
   char *qualifier,
   int instanceId,
   char *value);

void
AddressMap_UpdateIpSubnetMask(
   AddressMapType *map,
   char *qualifier,
   int instanceId,
   char *value);

void
MAPPING_UpdateNumNodesInSubnet(
        Node *node,
        NodeAddress subnetAddress,
        int numNewNodes);

int
MAPPING_GetUsapSlot(
        Node *node,
        NodeAddress subnetAddress);

int
AddressMap_ConvertSubnetMaskToNumHostBits(NodeAddress subnetMask);

// FUNCTION             : Mapping_AutoCreateIPv6SubnetAddress
// PURPOSE              : Create IPv6 Testing Address Prefix (RFC 2471)from
/// : ipv4 address.
///
/// \param ipAddress  The IPv4 address.
/// \param subnetString   :  string for IPv6 Testing Address Prefix:
///    RETURN               : void : NONE
void Mapping_AutoCreateIPv6SubnetAddress(
    NodeAddress ipAddress,
    char* subnetString);

/// Get subnet address from interface address.
///
/// \param node  The node address.
/// \param interfaceAddress  The interface address.
///    RETURN               : NodeAddress : subnet address
NodeAddress
MAPPING_GetSubnetAddressFromInterfaceAddress(
    Node *node,
    NodeAddress interfaceAddress);


/// Get ipv6 network Prefix from interface address.
///
/// \param node  The node.
/// \param ipv6InterfaceAddr  The IPv6 interface address.
/// \param ipv6SubnetAddr  The subnet address pointer .
BOOL
MAPPING_GetSubnetAddressFromInterfaceAddress(
    Node *node,
    in6_addr* ipv6InterfaceAddr,
    in6_addr* ipv6subnetAddr);

/// Get prefix length for interface address.
///
/// \param node  The node.
/// \param ipv6InterfaceAddr  The IPV6 interface address.
/// \param prefixLenth  The interface prefix length.
BOOL
MAPPING_GetPrefixLengthForInterfaceAddress(
    Node *node,
    in6_addr* ipv6InterfaceAddr,
    unsigned int* prefixLenth);

/// Get Network Protocol Type for the node.
///
/// \param nodeId  The node id.
/// \param nodeInput  The node input file
NetworkProtocolType MAPPING_GetNetworkProtocolTypeForNode(
    Node* node,
    NodeAddress nodeId);

//---------------------------------------------------------------------------
// FUNCTION             : MAPPING_GetNetworkProtocolTypeForInterface:
// PURPOSE              : Get Network Protocol Type for a interface:
// PARAMETERS           ::
// + map                : const AddressMapType *map
// + nodeId             : NodeId nodeId:
// + interfaceIndex     : int interfaceIndex:
// RETURN               : NetworkProtocolType:
//---------------------------------------------------------------------------
NetworkProtocolType
MAPPING_GetNetworkProtocolTypeForInterface(
    const AddressMapType *map,
    NodeId nodeId,
    int interfaceIndex);

/// This function determines the network type of a particular
/// interface of a node
///
/// \param node  Pointer to a node
/// \param interfaceIndex  interface index
///    RETURN : NetworkType: network type of an interface of a node
NetworkType MAPPING_GetNetworkTypeFromInterface(
    Node* node,
    Int32 interfaceIndex);

// Dynamic address
//---------------------------------------------------------------------------
// FUNCTION             : MAPPING_InvalidateIpv4AddressForInterface
// PURPOSE              : make given address invalid in mapping
// PARAMETERS           :
// + node               : Node*: node whose address needs to be
//                               invalidated
// + interfaceIndex     : Int32 : node's interface index
// + Address            : Address* : Interface's address
// RETURN               : void : None
//---------------------------------------------------------------------------
void
MAPPING_InvalidateIpv4AddressForInterface(
    Node* node,
    Int32 interfaceIndex,
    Address* address);

//---------------------------------------------------------------------------
// FUNCTION             : MAPPING_InvalidateIpv6AddressForInterface
// PURPOSE              : make given address invalid in mapping
// PARAMETERS           :
// + node               : Node*: node whose address needs to be
//                                    invalidated
// + interfaceIndex     : Int32 : node's interface index
// + Address            : Address* : Interface's address
// RETURN               : void : None
//---------------------------------------------------------------------------
void
MAPPING_InvalidateIpv6AddressForInterface(
    Node* node,
    Int32 interfaceIndex,
    Address* address);


//---------------------------------------------------------------------------
// FUNCTION             : MAPPING_ValidateIpv4AddressForInterface
// PURPOSE              : Adds new IPV4 address for an interface.
// PARAMETERS           :
// + node               : Node* : node to be updated
// + interfaceIndex     : int : node's interface index
// + address            : NodeAddress : new valid address
// + subnetMask         : NodeAddress : new subnetMask
// + networkType        : NetworkType
// RETURN               : void : None
//---------------------------------------------------------------------------
void
MAPPING_ValidateIpv4AddressForInterface(
    Node* node,
    Int32 interfaceIndex,
    NodeAddress address,
    NodeAddress subnetMask,
    NetworkType networkType);

//---------------------------------------------------------------------------
// FUNCTION             : MAPPING_ValidateIpv6AddressForInterface
// PURPOSE              : Adds new IPV6 address for an interface.
// PARAMETERS           :
// + node               : Node* : node to be updated
// + interfaceIndex     : int : node's interface index
// + address            : in6_addr : new valid address
// + prefixLen         : UInt32 : new prefixLen
// + networkType        : NetworkType
// + prefLifetime       : clocktype
// RETURN               : void : None
//---------------------------------------------------------------------------
void
MAPPING_ValidateIpv6AddressForInterface(
    Node* node,
    Int32 interfaceIndex,
    in6_addr address,
    UInt32 prefixLen,
    NetworkType networkType,
    clocktype prefLifetime);

//---------------------------------------------------------------------------
// API          :: MAPPING_GetInterfaceIdForDestAddress
// PURPOSE      :: For a given destination address find its interface index
// PARAMETERS   ::
// + node       : Node*  : A pointer to node being initialized.
// + nodeId     : NodeId : Node ID
// + destAddr   : Address: Destination address.
// RETURN       :: Int32 : inteface Id if address exists and valid
//                         INVALID_MAPPING : if address do not exist or 
//                                           invalid
//---------------------------------------------------------------------------

Int32
MAPPING_GetInterfaceIdForDestAddress(
    Node* node,
    NodeId nodeId,
    Address destAddr);

//---------------------------------------------------------------------------
// API          :: MAPPING_GetInterfaceAddrForNodeIdAndIntfId
// PURPOSE      :: For a given nodeId & InterfaceId
//                 find the associated address
// PARAMETERS   ::
// + node        : Node*  : The pointer to the node.
// + nodeId      : NodeId : Node ID
// + intfId      : int   : Interface ID.
// + networkType : NetworkType : networkType
// + address     : Address* : The address that will be returned
// RETURN       :: Int32 : 1 : if valid address found and returned
//                         INVALID_MAPPING: if address not found or invalid
//---------------------------------------------------------------------------
Int32
MAPPING_GetInterfaceAddrForNodeIdAndIntfId(
    Node* node,
    NodeId nodeId,
    int intfId,
    NetworkType networkType,
    Address* address);

//---------------------------------------------------------------------------
// FUNCTION             : MAPPING_GetDefaultIPv6AddressFromNodeId:
// PURPOSE              : Get default ipv6 address with best preffered time.
// PARAMETERS           ::
// + node               : Node *node:
// + nodeId             : NodeAddress nodeId:
// RETURN               : Address:
//---------------------------------------------------------------------------
Address
MAPPING_GetDefaultIPv6AddressFromNodeId(
    Node *node,
    NodeAddress nodeId);

//---------------------------------------------------------------------------
// FUNCTION             : MAPPING_InvalidateGlobalAddressForInterface
// PURPOSE              : make given address deprecated or invalid
// PARAMETERS           ::
// + node               : Node *node: node to be updated
// + interfaceIndex     : int interfaceIndex: node's interface index
// + globalAddr         : in6_addr* :
// + isDeprecated       : bool : default value is FALSE
// + depValidLifetime   : clocktype :default value is CLOCKTYPE_MAX
// RETURN               :: void : None
//---------------------------------------------------------------------------
void
MAPPING_InvalidateGlobalAddressForInterface(
    Node *node,
    Int32 interfaceIndex,
    in6_addr* globalAddr,
    bool isDeprecated,
    clocktype depValidLifetime);

//---------------------------------------------------------------------------
// FUNCTION             : MAPPING_UpdateIPv6Address:
// PURPOSE              : update an address with a new subnet from 
//                        another address.
// PARAMETERS           ::
// + previousAddr       : in6_addr* addr:
// + newAddr            : in6_addr* newAddr:
// + subnetPrefixLen    : unsigned int subnetPrefixLen:
// RETURN               : void:
//---------------------------------------------------------------------------
void MAPPING_UpdateIPv6AddressFromPreviousAddress(
    in6_addr* previousAddr,
    in6_addr* newAddr,
    UInt32 subnetPrefixLen);

//---------------------------------------------------------------------------
// FUNCTION             : MAPPING_UpdateIpv6InterfaceInfoForInterface
// PURPOSE              : Update IPV6 interface information for a interface.
// PARAMETERS           ::
// + node               : Node* node: node to be updated
// + interfaceIndex     : int interfaceIndex: node's interface index
// + newPrefferedaddr   : in6_addr* : new preffered global address
// + subnetPrefixLen    : unsigned int : sunet prefix length
// + newPrefLifetime    : clocktype : new address preffed life-time
// + depValidLifetime   : clocktype : deprecated address valid life-time
// RETURN               :: void : None
//---------------------------------------------------------------------------
void
MAPPING_UpdateIpv6InterfaceInfoForInterface(
    Node* node,
    Int32 interfaceIndex,
    in6_addr* newPrefferedaddr,
    UInt32 subnetPrefixLen,
    clocktype newPrefLifetime,
    clocktype depValidLifetime);

//---------------------------------------------------------------------------
// FUNCTION             : MAPPING_SetAddressInGUI
// PURPOSE              : sets the display address in GUI
// PARAMETERS           ::
// + qualifier          : char*        : node id
// + interfaceIndex     : Int32         : node's interface index
// + addressToSet       : char*         : address that is to be displayed
// + networkType        : NetworkType   : network type
// RETURN               :: void : None
//---------------------------------------------------------------------------
void MAPPING_SetAddressInGUI(
                char* qualifier,
                Int32 interfaceIndex,
                char* addressToSet,
                NetworkType networkType);

#endif /* MAPPING_H */

