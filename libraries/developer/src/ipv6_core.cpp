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

// Following is list of features implemented in this version.
// 1. IPv6 is implemented for mac protocols 802.3,802.11,CSMA,MACA & Switch
// 2. CBR, FTP/GENERIC, FTP, TELNET, HTTP/HTTPD ,TRAFFIC_GEN,SUPER-APPLICATION will be able
//    to use IPv6.
// 3. TCP & UDP support IPv6.
// 4. All types of IPv6 addressing is supported.
// 5. Neighbor Solicitation, Neighbor Advertisement, Router Solicitation,
//      Router Advertisement are supported.
// 6. IPv6 Routing Protocols supported are as below:
//      - Static Routing Protocol.
//      - Dynamic Routing Protocols:
//          1. RIPng
//          2. OSPFv3
//
// 7. ICMPv6 for IPv6 control packets sending is supported.
// 8. Fragmentation and Reassembly for large IPv6 packets is implemented.
// 9. Currently FIFO and priority scheduler support IPv6
// 10. dual ip is supported at node level.

// Listed features not implemented.
// 1. MLD is not supported in this version.
// 2. Router, Hop by Hop Option header, Destination Option header
//    processing is blocked due to non availability of other protocol or
//    control blocks.
// 3. dual ip is not supported at interface level.

// Assumptions taken to implement in QualNet simulation.
// 1. For Static Routing Global Aggreable address is taken, as site-local,
//    link-local addressing is confusing and also simulation
//    will have no effect if it is send by global/site/link level address.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "main.h"
#include "api.h"
#include "partition.h"
#include "network_ip.h"
#include "ipv6.h"

//---------------------------------------------------------------------------
// FUNCTION             : Ipv6GetPrefix
// PURPOSE             :: Gets the prefix from the specified address.
// PARAMETERS          ::
// + addr               : in6_addr* addr    : IPv6 Address pointer,
//                              the prefix of which to found.
// + prefix             : in6_addr* prefix  : IPv6 Address pointer
//                              the output prefix.
// + length             : int length        : IPv6 prefix length
// RETURN               : None
//---------------------------------------------------------------------------
void
Ipv6GetPrefix(
    const in6_addr* addr,
    in6_addr* prefix,
    unsigned int length)
{
    int counter;

    // prefix length in octet
    int prefixLength = length / 8;
    unsigned int extraBit = length - prefixLength * 8;

    memset(prefix, 0, sizeof(in6_addr));

    for (counter = 0; counter < prefixLength; counter++)
    {
        prefix->s6_addr8[counter] = addr->s6_addr8[counter];
    }

    if (extraBit)
    {
        unsigned char temp = addr->s6_addr8[counter];

        temp >>= (8 - extraBit);
        temp <<= (8 - extraBit);

        prefix->s6_addr8[counter] = temp;
    }
}


// Check if address is self loopback address.
//
// \param node  Pointer to node.
// \param address  ipv6 address
//
BOOL Ipv6IsLoopbackAddress(Node* node, in6_addr address)
{
    IPv6Data* ipv6  = (IPv6Data*) node->networkData.networkVar->ipv6;
    BOOL retVal = FALSE;

#ifdef CYBER_LIB
    if (ipv6 == NULL)
    {
        return FALSE;
    }
#endif // CYBER_LIB
    if (CMP_ADDR6(address, ipv6->loopbackAddr) == 0)
    {
        retVal = TRUE;
    }
    // Multicast Loopback
    else if (IS_MULTIADDR6(address)
            && (address.s6_addr8[1] & IP6_MULTI_INTERFACE_SCOPE))
    {
        retVal = TRUE;
    }

    return retVal;
}
