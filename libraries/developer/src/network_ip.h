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

/// \defgroup Package_IP IP

/// \file
/// \ingroup Package_IP
/// This file contains data structures and prototypes of
/// functions used by IP.

// Objective: Header file for IP (Internet Protocol)
// Reference: RFC 791

#ifndef IP_H
#define IP_H
#define IP_TTL_DEC 1

#include <string>
#ifdef unix
#include <netinet/in.h>
#include <netinet/ip.h>
#endif
#include "list.h"
#include "if_queue.h"
#include "if_scheduler.h"
#include "multicast_igmp.h"
#include "if_loopback.h"
#include "mac.h"
#include "node.h"
#ifdef CYBER_CORE
#include "network_ipsec.h"
#endif //CYBER_CORE
#include "dynamic.h"
#include "mac_llc.h"

#ifdef ENTERPRISE_LIB
#include "network_access_list.h"
#include "routing_ospfv2.h"
#include "routing_policy_routing.h"
#include "route_parse_util.h"
#include "route_redistribution.h"
#endif // ENTERPRISE_LIB


#ifdef CYBER_LIB
#include "network_secure_neighbor.h"
#endif // CYBER_LIB

#ifdef CYBER_CORE
#include "app_isakmp.h"
#include "network_iahep.h"
#endif // CYBER_CORE


#ifdef ADDON_DB
struct StatsDBNetworkAggregate;
struct OneHopNetworkData;
typedef std::multimap<std::pair<Address, Address>, OneHopNetworkData *, ltaddrpair> NetworkSumAggrData;
class StatsDBMulticastNetworkSummaryContent;
#endif

#ifdef ADDON_NGCNMS
#include "subnet_abstract.h"
#include "network_ngc_haipe.h"
#endif

#include "network_ipfilter.h"

#ifdef ADDON_ABSTRACT
#include "abstract_network.h"
#endif

#define SSM_DEFAULT_FIRST_GRP_ADDR   "232.0.0.0"
#define SSM_DEFAULT_LAST_GRP_ADDR    "232.255.255.255"

#define IANA_START_RANGE    0xE0000000
#define IANA_END_RANGE      0xE00000FF
// Forward declarations
class STAT_NetStatistics;

class D_IpPrint : public D_Command
{
private:
    Node *node;

public:
    D_IpPrint(Node *newNode) { node = newNode; }

    virtual void ExecuteAsString(const std::string& in, std::string& out);
};

//
// BSD-derived code START
//

/*
 * Ported from FreeBSD 2.2.2.
 * Definitions of the IP header and related information.
 */

//  Copyright (c) 1982, 1986, 1993
//   The Regents of the University of California.  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//  3. All advertising materials mentioning features or use of this software
//     must display the following acknowledgement:
//   This product includes software developed by the University of
//   California, Berkeley and its contributors.
//  4. Neither the name of the University nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
//  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
//  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
//  SUCH DAMAGE.
//
//   @(#)ip.h    8.2 (Berkeley) 6/1/94
//

/*
 * Definitions for internet protocol version 4.
 * Per RFC 791, September 1981.
 */


/// Version of IP
#define IPVERSION4       4

//-----------------------------------------------------------------------------
// Routing table (forwarding table)
//-----------------------------------------------------------------------------

//
// Initial allocated entries for routing table.
//

#define FORWARDING_TABLE_ROW_START_SIZE 8

//IP sourec route option padding

#define IP_SOURCE_ROUTE_OPTION_PADDING  1

//
// Structure of an internet header, naked of options.
//

/// IpHeaderType is 20 bytes,just like in the BSD code.

struct IpHeaderType
{
    UInt32 ip_v_hl_tos_len;//ip_v:4,        /* version */
                  //ip_hl:4;       /* header length */
    //unsigned char ip_tos;       /* type of service */
    //UInt16 ip_len;      /* total length */

    UInt16 ip_id;
    UInt16 ipFragment;
                 //ip_reserved:1,
                 //ip_dont_fragment:1,
                 //ip_more_fragments:1,
                 //ip_fragment_offset:13;

    unsigned char  ip_ttl;         /* time to live */
    unsigned char  ip_p;           /* protocol */
    unsigned short ip_sum;         /* checksum */
    unsigned       ip_src,ip_dst;  /* source and dest address */

};

/// Set the value of version number for IpHeaderType
///
/// \param ip_v_hl_tos_len  The variable containing the value of ip_v
///    ,ip_hl,ip_tos and ip_len
/// \param version  Input value for set operation
///
inline void IpHeaderSetVersion(UInt32 *ip_v_hl_tos_len, unsigned int version)
{
    //masks version within boundry range
    version = version & maskInt(29, 32);

    //clears first four bits
    *ip_v_hl_tos_len = *ip_v_hl_tos_len & (~(maskInt(1, 4)));

    //setting the value of version number in ip_v_hl_tos_len
    *ip_v_hl_tos_len = *ip_v_hl_tos_len | LshiftInt(version, 4);
}


/// Set the value of header length for IpHeaderType
///
/// \param ip_v_hl_tos_len  The variable containing the value of ip_v
///    ,ip_hl,ip_tos and ip_len
/// \param hlen  Input value for set operation
///
inline void IpHeaderSetHLen(UInt32 *ip_v_hl_tos_len, unsigned int hlen)
{
    //masks hlen within boundry range
    hlen = hlen & maskInt(29, 32);

    //clears last four bits
    *ip_v_hl_tos_len = *ip_v_hl_tos_len & (~(maskInt(5, 8)));

    //setting the value of header length in ip_v_hl_tos_len
    *ip_v_hl_tos_len = *ip_v_hl_tos_len | LshiftInt(hlen, 8);
}


/// Set the value of Type of Service for IpHeaderType
///
/// \param ip_v_hl_tos_len  The variable containing the value of ip_v
///    ,ip_hl,ip_tos and ip_len
/// \param ipTos  Input value for set operation
///
inline void IpHeaderSetTOS(UInt32 *ip_v_hl_tos_len, unsigned int ipTos)
{
    //masks ipTos within boundry range
    ipTos = ipTos & maskInt(25, 32);

    //clears first four bits
    *ip_v_hl_tos_len = *ip_v_hl_tos_len & (~(maskInt(9, 16)));

    //setting the value of version number in ip_v_hl_tos_len
    *ip_v_hl_tos_len = *ip_v_hl_tos_len | LshiftInt(ipTos, 16);

}


/// Set the value of ip length for IpHeaderType
///
/// \param ip_v_hl_tos_len  The variable containing the value of ip_v
///    ,ip_hl,ip_tos and ip_len
/// \param ipLen  Input value for set operation
///
inline void IpHeaderSetIpLength(UInt32 *ip_v_hl_tos_len, unsigned int ipLen)
{
    //masks ipLen within boundry range
    ipLen = ipLen & maskInt(17, 32);

    //clears last four bits
    *ip_v_hl_tos_len = *ip_v_hl_tos_len & (maskInt(1, 16));

    //setting the value of header length in ip_v_hl_tos_len
    *ip_v_hl_tos_len = *ip_v_hl_tos_len | ipLen;
}


/// Set the value of ip_fragment_offset for
/// IpHeaderType
///
/// \param ipFragment  The variable containing the value of
///    ip_reserved,ip_dont_fragment,ip_more_fragment and
///    ip_fragment_offset
/// \param offset  Input value for set operation
///
inline void IpHeaderSetIpFragOffset(UInt16* ipFragment, UInt16 offset)
{
    //masks offset within boundry range
    offset = offset & maskShort(4, 16);

    //clear last 13 bits of ipFragment
    *ipFragment = *ipFragment & maskShort(1, 3);

    //Setting the value of offset(last 13 bits) in ipFragment
    *ipFragment = *ipFragment | offset;
}


/// Set the value of ipReserved for IpHeaderType
///
/// \param ipFragment  The variable containing the value of
///    ip_reserved,ip_dont_fragment,ip_more_fragment and
///    ip_fragment_offset
/// \param ipReserved  Input value for set operation
///
inline void IpHeaderSetIpReserved(UInt16* ipFragment, BOOL ipReserved)
{
    UInt16 x = (UInt16)ipReserved;

    //masks ip_reserved within boundry range
    x = x & maskShort(16, 16);

    //clears the first bit of ipFragment
    *ipFragment = *ipFragment & (~(maskShort(1, 1)));

    //Setting the value of x at first position in ipFragment
    *ipFragment = *ipFragment | LshiftShort(x, 1);
}

/// Set the value of ip_dont_fragment for IpHeaderType
///
/// \param ipFragment  The variable containing the value of
///    ip_reserved,ip_dont_fragment,ip_more_fragment and
///    ip_fragment_offset
/// \param dontFrag  Input value for set operation
///
inline void IpHeaderSetIpDontFrag (UInt16* ipFragment, BOOL dontFrag)
{
    UInt16 x = (UInt16)dontFrag;

    //masks ip_dont_fragment within boundry range
    x = x & maskShort(16, 16);

    //clears the second bit of ipFragment
    *ipFragment = *ipFragment & (~(maskShort(2, 2)));

    //Setting the value of x at second position in ipFragment
    *ipFragment = *ipFragment | LshiftShort(x, 2);
}


/// Set the value of ip_more_fragment for IpHeaderType
///
/// \param ipFragment  The variable containing the value of
///    ip_reserved,ip_dont_fragment,ip_more_fragment and
///    ip_fragment_offset
/// \param moreFrag  Input value for set operation
///
inline void IpHeaderSetIpMoreFrag(UInt16* ipFragment, BOOL moreFrag)
{
    UInt16 x = (UInt16) moreFrag;

    //masks ip_more_fragment within boundry range
    x = x & maskShort(16, 16);

    //clears the third bit of ipFragment
    *ipFragment = *ipFragment & (~(maskShort(3, 3)));

    //Setting the value of x at third position in ipFragment
    *ipFragment = *ipFragment | LshiftShort(x, 3);
}


/// Returns the value of version number for
/// IpHeaderType
///
/// \param ip_v_hl_tos_len  The variable containing the value of ip_v
///    ,ip_hl,ip_tos and ip_len
///
/// \return unsigned int
inline unsigned int IpHeaderGetVersion(UInt32 ip_v_hl_tos_len)
{
    unsigned int version = ip_v_hl_tos_len;

    //clears last 4 bits
    version = version & maskInt(1, 4);

    //right shifts 4 bits so that last 4 bits contain the value of version
    version = RshiftInt(version, 4);

    return version;
}

/// Returns the value of version number for
/// IpHeaderType or ip6_hdr
///
/// \param void* header points to either a v4 or v6 header
///
/// \return unsigned int the protocol version number.
inline unsigned int IpHeaderGetVersion(const void* header)
{
    // both v6 and v6 store their version in the same location
    const IpHeaderType* header_v4 = reinterpret_cast<const IpHeaderType*>(header);
    return IpHeaderGetVersion(header_v4->ip_v_hl_tos_len);
}

/// Returns the value of header length for IpHeaderType
///
/// \param ip_v_hl_tos_len  The variable containing the value of ip_v
///    ,ip_hl,ip_tos and ip_len
///
/// \return unsigned int
inline unsigned int IpHeaderGetHLen(UInt32 ip_v_hl_tos_len)
{
    unsigned int hlen = ip_v_hl_tos_len;

    //clears first 4 bits
    hlen = hlen & maskInt(5, 8);

    //right shifts 8 bits so that last 4 bits contain the value of
    //header length
    hlen = RshiftInt(hlen, 8);

    return hlen;
}

/// Returns the value of Type of Service for
 //                       IpHeaderType
// PARAMETERS          ::
// + ip_v_hl_tos_len   : UInt32 :The variable containing the value of ip_v
//                       ,ip_hl,ip_tos and ip_len
// RETURN              :: unsigned int
#ifdef P64
// This is used to avoid optimization for this API on 64bit Windows.
// The optimized assembly code shows that the function definition is not correctly
// inlined by the compiler in case of API call in IF condition. The compiler is not
// performing the AND and Right Shift operation to the passed argument which leads
// to the incorrect output of the call. This is applicable with both VC9 and VC10
// 64bit compilers.
#pragma optimize( "", off )
#endif
inline unsigned int IpHeaderGetTOS(UInt32 ip_v_hl_tos_len)
{
    unsigned int ipTos = ip_v_hl_tos_len;

    //clears last 4 bits
    ipTos = ipTos & maskInt(9, 16);

    //right shifts 16 bits so that last 4 bits contain the value of ipTos
    ipTos = RshiftInt(ipTos, 16);

    return ipTos;
}
#ifdef P64
#pragma optimize( "", on )
#endif


/// Returns the value of ip length for IpHeaderType
///
/// \param ip_v_hl_tos_len  The variable containing the value of ip_v
///    ,ip_hl,ip_tos and ip_len
///
/// \return unsigned int
inline unsigned int IpHeaderGetIpLength(UInt32 ip_v_hl_tos_len)
{
    unsigned int ipLen = ip_v_hl_tos_len;

    //clears first 4 bits
    ipLen = ipLen & maskInt(17, 32);

    return ipLen;
}


/// Returns the value of ip_fragment_offset for
/// IpHeaderType
///
/// \param ipFragment  The variable containing the value of
///    ip_reserved,ip_dont_fragment,ip_more_fragment and
///    ip_fragment_offset
///
/// \return UInt16
inline UInt16 IpHeaderGetIpFragOffset(UInt16 ipFragment)
{
    UInt16 offset = ipFragment ;

    //clears first three bits
    offset = offset & maskShort(4, 16);

    return offset;
}

/// Returns the value of ip_dont_fragment for
/// IpHeaderType
///
/// \param ipFragment  The variable containing the value of
///    ip_reserved,ip_dont_fragment,ip_more_fragment and
///    ip_fragment_offset
///
/// \return BOOL
inline BOOL IpHeaderGetIpDontFrag(UInt16 ipFragment)
{
    UInt16 dontFrag = ipFragment;

    //clears all bits except second bit
    dontFrag = dontFrag & maskShort(2, 2);

    //right shifts 14 bits so that last bit contains the value of dontFrag
    dontFrag = RshiftShort(dontFrag, 2);

    return (BOOL)dontFrag;
}


/// Returns the value of ip_more_fragment for
/// IpHeaderType
///
/// \param ipFragment  The variable containing the value of
///    ip_reserved,ip_dont_fragment,ip_more_fragment and
///    ip_fragment_offset
///
/// \return BOOL
inline BOOL IpHeaderGetIpMoreFrag(UInt16 ipFragment)
{
    UInt16 moreFrag = ipFragment;

    //clears all bits except third bit
    moreFrag = moreFrag & maskShort(3, 3);

    //right shifts 13 bits so that last bit contains the value of moreFrag
    moreFrag = RshiftShort(moreFrag, 3);

    return (BOOL)moreFrag;
}


/// Returns the value of ipReserved for IpHeaderType
///
/// \param ipFragment  The variable containing the value of
///    ip_reserved,ip_dont_fragment,ip_more_fragment and
///    ip_fragment_offset
///
/// \return BOOL
inline BOOL IpHeaderGetIpReserved(UInt16 ipFragment)
{
    UInt16 frag_reserved = ipFragment;

    //clears all bits except first bit
    frag_reserved = frag_reserved & maskShort(1, 1);

    //right shifts 15 bits so that last bit contains the value of ipReserved
    frag_reserved = RshiftShort(frag_reserved, 1);

    return (BOOL) frag_reserved;
}



//  Definitions for IP type of service (ip_tos)

/// Type of service ( low delay)
#define IPTOS_LOWDELAY      0x10

/// Type of service ( throughput)
#define IPTOS_THROUGHPUT    0x08

/// Type of service (reliability)
#define IPTOS_RELIABILITY   0x04

/// Type of service ( minimum cost)
#ifndef IPTOS_MINCOST
#define IPTOS_MINCOST       0x02
#endif

/// Bits 6 and 7 in the IPv4 TOS octet are designated as
/// the ECN field. Bit 6 is designated as the ECT bit.
#define IPTOS_ECT      0x02

/// Bits 6 and 7 in the IPv4 TOS octet are designated as
/// the ECN field. Bit 7 is designated as the CE bit.
#define IPTOS_CE       0x01

//
//  Bits 0 to 5 in the IPv4 TOS octet are designated as DSCP field.
//  The range for this 6-bit field is < 0 - 63 >.
//

/// Bits 0 to 5 in the IPv4 TOS octet are designated as DSCP
/// field.The range for this 6-bit field is < 0 - 63 >.
#define IPTOS_DSCP_MAX         0x3f

/// Bits 0 to 5 in the IPv4 TOS octet are designated as DSCP
/// field.The range for this 6-bit field is < 0 - 63 >.
#define IPTOS_DSCP_MIN         0x00

//
//Definitions for IP precedence (also in ip_tos) (hopefully unused)
//

/// IP precedence 'EF clasee internet control'
#define IPTOS_PREC_EFINTERNETCONTROL 0xb8

/// IP precedence 'net control'
#ifndef IPTOS_PREC_NETCONTROL
#define IPTOS_PREC_NETCONTROL       0xe0
#endif

/// IP precedence 'internet control'
#ifndef IPTOS_PREC_INTERNETCONTROL
#define IPTOS_PREC_INTERNETCONTROL  0xc0
#endif
/// IP precedence 'critic ecp'
#ifndef IPTOS_PREC_CRITIC_ECP
#define IPTOS_PREC_CRITIC_ECP       0xa0
#endif

/// IP precedence 'flash override'
#ifndef IPTOS_PREC_FLASHOVERRIDE
#define IPTOS_PREC_FLASHOVERRIDE    0x80
#endif

/// IP precedence 'flash'
#ifndef IPTOS_PREC_FLASH
#define IPTOS_PREC_FLASH            0x60
#endif

/// IP precedence 'immediate'
#ifndef IPTOS_PREC_IMMEDIATE
#define IPTOS_PREC_IMMEDIATE  0x40
#endif

/// IP precedence 'priority'
#ifndef IPTOS_PREC_PRIORITY
#define IPTOS_PREC_PRIORITY    0x20
#endif

/// IP precedence 'routing'
#ifndef IPTOS_PREC_ROUTINE
#define IPTOS_PREC_ROUTINE          0x00
#endif

// CONSTANT    :: IPTOS_PREC_NETCONTROL_MIN_DELAY_SET  : 0xf0
// DESCRIPTION :: IP precedence 'net control'with the 'minimize delay' bit set
#define IPTOS_PREC_NETCONTROL_MIN_DELAY_SET       0xf0

/// IP precedence 'internet control'with the 'minimize delay' bit set
#define IPTOS_PREC_INTERNETCONTROL_MIN_DELAY_SET  0xd0

/// IP precedence 'critic ecp'with the 'minimize delay' bit set
#define IPTOS_PREC_CRITIC_ECP_MIN_DELAY_SET       0xb0

//
// Definitions for options.
//

/// IP option 'copied'.
#ifndef IPOPT_COPIED
#define IPOPT_COPIED(o)     ((o)&0x80)
#endif

/// IP option 'class'
#ifndef IPOPT_CLASS
#define IPOPT_CLASS(o)      ((o)&0x60)
#endif

/// IP option 'number'
#ifndef IPOPT_NUMBER
#define IPOPT_NUMBER(o)     ((o)&0x1f)
#endif


/// IP option 'control'
#define IPOPT_CONTROL       0x00

/// IP option 'reserved1'.
#define IPOPT_RESERVED1     0x20

/// IP option 'debmeas'
#define IPOPT_DEBMEAS       0x40

/// IP option 'reserved2'
#define IPOPT_RESERVED2     0x60

/// IP option 'end of option list'.
#define IPOPT_EOL       0

/// IP option 'no operation'.
#define IPOPT_NOP       1

/// IP option 'record packet route'.
#define IPOPT_RR        7

/// IP option 'timestamp'.
#define IPOPT_TS        68

/// IP option ' provide s,c,h,tcc'.
#define IPOPT_SECURITY  130

/// IP option 'loose source route'.
#define IPOPT_LSRR      131

/// IP option 'satnet id'.
#define IPOPT_SATID     136

/// IP option 'strict source route '.
#define IPOPT_SSRR      137

/// IP option 'Traceroute'.
#define IPOPT_TRCRT      82

//
// Offsets to fields in options other than EOL and NOP.
//

/// Offset to IP option 'option ID'
#define IPOPT_OPTVAL        0

/// Offset to IP option 'option length'
#define IPOPT_OLEN          1

/// Offset to IP option 'offset within option'
#define IPOPT_OFFSET        2

/// Offset to IP option 'min value of above'
#define IPOPT_MINOFF        4

/// Time stamp option structure.
struct ip_timestamp_str
{
    unsigned char ipt_code;       /* IPOPT_TS */
    unsigned char ipt_len;        /* size of structure (variable) */
    unsigned char ipt_ptr;        /* index of current entry */
    unsigned char flgOflw;
             //ipt_flg:4,        /* flags, see below */
             //ipt_oflw:4;       /* overflow counter */

    union ipt_timestamp
    {
        UInt32 ipt_time[1];
        struct ipt_ta
        {
            NodeAddress ipt_addr;
            UInt32 ipt_time;
        } ipt_ta[1];
    } ipt_timestamp;
};

/// TraceRoute option structure.

typedef struct
{
    unsigned char type;
    unsigned char length;

    unsigned short idNumber;
    unsigned short outboundHopCount;

    unsigned short returnHopCount;
    unsigned int originatorIpaddress;

} ip_traceroute;



/// Set the value of flag for ip_timestamp_str
///
/// \param flgOflw  The variable containing the value of flag and
///    overflow counter
/// \param flag  Input value for set operation
///
inline void Ip_timestampSetFlag (unsigned char *flgOflw, unsigned char flag)
{
    //masks ipt_flg within boundry range
    flag = flag & maskChar(5, 8);

    //clears the first 4 bits of flgOflw
    *flgOflw = *flgOflw & maskChar(5, 8);

    //Setting the value of flag in flgOflw
    *flgOflw = *flgOflw | LshiftChar(flag, 4);
}


/// Set the value of ovflw for ip_timestamp_str
///
/// \param flgOflw  The variable containing the value of flag and
///    overflow counter
/// \param ovflw  Input value for set operation
///
inline void Ip_timestampSetOvflw (unsigned char *flgOflw,
                                  unsigned char ovflw)
{
    //masks ipt_oflw within boundry range
    ovflw = ovflw & maskChar(5, 8);

    //clears the last 4 bits of flgOflw
    *flgOflw = *flgOflw & maskChar(1, 4);

    //Setting the value of ovflw in flgOflw
    *flgOflw = *flgOflw | ovflw;
}


/// Returns the value of flag for ip_timestamp_str
///
/// \param flgOflw  The variable containing the value of flag and
///    overflow counter
///
/// \return unsigned char
inline unsigned char Ip_timestampGetFlag (unsigned char flgOflw)
{
    unsigned char flag = flgOflw;

    //clears last 4 bits
    flag = flag & maskChar(1, 4);

    //right shifts 4 bits so that last 4 bits contain the value of flag
    flag = RshiftChar(flag, 4);

    return flag;
}


/// Returns the value of overflow counter for
/// ip_timestamp_str
///
/// \param flgOflw  The variable containing the value of flag and
///    overflow counter
///
/// \return unsigned char
inline unsigned char Ip_timestampGetOvflw (unsigned char flgOflw)
{
    unsigned char oflwc = flgOflw;

    //clears first 4 bits
    oflwc = oflwc & maskChar(5, 8);

    return oflwc;
}


//
// flag bits for ipt_flg
//

/// Flag bits for ipt_flg (timestamps only );
#define IPOPT_TS_TSONLY     0

/// Flag bits for ipt_flg (timestamps and addresses );
#define IPOPT_TS_TSANDADDR  1

/// Flag bits for ipt_flg (specified modules only );
#define IPOPT_TS_PRESPEC    3

//
// bits for security (not byte swapped)
//

/// 'unclass' bits for security in IP option field
#define IPOPT_SECUR_UNCLASS 0x0000

/// 'confid' bits for security in IP option field
#define IPOPT_SECUR_CONFID  0xf135

/// 'efto' bits for security in IP option field
#define IPOPT_SECUR_EFTO    0x789a

/// 'mmmm' bits for security in IP option field
#define IPOPT_SECUR_MMMM    0xbc4d

/// 'restr' bits for security in IP option field
#define IPOPT_SECUR_RESTR   0xaf13

/// 'secreat' bits for security in IP option field
#define IPOPT_SECUR_SECRET  0xd788

/// 'top secret' bits for security in IP option field
#define IPOPT_SECUR_TOPSECRET   0x6bc5

/// Internet implementation parameters
/// (maximum time to live (seconds) )
#define MAXTTL      255

/// TTL value for which we consider TTL not set.
/// Used in TCP/UDP app and transport layer TTL
#define TTL_NOT_SET 0

/// Internet implementation parameters
/// (default ttl, from RFC 1340 )
#define IPDEFTTL    64

/// Internet implementation parameters
/// (time to live for frags, slowhz )
#define IPFRAGTTL   60

/// Internet implementation parameters (subtracted
/// when forwarding)
#define IPTTLDEC    1

/// Internet implementation parameters (default TOS )
#define IPDEFTOS    0x10

/// Internet implementation parameters ( default maximum
/// segment size )
#define IP_MSS      576

//----------------------------------------------------------
// DEFINES
//----------------------------------------------------------

//----------------------------------------------------------
// IP protocol numbers
//----------------------------------------------------------

// IP protocol numbers for network-layer and transport-layer protocols

/// IP protocol numbers.
#ifndef IPPROTO_IP
#define IPPROTO_IP  0
#endif

/// IP protocol numbers for ICMP.
#ifndef IPPROTO_ICMP
#define IPPROTO_ICMP            1
#endif

/// IP protocol numbers for IGMP.
#ifndef IPPROTO_IGMP
#define IPPROTO_IGMP            2
#endif

/// IP protocol numbers for IP tunneling.
#ifndef IPPROTO_IPIP
#define IPPROTO_IPIP              4
#endif

/// IP protocol numbers for TCP .
#ifndef IPPROTO_TCP
#define IPPROTO_TCP             6
#endif

/// IP protocol numbers for UDP
#ifndef IPPROTO_UDP
#define IPPROTO_UDP             17
#endif

/// IP protocol number for DUAL-IP.
#ifndef IPPROTO_IPV6
#define IPPROTO_IPV6  41
#endif



/// IP protocol numbers for RSVP.
#ifndef IPPROTO_RSVP
#define IPPROTO_RSVP            46
#endif

/// IP protocol numbers for MOBILE_IP.
#define IPPROTO_MOBILE_IP       48

/// IP protocol numbers for IPSEC.
#ifndef IPPROTO_ESP
#define IPPROTO_ESP             50
#endif

/// IP protocol numbers for IPSEC.
#ifndef IPPROTO_AH
#define IPPROTO_AH              51
#endif

/// IP protocol numbers for IPSEC.
#define IPPROTO_ISAKMP          52


/// IP protocol numbers.
#ifndef IPPROTO_IAHEP
#define IPPROTO_IAHEP  54
#endif




/// IP protocol numbers for OSPF .
#define IPPROTO_OSPF            89

/// IP protocol numbers for PIM .
#ifndef IPPROTO_PIM
#define IPPROTO_PIM             103
#endif

/// IP protocol numbers for IGRP .
#ifndef IPPROTO_IGRP
#define IPPROTO_IGRP            100
#endif

/// IP protocol numbers for EIGRP .
#ifndef IPPROTO_EIGRP
#define IPPROTO_EIGRP           88
#endif

/// IP protocol numbers for BELLMANFORD.
#define IPPROTO_BELLMANFORD     150

#ifdef ADDON_NGCNMS
/// IP protocol numbers for IP_RED.
#define IPPROTO_IPIP_RED       151
#endif

/// IP protocol numbers for FISHEYE .
#define IPPROTO_FISHEYE         160

/// IP protocol numbers for LANMAR .
#define IPPROTO_FSRL            161

#ifdef CYBER_LIB
/// IP protocol numbers for ANODR .
#define IPPROTO_ANODR           162

/// IP protocol numbers for secure neighbor discovery .
#define IPPROTO_SECURE_NEIGHBOR 163

/// IP protocol numbers for secure routing community
#define IPPROTO_SECURE_COMMUNITY 164
#endif // CYBER_LIB


/// IP protocol numbers for AODV .
#define IPPROTO_AODV            123

/// IP protocol numbers for DYMO .
#define IPPROTO_DYMO            132

#ifdef ADDON_MAODV
/// IP protocol numbers for MAODV.
#define IPPROTO_MAODV           124
#endif // ADDON_MAODV

/// IP protocol numbers for DSR .
#define IPPROTO_DSR             135

/// IP protocol numbers for ODMRP .
#define IPPROTO_ODMRP           145

/// IP protocol numbers for LAR1.
#define IPPROTO_LAR1            110

/// IP protocol numbers for STAR.
#define IPPROTO_STAR            136

/// IP protocol numbers for DAWN.
#define IPPROTO_DAWN            120

/// IP protocol numbers for EPLRS protocol.

/// IP protocol numbers for DVMRP.
#define IPPROTO_DVMRP           200

#ifdef CELLULAR_LIB
/// IP protocol numbers for GSM.
#define IPPROTO_GSM             202
#endif // CELLULAR_LIB

#ifdef ADVANCED_WIRELESS_LIB
// /*
// CONSTANT   :: IPPROTO_DOT16 : 249
// DESCRITION :: IP protocol number for dot16 backbone msg
#define IPPROTO_DOT16           249
#endif // ADVANCED_WIRELESS_LIB
/// IP protocol  for external interface.
#define IPPROTO_EXTERNAL        233

#ifdef EXATA
/// IP protocol numbers for Internet gateway for emulated nodes
#define IPPROTO_INTERNET_GATEWAY 240

/// IP protocol numbers for Internet gateway for emulated nodes
#define IPPROTO_EXATA_VIRTUAL_LAN 241
#endif

/// IP protocol numbers for NDP.
#define IPPROTO_NDP             255

//startCellular
#define IPPROTO_CELLULAR 250
//endCellular

/// IP protocol numbers for BRP .
#define IPPROTO_BRP             251

//StartZRP
#define IPPROTO_ZRP    252
//EndZRP
//StartIERP
#define IPPROTO_IERP    253
//EndIERP
//StartIARP
#define IPPROTO_IARP    254
//EndIARP
//InsertPatch ROUTING_IPPROTO

//----------------------------------------------------------
// IP header
//----------------------------------------------------------

/// Minimum IP header size in bytes
#define IP_MIN_HEADER_SIZE       20

/// Maximum IP header size in bytes
#define IP_MAX_HEADER_SIZE      60

/// Fragmented packets hold time.
#define IP_FRAGMENT_HOLD_TIME  60 * SECOND

/// Returns IP header ip_hl field * 4, which is the size of
/// the IP header in bytes.

// PARAMETERS   IpHeaderType *ipHeader
//                  Pointer to the IP header.
//
// NOTES        (ipHeader)->ip_hl is the number of 4-byte words in the
//              IP header.
//

#define IpHeaderSize(ipHeader) (RshiftInt((ipHeader)->ip_v_hl_tos_len & maskInt(5,8),8) * 4)

//-----------------------------------------------------------------------------
// MACRO        IpHeaderHasSourceRoute()
// PURPOSE      Returns boolean depending on whether IP header has a
//              source route.  (Just calls IpHeaderSourceRouteOptionField()
//              and checks if what's returned is NULL.)
// PARAMETERS   IpHeaderType *IpHeader
//                  Pointer to IP header.
// RETURNS      true, if ipHeader has source route.
//              false, if ipHeader does not have source route.
//-----------------------------------------------------------------------------

#define IpHeaderHasSourceRoute(ipHeader) \
    (IpHeaderSourceRouteOptionField(ipHeader) != NULL)

#ifdef CYBER_CORE
#define BROADCAST_FOWARDING_TIMEOUT 10 * SECOND
#endif // CYBER_CORE


/// Sets IP header ip_hl field (header length) to Size
/// divided by 4

// PARAMETERS   IpHeaderType *IpHeader
//                  Pointer to IP header.
//              unsigned Size
//                  Size of the IP header in bytes.
//
// NOTES        This macro checks that Size is evenly divisible by 4,
//              and also that Size falls within IP_MIN_HEADER_SIZE
//              and IP_MAX_HEADER_SIZE (inclusive).
//

/*#define SetIpHeaderSize(IpHeader, Size) \
    IpHeader->ip_hl = (Size) / 4; \
    ERROR_Assert(((Size) % 4 == 0) && \
                 (Size >= IP_MIN_HEADER_SIZE) && \
                 (Size <= IP_MAX_HEADER_SIZE), "Invalid IP header size");
*/
//----------------------------------------------------------
// Multicast
//----------------------------------------------------------

//
// Used to determine whether an IP address is multicast.
//

/// Used to determine whether an IP address is multicast.
#define IP_MIN_MULTICAST_ADDRESS 0xE0000000    // 224.0.0.0

/// Used to determine whether an IP address is multicast.
#define IP_MAX_MULTICAST_ADDRESS 0xEFFFFFFF    // 239.255.255.255

/// Default multicast interface address (224.0.0.0).
#define MULTICAST_DEFAULT_INTERFACE_ADDRESS  3758096384u    // 224.0.0.0

/// Minimum reserve multicast address (224.0.0.0).
#define IP_MIN_RESERVED_MULTICAST_ADDRESS 0xE0000000  // 224.0.0.0

/// Maximum reserve multicast address (224.0.0.255).
#define IP_MAX_RESERVED_MULTICAST_ADDRESS 0xE00000FF  // 224.0.0.255


#define RIPV2_DEST_ADDRESS  0xE0000009L

/// Multicast default num host bit
#define MULTICAST_DEFAULT_NUM_HOST_BITS      27

// STATS DB CODE
#define DEFAULT_CONNECTIVITY_INTERVAL 500 // (in seconds)
//----------------------------------------------------------
// Routing
//----------------------------------------------------------


/// Network unreachable.
#define NETWORK_UNREACHABLE     -2

/// Default interface index
#define DEFAULT_INTERFACE       0

//----------------------------------------------------------
// STRUCTS, ENUMS, AND TYPEDEFS
//----------------------------------------------------------
//-----------------------------------------------------------------------------
// Info field
//-----------------------------------------------------------------------------

//
// Used by NetworkIpSendPacketOnInterfaceWithDelay().
//
typedef struct
{
    int incomingInterface;
    int outgoingInterface;
    NodeAddress nextHop;
#ifdef ADDON_DB
    BOOL rawPacket;
#endif // ADDON_DB
}
DelayedSendToMacLayerInfoType;


//
// Extra information stored in a message while it's in an IP queue.
//
// Currently contains only the next hop address.
// This is not really part of an IP packet, and for efficiency do not
// want to add another header just for this field, hence, the reason
// for this struct.
//

typedef
struct
{
    // Used incase of IPv6 packet.
    in6_addr ipv6NextHopAddr;
    NodeAddress nextHopAddress;
    //MacHWAddress destAddr;
    unsigned char macAddress [MAX_MACADDRESS_LENGTH];
    unsigned short hwLength;
    unsigned short hwType;

    union
    {
        NodeAddress ipv4DestAddr;
        in6_addr    ipv6DestAddr;
    } destinationAddress;

    // What type of packet is it?  IP, Link-16 and others...
    int networkType;

    // Used to determine where packet will go after passing through
    // the backplane.
    int outgoingInterface;

    // Used to restore user priority
    TosType userTos;

    // Used for CENTRAL backplane to get the information of the
    // incoming interface of the packet. At present all packets
    // are store into CPU queue instead of internal scheduler
    // to handles all INPUT and CPU queues.
    int incomingInterface;

    // SEB - MIMDL Updated for WNW MI/MDL interaction
    // reservation Id for packet assigned by MI layer
    // usage specific for the WNW MI/MDL layers..
    unsigned int rsvnId;

    // Sequence number is used to identify the transmit request for
    // transmit completion statusing.
    // Usage specific for the WNW MI/MDL layers..
    unsigned long seqNumber;

} QueuedPacketInfo;


/// Structure maintaining IP Back plane Information
typedef struct networkIpBackplaneInfoStruct
{
    int incomingInterface;
    NodeAddress hopAddr;
    MacHWAddress hopMacAddr;

} NetworkIpBackplaneInfo;

/// Structure maintaining IP header size Information

typedef struct ipHeaderSizeInfoStruct
{
    int size;
} ipHeaderSizeInfo;


//----------------------------------------------------------
// Function Pointers
//----------------------------------------------------------

typedef
void (*RouterFunctionType)(
    Node *node,
    Message *msg,
    NodeAddress destAddr,
    NodeAddress previousHopAddress,
    BOOL *PacketWasRouted);

typedef
void (*MacLayerStatusEventHandlerFunctionType)(
    Node *node,
    const Message *msg,
    const NodeAddress nextHop,
    const int interfaceIndex);

typedef
void (*PromiscuousMessagePeekFunctionType)(
    Node *node,
    const Message *msg,
    NodeAddress prevHop);

typedef
void (*MacLayerAckHandlerType) (
    Node* node,
    int interfaceIndex,
    const Message* msg,
    NodeAddress nextHop);

typedef
void (*NetworkRouteUpdateEventType)(
    Node *node, NodeAddress destAddr, NodeAddress destAddrMask,
    NodeAddress nextHop, int interfaceId, int metric,
    NetworkRoutingAdminDistanceType adminDistance);



//----------------------------------------------------------
// Multicast
//----------------------------------------------------------

typedef
void (*MulticastRouterFunctionType)(
    Node *node,
    Message *msg,
    NodeAddress destAddr,
    int interfaceIndex,
    BOOL *PacketWasRouted,
    NodeAddress prevHop);

/// Structure of an entity of multicast forwarding table.
typedef
struct
{
    NodeAddress sourceAddress;
    NodeAddress sourceAddressMask;        // Not used
    NodeAddress multicastGroupAddress;
    LinkedList *outInterfaceList;
} NetworkMulticastForwardingTableRow;

/// Structure of multicast forwarding table
typedef
struct
{
    int size;
    int allocatedSize;
    NetworkMulticastForwardingTableRow *row;
} NetworkMulticastForwardingTable;

/// Structure for Multicast Group Entry
typedef struct
{
    NodeAddress groupAddress;
    int memberCount;
    // Used only in IGMPv3 cases.
    NodeAddress srcAddress;

    // Not used.
    int interfaceIndex;

} NetworkIpMulticastGroupEntry;

//----------------------------------------------------------
// Per Hop Behavior (PHB)
//----------------------------------------------------------

/// Structure to maintain DS priority queue mapping
typedef
struct
{
    unsigned char ds;
    QueuePriorityType priority;
} IpPerHopBehaviorInfoType;

//----------------------------------------------------------
// Diffserv Multi-Field Traffic Conditioner
//----------------------------------------------------------

// Function pointer definition for Diffserv Meters
typedef BOOL (*MeterFunctionType)(
    Node* node,
    NetworkDataIp* ip,
    Message* msg,
    int conditionClass,
    int* preference);

// Variables of the structure define a unique condition class

/// Variables of the structure define a unique condition class
typedef
struct
{
    Address             srcAddress; //Address
    Address             destAddress; //Address
    unsigned char       ds;
    unsigned char       protocolId;
    int                 sourcePort;
    int                 destPort;
    int                 incomingInterface;
    int                 conditionClass;
} IpMftcParameter;

/// Structure used to store traffic condition.
typedef
struct
{
    int                 conditionClass;

    // Variables used for Meter or Policer.
    Int32               committedBurstSize;
    Int32               excessBurstSize;    // also used for peak burst size
    BOOL                colorAware;
    Int32               tokens;
    Int32               largeBucketTokens;  // also used for peak bucket size
    Int32               rate;
    Int32               peakInformationRate;
    clocktype           lastTokenUpdateTime;
    double              avgRate;            // Used for TSW metering
    double              winLen;             // Used for TSW metering
    MeterFunctionType   meterFunction;

    // Variables used for statistics collection.
    unsigned int        numPacketsIncoming;
    unsigned int        numPacketsConforming;
    unsigned int        numPacketsPartConforming;
    unsigned int        numPacketsNonConforming;

    // Variables used for action taken on Out-profile packet.
    BOOL                dropOutProfilePackets;
    unsigned char       dsToMarkOutProfilePackets;

    // Variables used for PHB Service.
    unsigned char       serviceClass;
}
IpMultiFieldTrafficConditionerInfo;


//----------------------------------------------------------
// IP options
//----------------------------------------------------------

/// Structure of optional header for IP source route
typedef
struct ip_options  /* options header */
{
    unsigned char code;
    unsigned char len;
    unsigned char ptr;
}
IpOptionsHeaderType;


//----------------------------------------------------------
// Router Model
//----------------------------------------------------------
/// NetworkIp backplane type(either CENTRAL or DISTRIBUTED)

typedef enum
{
    BACKPLANE_TYPE_CENTRAL = 0,
    BACKPLANE_TYPE_DISTRIBUTED = 1
}
BackplaneType;


//------------------------------------------------------------
// Statistics
//------------------------------------------------------------


/// Structure used to keep track of all stats of network layer.
typedef
struct
{
    // SNMP statistics for IP.

    // Total number of received IP datagrams from all interfaces.
    D_UInt32 ipInReceives;
    int ipInReceivesId;
    // Number of IP datagrams discarded because of header errors
    // (e.g., checksum error, version number mismatch, TTL exceeded,
    // etc.).
    UInt32 ipInHdrErrors;
    int ipInHdrErrorsId;
    // Number of IP datagrams discarded because of incorrect
    // destination address.
    UInt32 ipInAddrErrors;
    // Number of IP datagrams for which an attempt was made to forward.
    UInt32 ipInForwardDatagrams;
    // Number of IP datagrams received successfully but discarded
    // because of an unknown or unsupported protocol
    UInt32 ipInUnknownProtos;
    // Number of received IP datagrams discarded because of a lack of
    // buffer space.
    UInt32 ipInDiscards;
    // Number of IP datagrams delivered to appropriate protocol module.
    UInt32 ipInDelivers;
    // Total number of IP datagrams passed to IP for transmission.
    // Does not include those counted in ipForwardDatagrams.
    UInt32 ipOutRequests;
    // Number of output IP datagrams discarded because of a lack of
    // buffer space.
    UInt32 ipOutDiscards;
    // Number of IP datagrams discarded because no route could be found.
    UInt32 ipOutNoRoutes;
    // Number of IP fragments received that needed to be reassembled.
    D_UInt32 ipReasmReqds;
    // Number of IP datagrams successfully reassembled.
    UInt32 ipReasmOKs;
    // Number of failures by IP reassembly algorithm.
    UInt32 ipReasmFails;
    // Number of IP datagrams that have been successfully fragmented.
    UInt32 ipFragOKs;
    // Number of IP fragmented datagrams that are in fragments buffer.
    UInt32 ipFragsInBuff;
    // Number of IP fragments created.
    D_UInt32 ipFragsCreated;
    // Number of IP datagrams after joining fragments.
    UInt32 ipPacketsAfterFragsReasm;

    // Number of IP datagrams that have been
    // discarded because they needed to be fragmented at
    // this entity but could not be, e.g., because their
    // Don't Fragment flag was set..
    D_UInt32 ipFragFails;

    // ATM : Number of Ip datagrams routed thru gateway
    UInt32 ipRoutePktThruGt;
    UInt32 ipSendPktToOtherNetwork;
    UInt32 ipRecvdPktFromOtherNetwork;
    //end for ATM

    // Variables used for run-time statistics.

    UInt32 ipInReceivesLastPeriod;
    UInt32 ipInHdrErrorsLastPeriod;
    UInt32 ipInAddrErrorsLastPeriod;
    UInt32 ipInForwardDatagramsLastPeriod;
    UInt32 ipInDiscardsLastPeriod;
    UInt32 ipInDeliversLastPeriod;
    UInt32 ipOutRequestsLastPeriod;
    UInt32 ipOutDiscardsLastPeriod;
    UInt32 ipOutNoRoutesLastPeriod;
    UInt32 ipReasmReqdsLastPeriod;
    UInt32 ipReasmOKsLastPeriod;
    UInt32 ipReasmFailsLastPeriod;
    UInt32 ipFragOKsLastPeriod;

    // Total value of TTLs for packets delivered to a node.  This is
    // used to calculate an "ipInDelivers TTL-based average hop count"
    // metric.
    UInt32 deliveredPacketTtlTotal;

    // Keep track of how many packets are dropped due to backplane limit.
    UInt32 ipNumDroppedDueToBackplaneLimit;

    // Fragment related statistics.
    UInt32 ipNoOfPacketsAfterJoiningFragments;
    BOOL bufferSizeStats;


    // Stats for Network Aggregate Tables
#ifdef ADDON_DB
    StatsDBNetworkAggregate* aggregateStats;
#endif

    UInt32 ipCommsDropped;
    UInt32 ipPktsWarmupDelay;
    UInt32 ipPktsWarmupDropped;
}
NetworkIpStatsType;

//----------------------------------------------------------
// Routing table (forwarding table)
//----------------------------------------------------------

/// Structure of an entity of forwarding table.
typedef
struct
{
    NodeAddress destAddress;        // destination address
    NodeAddress destAddressMask;    // subnet destination Mask
    int interfaceIndex;           // index of outgoing interface
    NodeAddress nextHopAddress;     // next hop IP address

    int cost;

    // routing protocol type
    NetworkRoutingProtocolType protocolType;

    // administrative distance for the routing protocol
    NetworkRoutingAdminDistanceType adminDistance;

    BOOL interfaceIsEnabled;
}
NetworkForwardingTableRow;

/// Structure of forwarding table.
typedef
struct
{
    int size;                        // number of entries
    int allocatedSize;
    int numStaticRoutes; // number of static routes in routing table
    NetworkForwardingTableRow *row;  // allocation in Init function in Ip
}
NetworkForwardingTable;

//-----------------------------------------------------------
// Interface info
//-----------------------------------------------------------

/// Structure for maintaining IP interface informations.  This
/// struct must be allocated by new, not MEM_malloc.  All
/// member variables MUST be initialized in the constructor.
struct IpInterfaceInfoType
{
    // Constructor.  All member variables MUST be initialized in the
    // constructor.
    IpInterfaceInfoType();

    Scheduler* scheduler;
    Scheduler* inputScheduler;
    NetworkIpBackplaneStatusType backplaneStatus;

    RandomSeed dropSeed;

    D_NodeAddress ipAddress;
    int numHostBits;
    char interfaceName[MAX_STRING_LENGTH];

    // to accomodate all the possible ports..etc
    char* intfNumber;

    RouterFunctionType                      routerFunction;
    NetworkRoutingProtocolType              routingProtocolType;
    void*                                   routingProtocol;

    BOOL multicastEnabled;
    MulticastRouterFunctionType multicastRouterFunction;
    NetworkRoutingProtocolType multicastProtocolType;
    void *multicastRoutingProtocol;

    MacLayerStatusEventHandlerFunctionType  macLayerStatusEventHandlerFunction;
    PromiscuousMessagePeekFunctionType      promiscuousMessagePeekFunction;
    MacLayerAckHandlerType                  macAckHandler;
    BOOL hsrpEnabled;
    // IPv6 interface information
    NetworkType interfaceType;
    BOOL isVirtualInterface;
    //for Unnumbered interface support
    BOOL isUnnumbered;

#ifdef ENTERPRISE_LIB
    // enum defined in /network/rt_parse_util.h
    RtInterfaceType intfType;

    // Access list
    AccessListPointer *accessListInPointer;
    AccessListPointer *accessListOutPointer;
    AccessListStats accessListStat;

    // Route Redistribute
    RoutingTableUpdateFunction routingTableUpdateFunction;

    // For policy based routing
    RouteMap *rMapForPbr;
    PbrStat pbrStat;
#endif // ENTERPRISE_LIB

    // For MPR
    BOOL useRoutingCesMpr;

    struct ipv6_interface_struct* ipv6InterfaceInfo;

    //Tunnel information
    LinkedList* InterfacetunnelInfo;

    // Need to use clocktype since some router distributed backplane
    // throughput are larger than long type.
    clocktype disBackplaneCapacity;

#ifdef TRANSPORT_AND_HAIPE
    // For HAIPE specification
    HAIPESpec haipeSpec;
#endif // TRANSPORT_AND_HAIPE
#ifdef CYBER_LIB
    // Should count PHY turnaround time for the (wormhole victim) node?
    BOOL countWormholeVictimTurnaroundTime;
    // the turnaround time can also be specified by user
    clocktype wormholeVictimTurnaroundTime;

    // For eavesdropping
    FILE *eavesdropFile;

    // For auditing
    FILE *auditFile;

    // Each network interface/address has a valid certificate
    // to prove the authenticity of its key
    char *certificate;
    int certificateLength;
    BOOL certificateFileLog;

    // For jamming
    //PhyJammingData *jammingData;
#ifdef DO_ECC_CRYPTO
    // Each network interface/address has an ECC key
    MPI eccKey[12];  // the 1st 10 elements construct the public key
#endif // DO_ECC_CRYPTO
#endif // CYBER_LIB

    D_Int32 ipFragUnit;

    // Interface based stats. MIBS
    D_UInt32 ifInUcastPkts;
    D_UInt32 ifInNUcastPkts;
    D_String ifDescr;
    D_UInt32 ifOutUcastPkts;
    D_UInt32 ifOutNUcastPkts;
    D_UInt32 ifInMulticastPkts;
    D_UInt32 ifInBroadcastPkts;
    D_UInt32 ifOutMulticastPkts;
    D_UInt32 ifOutBroadcastPkts;
    D_UInt32 ifInDiscards;
    D_UInt32 ifOutDiscards;

    D_UInt64 ifHCInUcastPkts;
    D_UInt64 ifHCInMulticastPkts;
    D_UInt64 ifHCInBroadcastPkts;
    D_UInt64 ifHCOutUcastPkts;
    D_UInt64 ifHCOutMulticastPkts;
    D_UInt64 ifHCOutBroadcastPkts;

    D_UInt32 ipAddrIfIdx;
    D_NodeAddress ipAddrNetMask;
    D_UInt32 ipAddrBcast;

    // Stats for NAIL Analysis
    D_UInt32 ifInUcastDataPackets;
    D_UInt32 ifOutUcastDataPackets;
    D_UInt32 inUcastDataPacketSize;
    D_UInt32 inUcastPacketSize;
    D_UInt32 inNUcastPacketSize;
    D_UInt32 inMulticastPacketSize;
    D_UInt32 inBroadcastPacketSize;
    clocktype firstInUcastPacketTime;
    clocktype lastInUcastPacketTime;
    clocktype firstInUcastDataPacketTime;
    clocktype lastInUcastDataPacketTime;
    clocktype firstInNUcastPacketTime;
    clocktype lastInNUcastPacketTime;
    clocktype firstInMulticastPacketTime;
    clocktype lastInMulticastPacketTime;
    clocktype firstInBroadcastPacketTime;
    clocktype lastInBroadcastPacketTime;

    D_UInt32 outUcastDataPacketSize;
    D_UInt32 outUcastPacketSize;
    D_UInt32 outNUcastPacketSize;
    D_UInt32 outMulticastPacketSize;
    D_UInt32 outBroadcastPacketSize;
    clocktype firstOutUcastPacketTime;
    clocktype lastOutUcastPacketTime;
    clocktype firstOutUcastDataPacketTime;
    clocktype lastOutUcastDataPacketTime;
    clocktype firstOutNUcastPacketTime;
    clocktype lastOutNUcastPacketTime;
    clocktype firstOutMulticastPacketTime;
    clocktype lastOutMulticastPacketTime;
    clocktype firstOutBroadcastPacketTime;
    clocktype lastOutBroadcastPacketTime;

     unsigned char routingInstance;
#ifdef CYBER_CORE
    // For IPsec
    IPsecSecurityPolicyInformation* spdIN;
    IPsecSecurityPolicyInformation* spdOUT;
    int             iahepInterfaceType;
    IAHEPStatsType  iahepstats;
    D_Int32         iahepFragUnit;
    unsigned iahepEncapsulationOverheadSize;
    unsigned iahepControlEncryptionRate;
    unsigned iahepControlHMACRate;
    /***** Start: OPAQUE-LSA *****/
    NodeAddress     iahepDeviceAddress;
    /***** End: OPAQUE-LSA *****/
#endif // CYBER_CORE

#ifdef ADDON_DB
    MetaDataStruct* metaData;
#endif

    clocktype commsDelay;
    double commsDrop;

    // Dynamic address
    AddressState addressState; // current address state
    bool isDhcpEnabled; // Flag to check if DHCP enabled or not.
    Address primaryDnsServer; // allocated by DHCP server
    Address subnetMask; // allocated by DHCP server
    LinkedList*  listOfSecondaryDNSServer; // allocated by DHCP server
    // Dynamic address statistics
    UInt32 ipNumPktDropDueToInvalidAddressState; // app/network packets
                                                 // dropped due to INVALID
                                                 // address state

};

//----------------------------------------------------------
// IP Fragmentation structure.
//----------------------------------------------------------
/// QualNet typedefs struct ip_frag_data to
/// IpFragData. is a simple queue to hold
/// fragmented packets.
typedef struct ip_frag_data
{
    Message* msg;
    struct ip_frag_data* nextMsg;
}IpFragData;



/// Ipv6 fragment queue structure.
typedef struct ip_frag_q_struct
{
    UInt32 ipFrg_id;
    NodeAddress ipFrg_src;
    NodeAddress ipFrg_dst;
    clocktype fragHoldTime;
    IpFragData* firstMsg;
    IpFragData* lastMsg;

    unsigned int actualacketSize;
    unsigned int totalFragmentSize;

    struct ip_frag_q_struct* next;
}IpFragQueue;



/// QualNet typedefs struct fragmeted_msg_struct to
/// ip6q. struct fragmeted_msg_struct is a simple
/// fragmented packets msg hold structure.
struct ip_fragmeted_msg_struct
{
    Message* msg;
    struct ip_fragmeted_msg_struct* next;
};

typedef struct ip_fragmeted_msg_struct ipFragmetedMsg;

//----------------------------------------------------------
// IP
//----------------------------------------------------------

/// Main structure of network layer.
struct NetworkDataIp
{
    unsigned short              packetIdCounter;  // Used for identifying
    // datagram
    NetworkForwardingTable      forwardTable;

    BOOL                        checkMessagePeekFunction;
    BOOL                        checkMacAckHandler;
    int maxPacketLength;

    // added for Gateway
    BOOL gatewayConfigured;
    NodeAddress defaultGatewayId;

// For Dymo
    BOOL    isManetGateway;
    UInt8   manetPrefixlength;
    Address manetPrefixAddr;
// end for Dymo
    // Fragmentation disabled.
    // IpReassemblyBufferListType reassemblyBufferList;

    IpInterfaceInfoType* interfaceInfo[MAX_NUM_INTERFACES];

    IpPerHopBehaviorInfoType    *phbInfo;
    int                         numPhbInfo;
    int                         maxPhbInfo;

    IpMultiFieldTrafficConditionerInfo  *mftcInfo;
    int                                 numMftcInfo;
    int                                 maxMftcInfo;

    BOOL                        diffservEnabled;
    BOOL                        isEdgeRouter;
    BOOL                        mftcStatisticsEnabled;
    IpMftcParameter             *ipMftcParameter;
    int                         numIpMftcParameters;
    int                         maxIpMftcParameters;

    BOOL                        ipForwardingEnabled;

    NetworkIpStatsType stats;

    STAT_NetStatistics* newStats;

    LinkedList                          *multicastGroupList;
    NetworkMulticastForwardingTable     multicastForwardingTable;

    BOOL                                isIgmpEnable;
    IgmpData                            *igmpDataPtr;
    BOOL                                isSSMEnable;
    NodeAddress                         SSMStartGrpAddr;
    NodeAddress                         SSMLastGrpAddr;

    BOOL                                isIcmpEnable;
    void                                *icmpStruct;

    // BEGIN start NDP
    BOOL                                isNdpEnable;
    void                                *ndpData;
    // END start NDP


    // BEGIN start ROUTING_CES_SDR
    // END start ROUTING_CES_SDR
    // ECN related variables
    BOOL                                isPacketEcnCapable;

    NetworkRouteUpdateEventType routeUpdateFunction;

    BOOL traceEnabled;

    // Need to use clocktype since some router backplane throughput
    // are larger than long type.
    clocktype backplaneThroughputCapacity;

    Scheduler* cpuScheduler;
    NetworkIpBackplaneStatusType backplaneStatus;

#ifdef ENTERPRISE_LIB
    // Keep track of all name value lists
    AccessListParameterNameValue* allNameValueLists;

    // Keep track of head of access list.
    // This is for number access list.
    AccessList** accessList;

    // This is for name access list
    AccessListName* accessListName;

    // Has got the nested reflex in the next
    AccessListNest* nestReflex;

    // Keep track of the existing sessions
    AccessListSessionList* accessListSession;

    BOOL accessListTrace;

    // Used to determine if access list stats are printed at the end of
    // simulation.
    BOOL isACLStatOn;

    // List of route maps for this node
    RouteMapList* routeMapList;

    RouteMapHookList* rtMapList;

    // Link to the last route map entry
    RouteMapEntry* bufferLastEntryRMap;

    RouteRedistribute*      rtRedistributeInfo;
    BOOL                    rtRedistributeIsEnabled;
    BOOL                    isRtRedistributeStatOn;

    // For policy based routing
    RouteMap* rMapForPbr;
    // Is local PBR enabled
    BOOL local;

    BOOL isPBRStatOn;
    BOOL pbrTrace;

    // one more pair of stats for local PBR
    PbrStat pbrStat;
#endif // ENTERPRISE_LIB

    // Timeout time for reflex
    clocktype reflexTimeout;

    // IPv6 data structure
    struct ipv6_data_struct* ipv6;

    // dualIp data structure
    struct dual_ip_data_struct* dualIp;

    void *mobileIpStruct;

    // Fragment Id counter.
    UInt32 ipFragmentId;
    IpFragQueue* fragmentListFirst;
    clocktype ipFragHoldTime;

    D_Int32 ipFragUnit;
    // Loopback
    BOOL isLoopbackEnabled;
    NetworkForwardingTable loopbackFwdTable;

#ifdef CYBER_LIB
    // Each node has a (per-node) secure neighbor list
    BOOL isSecureneighborEnabled;
    SecureneighborData* neighborData;

    //BOOL isSecureCommunityEnabled;  // for Routing Security
    //SecureCommunityData* communityData;
#endif

#ifdef CYBER_CORE
    BOOL isIPsecEnabled;
    BOOL isOpenSSLEnabled;
    IPsecSecurityAssociationDatabase* sad;
    BOOL         iahepEnabled;
    IAHEPData*   iahepData;
    // Fragment Id counter.
    UInt32       iahepFragmentId;
    IpFragQueue* iahepfragmentListFirst;
    BOOL isExchangeRunning;
#endif // CYBER_CORE



#ifdef ADDON_NGCNMS
    BOOL collectRoutes;
    char malsrRouteFileName0[MAX_STRING_LENGTH];
    char rospfRouteFileName0[MAX_STRING_LENGTH];
    char malsrRouteFileName1[MAX_STRING_LENGTH];
    char rospfRouteFileName1[MAX_STRING_LENGTH];
    clocktype collectRoutesUpdateInterval;
    NodeAddress gatewayAddress;
    int numSubnets;
    BOOL outputStats;
    NetworkNgcHaipeData* haipeData;
#endif

    // Keep backplane type.information
    BackplaneType backplaneType;

    // This is here because diffserv doesn't have it's own data structure
    // and I don't want to create one.
    RandomSeed trafficConditionerSeed;


    // STATS DB CODE
#ifdef ADDON_DB

    // For Network Session Table
    NetworkSumAggrData *oneHopData ;
    // For Multicast Network Summary Table
    StatsDBMulticastNetworkSummaryContent*  ipMulticastNetSummaryStats;
#endif

#ifdef CYBER_CORE
//BROADCAST_IAHEP_START
    BOOL isAppBroadcastForwardingEnabled;
    map<Int64,clocktype>* broadcastAppMapping;
    clocktype broadcastForwardingTimeout;
//BROADCAST_IAHEP_END
#endif // CYBER_CORE



    NetworkIpFilter *filters;
    int numFilters;

    // Dynamic address
    LinkedList* addressChangedHandlerList;
};

//----------------------------------------------------------
// INLINED FUNCTIONS
//----------------------------------------------------------

//----------------------------------------------------------
// IP address
//----------------------------------------------------------


/// To generate subnetmask using number of host bit
///
/// \param numHostBits  number of host bit.
///
/// \return subnetmask
inline NodeAddress
ConvertNumHostBitsToSubnetMask(int numHostBits)
{
    if (numHostBits == 32)
    {
        return 0;
    }

    return 0xffffffff << numHostBits;
}

/// To generate number of host bit using subnetmask.
///
/// \param subnetMask  subnetmask.
///
/// \return number of host bit.
inline int
ConvertSubnetMaskToNumHostBits(NodeAddress subnetMask)
{
    int numHostBits = 0;
    unsigned mask = 1;

    if (subnetMask == 0)
    {
        return 32;
    }
    while (1)
    {
        if (subnetMask & mask)
        {
            return numHostBits;
        }
        numHostBits++;
        mask = mask << 1;
    }
}

/// To mask a ip address.
///
/// \param address  address of a node
/// \param mask  mask of subnet.
///
/// \return masked node address.
inline NodeAddress
MaskIpAddress(NodeAddress address, NodeAddress mask)
{
    return (address & mask);
}

/// To mask a ip address.
///
/// \param address  address of a node.
/// \param numHostBits  number of host bit.
///
/// \return masked node address.
inline NodeAddress
MaskIpAddressWithNumHostBits(NodeAddress address, int numHostBits)
{
    return MaskIpAddress(address,
                         ConvertNumHostBitsToSubnetMask(numHostBits));
}

/// To generate broadcast address.
///
/// \param address  address of a node.
/// \param numHostBits  number of host bit.
///
/// \return Broadcast address.
inline NodeAddress
CalcBroadcastIpAddress(NodeAddress address, int numHostBits)
{
    int i;
    if (numHostBits == 0)
    {
        return address;
    }
    for (i = 0; i < numHostBits; i++)
    {
        address |= (1 << i);
    }

    return address;
}

/// To check if a ip address belongs to a subnet.
///
/// \param address  address of a node.
/// \param subnetAddress  address of a subnet.
/// \param numHostbits  number of host bit.
///
/// \return TRUE if ip address belongs to a subnet
/// else FALSE.
inline BOOL
IsIpAddressInSubnet(
    NodeAddress address,
    NodeAddress subnetAddress,
    int numHostBits)
{
    NodeAddress subnetMask = ConvertNumHostBitsToSubnetMask(numHostBits);
    return (BOOL) (MaskIpAddress(address, subnetMask) == subnetAddress);
}

//----------------------------------------------------------
// IP header
//----------------------------------------------------------

/// Add an IP packet header to a message.
/// Just calls AddIpHeader.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message.
/// \param sourceAddress  Source IP address.
/// \param destinationAddress  Destination IP address.
/// \param priority  Currently a TosType.
///    (values are not standard for "IP type of
///    service field" but has correct function)
/// \param protocol  IP protocol number.
/// \param ttl  Time to live.If 0, uses default
///    value IPDEFTTL, as defined in include/ip.h.
void
NetworkIpAddHeader(
    Node *node,
    Message *msg,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    TosType priority,
    unsigned char protocol,
    unsigned ttl);

/// Searches the IP header for the option field with option
/// code that matches optionKey, and returns a pointer to
/// the option field header.
///
/// \param ipHeader  Pointer to an IP header.
/// \param optionKey  Option code for desired option field.
///
/// \return to the header of the desired option
/// field. NULL if no option fields, or
/// the desired option field cannot be
/// found.
inline IpOptionsHeaderType *
FindAnIpOptionField(
    const IpHeaderType *ipHeader,
    const int optionKey)
{
    IpOptionsHeaderType *currentOption;

    // If the passed in IP header is the minimum size, return NULL.
    // (no option in IP header)
    if (IpHeaderSize(ipHeader) == sizeof(IpHeaderType))
    {
        return NULL;
    }

    // Move pointer over 20 bytes from start of IP header,
    // so currentOption points to first option.
    currentOption = (IpOptionsHeaderType *)
                    ((char *) ipHeader + sizeof(IpHeaderType));
    // Loop until an option code matches optionKey.
    while (currentOption->code != optionKey)
    {
        // Current option code doesn't match; move pointer over to next
        // option.
        if (currentOption->code == IPOPT_NOP)
        {
            currentOption = (IpOptionsHeaderType *)
                            ((char *) currentOption + 1);
            continue;
        }
        // Options should never report their length as 0.
        if (currentOption->len == 0)
        {
            return NULL;
        }
        currentOption =
            (IpOptionsHeaderType *) ((char *) currentOption +
                                                         currentOption->len);

        // If we've run out of options, return NULL.
        if ((char *) currentOption
                >= (char *) ipHeader + IpHeaderSize(ipHeader)||
            *((char *) currentOption) == IPOPT_EOL)
        {
            return NULL;
        }
    }

    // Found option with code matching optionKey.  Return pointer to option.

    return currentOption;
}


//----------------------------------------------------------
// PROTOTYPES FOR FUNCTIONS WITH EXTERNAL LINKAGE
//----------------------------------------------------------

//----------------------------------------------------------
// Init functions
//----------------------------------------------------------

/// IP initialization required before any of the other
/// layers are initialized.
/// This is mainly for MAC initialization, which requires
/// certain IP structures be pre-initialized.
///
/// \param node  pointer to node.
void
NetworkIpPreInit(Node *node);

/// Initialize IP variables, and all network-layer IP
/// protocols..
///
/// \param node  pointer to node.
/// \param nodeInput  Pointer to node input.
void
NetworkIpInit(Node *node, const NodeInput *nodeInput);

//-----------------------------------------------------------------------------
// FUNCTION     NetworkIpInitInterfaceStatsSNMP()
// PURPOSE      Initialize interface based IP stats
// PARAMETERS   Node *node
//                  Pointer to node.
// RETURN       None.
//-----------------------------------------------------------------------------

void
NetworkIpInitInterfaceStatsSNMP(Node* node);


//----------------------------------------------------------
// Layer function
//----------------------------------------------------------


/// Handle IP layer events, incoming messages and messages
/// sent to itself (timers, etc.).
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message.
void
NetworkIpLayer(Node *node, Message *msg);

//----------------------------------------------------------
// Finalize function
//----------------------------------------------------------


/// Finalize function for the IP model.  Finalize functions
/// for all network-layer IP protocols are called here.
///
/// \param node  Pointer to node.
void
NetworkIpFinalize(Node *node);

//----------------------------------------------------------
// Transport layer to IP, sends IP packets out to network
//----------------------------------------------------------


/// Called by transport layer protocols (UDP, TCP) to send
/// UDP datagrams and TCP segments using IP.  Simply calls
/// NetworkIpSendRawMessage().
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message from transport
///    layer containing payload data (UDP datagram, TCP
///    segment) for an IP packet. (IP header needs to be
///    added)
/// \param sourceAddress  Source IP address.
///    See NetworkIpSendRawMessage() for special values
/// \param destinationAddress  Destination IP address.
/// \param outgoingInterface  outgoing interface to use to
///    transmit packet.
/// \param priority  Priority of packet.
/// \param protocol  IP protocol number.
/// \param isEcnCapable  Is this node ECN capable?
void
NetworkIpReceivePacketFromTransportLayer(
    Node *node,
    Message *msg,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    int outgoingInterface,
    TosType priority,
    unsigned char protocol,
    BOOL isEcnCapable,
    UInt8 ttl = IPDEFTTL);


void
NetworkIpReceivePacketFromTransportLayer(
    Node *node,
    Message *msg,
    Address sourceAddress,
    Address destinationAddress,
    int outgoingInterface,
    TosType priority,
    unsigned char protocol,
    BOOL isEcnCapable,
    UInt8 ttl = IPDEFTTL);

//----------------------------------------------------------
// Network layer to MAC layer, sends IP packets out to network
//----------------------------------------------------------


/// Called by NetworkIpReceivePacketFromTransportLayer() to
/// send to send UDP datagrams, TCP segments using IP.  Also
/// called by network-layer routing protocols (AODV, OSPF,
/// etc.) to send IP packets.  This function adds an IP
/// header and calls RoutePacketAndSendToMac().
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with payload data
///    for IP packet.(IP header needs to
///    be added)
/// \param sourceAddress  Source IP address.
///    If sourceAddress is ANY_IP, lets IP assign the
///    source  address (depends on the route).
/// \param destinationAddress  Destination IP address.
/// \param outgoingInterface  outgoing interface to use to
///    transmit packet.
/// \param priority  Priority of packet.
/// \param protocol  IP protocol number.
/// \param ttl  Time to live.
///    See AddIpHeader() for special values.
void
NetworkIpSendRawMessage(
    Node *node,
    Message *msg,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    int outgoingInterface,
    TosType priority,
    unsigned char protocol,
    unsigned ttl);


/// Same as NetworkIpSendRawMessage(), but schedules
/// event after a simulation delay.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with payload data
///    for IP packet.(IP header needs to
///    be added)
/// \param sourceAddress  Source IP address.
///    If sourceAddress is ANY_IP, lets IP assign the
///    source  address (depends on the route).
/// \param destinationAddress  Destination IP address.
/// \param outgoingInterface  outgoing interface to use to
///    transmit packet.
/// \param priority  TOS of packet.
/// \param protocol  IP protocol number.
/// \param ttl  Time to live.
///    See AddIpHeader() for special values.
/// \param delay  Delay
void
NetworkIpSendRawMessageWithDelay(
    Node *node,
    Message *msg,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    int outgoingInterface,
    TosType priority,
    unsigned char protocol,
    unsigned ttl,
    clocktype delay);


/// Called by network-layer routing protocols (AODV, OSPF,
/// etc.) to add an IP header to payload data, and with
/// the resulting IP packet, calls
/// NetworkIpSendPacketOnInterface().
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with payload data
///    for IP packet.(IP header needs to
///    be added)
/// \param sourceAddress  Source IP address.
/// \param destinationAddress  Destination IP address.
/// \param priority  TOS of packet.
/// \param protocol  IP protocol number.
/// \param ttl  Time to live.
///    See AddIpHeader() for special values.
/// \param interfaceIndex  Index of outgoing interface.
/// \param nextHop  Next hop IP address.
void
NetworkIpSendRawMessageToMacLayer(
    Node *node,
    Message *msg,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    TosType priority,
    unsigned char protocol,
    unsigned ttl,
    int interfaceIndex,
    NodeAddress nextHop);


/// Same as NetworkIpSendRawMessageToMacLayer(),
/// but schedules the event after a simulation delay
/// by calling NetworkIpSendPacketOnInterfaceWithDelay().
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with payload data
///    for IP packet.(IP header needs to
///    be added)
/// \param sourceAddress  Source IP address.
/// \param destinationAddress  Destination IP address.
/// \param priority  TOS of packet.
/// \param protocol  IP protocol number.
/// \param ttl  Time to live.
///    See AddIpHeader() for special values.
/// \param interfaceIndex  Index of outgoing interface.
/// \param nextHop  Next hop IP address.
/// \param delay  delay.
void
NetworkIpSendRawMessageToMacLayerWithDelay(
    Node *node,
    Message *msg,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    TosType priority,
    unsigned char protocol,
    unsigned ttl,
    int interfaceIndex,
    NodeAddress nextHop,
    clocktype delay);


/// This function is called once the outgoing interface
/// index and next hop address to which to route an IP
/// packet are known.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param interfaceIndex  Index of outgoing interface.
/// \param nextHop  Next hop IP address.
void
NetworkIpSendPacketToMacLayer(
    Node *node,
    Message *msg,
    int interfaceIndex,
    NodeAddress nextHop);


/// This function is called once the outgoing interface
/// index and next hop address to which to route an IP
/// packet are known.  This queues an IP packet for delivery
/// to the MAC layer.  This functions calls
/// QueueUpIpFragmentForMacLayer().
/// This function is used to initiate fragmentation if
/// required, but since fragmentation has been disabled, all
/// it does is assert false if the IP packet is too big
/// before calling the next function.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param incomingInterface  Index of incoming interface.
/// \param outgoingInterface  Index of outgoing interface.
/// \param nextHop  Next hop IP address.
void
NetworkIpSendPacketOnInterface(
    Node *node,
    Message *msg,
    int incomingInterface,
    int outgoingInterface,
    NodeAddress nextHop);

/// Same as NetworkIpSendPacketOnInterface(), but schedules
/// event after a simulation delay.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param interfaceIndex  Index of outgoing interface.
/// \param nextHop  Next hop IP address.
/// \param delay  delay
void
NetworkIpSendPacketToMacLayerWithDelay(
    Node *node,
    Message *msg,
    int interfaceIndex,
    NodeAddress nextHop,
    clocktype delay);


/// Same as NetworkIpSendPacketOnInterface(), but schedules
/// event after a simulation delay.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param incommingInterface  Index of incomming interface.
/// \param outgoingInterface  Index of outgoing interface.
/// \param nextHop  Next hop IP address.
/// \param delay  delay
void
NetworkIpSendPacketOnInterfaceWithDelay(
    Node *node,
    Message *msg,
    int incomingInterface,
    int outgoingInterface,
    NodeAddress nextHop,
    clocktype delay);

#ifdef ADDON_DB
/// Same as NetworkIpSendPacketOnInterface(), but schedules
/// event after a simulation delay and denotes raw packet.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param incommingInterface  Index of incomming interface.
/// \param outgoingInterface  Index of outgoing interface.
/// \param nextHop  Next hop IP address.
/// \param delay  delay
void
NetworkIpSendRawPacketOnInterfaceWithDelay(
    Node *node,
    Message *msg,
    int incomingInterface,
    int outgoingInterface,
    NodeAddress nextHop,
    clocktype delay);
#endif  // ADDON_DB


/// Tacks on a new source route to an existing IP packet and
/// then sends the packet to the MAC layer.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param newRouteAddresses  Source route (address array).
/// \param numNewRouteAddresses  Number of array elements.
/// \param removeExistingRecordedRoute  Flag to indicate previous record
///    route should be removed or not
///    TRUE ->removed else FALSE
void
NetworkIpSendPacketToMacLayerWithNewStrictSourceRoute(
    Node *node,
    Message *msg,
    NodeAddress newRouteAddresses[],
    int numNewRouteAddresses,
    BOOL removeExistingRecordedRoute);

//----------------------------------------------------------
// MAC layer to IP, receives IP packets from the MAC layer
//----------------------------------------------------------

/// IP received IP packet from MAC layer. Updates the Stats
/// database and then calls NetworkIpReceivePacket.
///
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param previousHopNodeId  nodeId of the previous hop.
/// \param interfaceIndex  Index of interface on which packet arrived.
void
NetworkIpReceivePacketFromMacLayer(
    Node *node,
    Message *msg,
    NodeAddress previousHopNodeId,
    int interfaceIndex);

/// IP received IP packet.  Determine whether
/// the packet is to be delivered to this node, or needs to
/// be forwarded.
/// ipHeader->ip_ttl is decremented here, instead of the
/// way BSD TCP/IP does it, which is to decrement TTL right
/// before forwarding the packet.  QualNet's alternative
/// method suits its network-layer ad hoc routing protocols,
/// which may do their own forwarding.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param previousHopNodeId  nodeId of the previous hop.
/// \param interfaceIndex  Index of interface on which packet arrived.
void
NetworkIpReceivePacket(
    Node *node,
    Message *msg,
    NodeAddress previousHopNodeId,
    int interfaceIndex);
//----------------------------------------------------------
// MAC-layer packet drop callbacks
//----------------------------------------------------------

/// Invoke callback functions when a packet is dropped.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param nextHopNodeAddres  next hop address of dropped packet.
/// \param interfaceIndex  interface that experienced the packet drop.
void
NetworkIpNotificationOfPacketDrop(Node *node,
                                  Message *msg,
                                  NodeAddress nextHopAddres,
                                  int interfaceIndex);

/// Get the status event handler function pointer.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface associated with the status
///    handler function.
///
/// \return Status event
/// handler function.
MacLayerStatusEventHandlerFunctionType
NetworkIpGetMacLayerStatusEventHandlerFunction(
    Node *node,
    int interfaceIndex);

/// Allows the MAC layer to send status messages (e.g.,
/// packet drop, link failure) to a network-layer routing
/// protocol for routing optimization.
///
/// \param node  Pointer to node.
/// \param StatusEventHandlerPtr  interface
///    that experienced the packet drop.
/// \param interfaceIndex  interface associated with the status
///    event handler.
void
NetworkIpSetMacLayerStatusEventHandlerFunction(
    Node *node,
    MacLayerStatusEventHandlerFunctionType StatusEventHandlerPtr,
    int interfaceIndex);

//------------------------------------------------------------------------
// MAC-layer promiscuous mode callbacks
//------------------------------------------------------------------------

/// Called Directly by the MAC layer, this allows a routing
/// protocol to "sneak a peek" or "tap" messages it would not
/// normally see from the MAC layer.  This function will
/// possibly unfragment such packets and call the function
/// registered by the routing protocol to do the "Peek".
///
/// \param node  Pointer to node.
/// \param msg  The message being peeked at from the
///    MAC layer.Must not be freed
///    or modified!.
/// \param interfaceIndex  The interface of which the "peeked" message belongs to.
///    packet drop.
/// \param prevHop  next hop address of dropped packet.
void
NetworkIpSneakPeekAtMacPacket(Node *node,
                              const Message *msg,
                              int interfaceIndex,
                              MacHWAddress& prevHopHwAddr);

/// Returns the network-layer function which will
/// promiscuously inspect packets.
/// See NetworkIpSneakPeekAtMacPacket().
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface associated with the peek function.
///
/// \return Function pointer
PromiscuousMessagePeekFunctionType
NetworkIpGetPromiscuousMessagePeekFunction(
    Node *node,
    int interfaceIndex);

/// Sets the network-layer function which will
/// promiscuously inspect packets.
/// See NetworkIpSneakPeekAtMacPacket().
///
/// \param node  Pointer to node.
/// \param PeekFunctionPtr  Peek function.
/// \param interfaceIndex  Interface associated with the peek function.
void
NetworkIpSetPromiscuousMessagePeekFunction(
    Node *node,
    PromiscuousMessagePeekFunctionType PeekFunctionPtr,
    int interfaceIndex);

// Mac layer acknowledgement handlers

/// MAC received an ACK, so call ACK handler function.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface associated with the  ACK handler function.
/// \param msg  Message that was ACKed.
/// \param nextHop  Next hop that sent the MAC layer ACK
void
NetworkIpReceiveMacAck(
    Node* node,
    int interfaceIndex,
    const Message* msg,
    NodeAddress nextHop);

/// Get MAC layer ACK handler
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface associated with ACK handler function
///
/// \return MAC acknowledgement function pointer
MacLayerAckHandlerType
NetworkIpGetMacLayerAckHandler(
    Node *node,
    int interfaceIndex);

/// Set MAC layer ACK handler
///
/// \param node  Pointer to node.
/// \param macAckHandlerPtr  Callback function handling
///    mac layer acknowledgement that it has
///    successfully transmitted one packet
/// \param interfaceIndex  Interface associated with the  ACK handler
///    function.
void
NetworkIpSetMacLayerAckHandler(
    Node *node,
    MacLayerAckHandlerType macAckHandlerPtr,
    int interfaceIndex);

//----------------------------------------------------------
// Network layer to transport layer
//----------------------------------------------------------
//
// ODMRP is the only model which uses these functions outside of ip.c!
//

/// Sends a UDP packet to UDP in the transport layer.
/// The source IP address, destination IP address, and
/// priority of the packet are also sent.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with UDP packet.
/// \param priority  TOS of UDP packet.
/// \param sourceAddress  Source IP address.
/// \param destinationAddress  Destination IP address.
/// \param incomingInterfaceIndex  interface that received the packet
void
SendToUdp(
    Node *node,
    Message *msg,
    TosType priority,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    int incomingInterfaceIndex);

/// Sends a TCP packet to TCP in the transport layer.
/// The source IP address, destination IP address, and
/// priority of the packet are also sent..
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with TCP packet.
/// \param priority  TOS of TCP packet.
/// \param sourceAddress  Source IP address.
/// \param destinationAddress  Destination IP address.
/// \param aCongestionExperienced  Determine if congestion is
///    experienced (via ECN)..
void
SendToTcp(
    Node *node,
    Message *msg,
    TosType priority,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    BOOL aCongestionExperienced);

/// Sends a RSVP packet to RSVP in the transport layer.
/// The source IP address, destination IP address, and
/// priority of the packet are also sent.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with RSVP packet.
/// \param priority  TOS of UDP packet.
/// \param sourceAddress  Source IP address.
/// \param destinationAddress  Destination IP address.
/// \param interfaceIndex  incoming interface index.
/// \param ttl  Receiving TTL
void
SendToRsvp(
    Node *node,
    Message *msg,
    TosType priority,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    int interfaceIndex,
    unsigned ttl);


//-----------------------------------------------------------------------
// IP header
//-----------------------------------------------------------------------

/// Removes the IP header from a message while also
/// returning all the fields of the header.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message
/// \param sourceAddress  Storage for source IP address.
/// \param destinationAddress  Storage for destination IP
///    address.
/// \param priority  Storage for TosType.(values are
///    not standard for "IP type of service field"
///    but has correct function)
/// \param protocol  Storage for IP protocol number
/// \param ttl  Storage for time to live.
void
NetworkIpRemoveIpHeader(
    Node *node,
    Message *msg,
    NodeAddress *sourceAddress,
    NodeAddress *destinationAddress,
    TosType *priority,
    unsigned char *protocol,
    unsigned *ttl);

//----------------------------------------------------------
// IP header option field
//----------------------------------------------------------

//
// ODMRP is the only model which uses these functions outside of ip.c!
//

/// Inserts an option field in the header of an IP packet.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message
/// \param optionCode  The option code
/// \param optionSize  Size of the option
void
AddIpOptionField(
    Node *node,
    Message *msg,
    int optionCode,
    int optionSize);

//----------------------------------------------------------
// Source route
//----------------------------------------------------------

/// Retrieves a copy of the source and recorded route from
/// the options field in the header.
///
/// \param msg  Pointer to message with IP packet.
/// \param RouteAddresses  Storage for source/recorded route.
/// \param NumAddresses  Storage for size of RouteAddresses[] array.
/// \param RouteAddressIndex  The index of the first address of the
///    source  route; before this index is the
///    recorded route.
void
ExtractIpSourceAndRecordedRoute(
    Message *msg,
    NodeAddress RouteAddresses[],
    int *NumAddresses,
    int *RouteAddressIndex);

//----------------------------------------------------------
// Router info helper functions
//----------------------------------------------------------

/// Get the router function pointer.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface associated with router function
///
/// \return router function pointer.
RouterFunctionType
NetworkIpGetRouterFunction(
    Node *node,
    int interfaceIndex);

/// Allows a routing protocol to set the "routing function"
/// (one of its functions) which is called when a packet
/// needs to be routed.
/// NetworkIpSetRouterFunction() allows a routing protocol
/// to define the routing function.  The routing function
/// is called by the network layer to ask the routing
/// protocol to route the packet.  The routing function is
/// given the packet and its destination.  The routing
/// protocol can route the packet and set "PacketWasRouted"
/// to TRUE; or not route the packet and set to FALSE.  If
/// the packet, was not routed, then the network layer will
/// try to use the forwarding table or the source route the
/// source route in the IP header.  This function will also
/// be given packets for the local node the routing
/// protocols can look at packets for protocol reasons.  In
/// this case, the message should not be modified and
/// PacketWasRouted must be set to FALSE.
///
/// \param node  Pointer to node.
/// \param RouterFunctionPtr  Router function to set.
/// \param interfaceIndex  interface associated with router function.
void
NetworkIpSetRouterFunction(
    Node *node,
    RouterFunctionType RouterFunctionPtr,
    int interfaceIndex);


/// Add unicast routing protocol type to interface.
///
/// \param node  Pointer to node.
/// \param routingProtocolType  Router function to add.
/// \param interfaceIndex  Interface associated with the router function.
void
NetworkIpAddUnicastRoutingProtocolType(
    Node *node,
    NetworkRoutingProtocolType routingProtocolType,
    int interfaceIndex,
    NetworkType networkType = NETWORK_IPV4);


/// Get routing protocol structure associated with
/// routing protocol
/// running on this interface.
///
/// \param node  Pointer to node.
/// \param routingProtocolType  Router function to
///    retrieve.
///
/// \return Routing protocol structure requested.
void *
NetworkIpGetRoutingProtocol(
    Node *node,
    NetworkRoutingProtocolType routingProtocolType,
    NetworkType networkType = NETWORK_IPV4);

/// Get unicast routing protocol type on this interface.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  network interface for request.
///
/// \return The unicast routing
/// protocol type.
NetworkRoutingProtocolType
NetworkIpGetUnicastRoutingProtocolType(
    Node *node,
    int interfaceIndex,
    NetworkType networkType = NETWORK_IPV4);

/// To enable hsrp on a interface
///
/// \param node  Pointer to node.
/// \param interfaceIndex  network interface.
void NetworkIpSetHsrpOnInterface(Node *node, int interfaceIndex);


/// To test if any interface is hsrp enabled.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  network interface.
///
/// \return return TRUE if any one interface is hsrp enabled
/// else return FALSE.
BOOL NetworkIpIsHsrpEnabled(Node *node, int interfaceIndex);


//----------------------------------------------------------
// Interface creation
//----------------------------------------------------------

/// Add new interface to node.
///
/// \param node  Pointer to node.
/// \param interfaceIpAddress  Interface to add.
/// \param numHostBits  Number of host bits for the interface.
/// \param newInterfaceIndex  The interface number of the new interface.
/// \param nodeInput  Provides access to
///    configuration file.
void
NetworkIpAddNewInterface(
    Node *node,
    NodeAddress interfaceIpAddress,
    int numHostBits,
    int *newInterfaceIndex,
    const NodeInput *nodeInput,
    BOOL isNewInterface);

//----------------------------------------------------------
// Queue setup
//----------------------------------------------------------

void //inline//
IpInitPerHopBehaviors(
    Node *node,
    const NodeInput *nodeInput);

/// Initializes cpu queue parameters during startup.
///
/// \param node  Pointer to node.
/// \param nodeInput  Pointer to node input.
/// */
/// static
void
NetworkIpInitCpuQueueConfiguration(
    Node *node,
    const NodeInput *nodeInput);


/// Initializes input queue parameters during startup.
///
/// \param node  Pointer to node.
/// \param nodeInput  Pointer to node input.
/// \param interfaceIndex  interface associated with queue.
void
NetworkIpInitInputQueueConfiguration(
    Node *node,
    const NodeInput *nodeInput,
    int interfaceIndex);


/// Initializes queue parameters during startup.
///
/// \param node  Pointer to node.
/// \param nodeInput  Pointer to node input.
/// \param interfaceIndex  interface associated with queue.
void
NetworkIpInitOutputQueueConfiguration(
    Node *node,
    const NodeInput *nodeInput,
    int interfaceIndex);


/// Initializes input and output queue parameters during startup
///
/// \param node  Pointer to node.
/// \param nodeInput  Pointer to node input.
/// \param interfaceIndex  interface associated with queue.
void
NetworkIpCreateQueues(
    Node *node,
    const NodeInput *nodeInput,
    int interfaceIndex);


/// Initialize the scheduler parameters and also allocate
/// memory for queues if require.
///
/// \param schedulerPtr  pointer to schedular
/// \param numPriorities  Number of priorities available
/// \param queue  pointer to ip queue.
void
NetworkIpSchedulerParameterInit(
    Scheduler *schedulerPtr,
    const int numPriorities,
    Queue *queuePtr);


/// Call initialization function for appropriate scheduler.
///
/// \param node  pointer to node
/// \param nodeInput  pointer to nodeinput
/// \param interfaceIndex  interface index
/// \param schedulerPtr  type of Scheduler
/// \param schedulerTypeString  Scheduler name
void
NetworkIpSchedulerInit(
    Node *node,
    const NodeInput *nodeInput,
    int interfaceIndex,
    Scheduler *schedulerPtr,
    const char *schedulerTypeString);

//----------------------------------------------------------
// Network-layer enqueueing
//----------------------------------------------------------


/// Calls the cpu packet scheduler for an interface to retrieve
/// an IP packet from a queue associated with the interface.
/// The dequeued packet, since it's already been routed,
/// has an associated next-hop IP address.  The packet's
/// priority value is also returned.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with IP packet.
/// \param nextHopAddress  Packet's next hop address.
/// \param destinationAddress  Packet's destination address.
/// \param outgoingInterface  Used to determine where packet
///    should go after passing through
///    the backplane.
/// \param networkType  Type of network packet is using (IP,
///    Link-16, ...)
/// \param queueIsFull  Storage for boolean indicator.
///    If TRUE, packet was not queued because
///    scheduler reported queue was
///    (or queues were) full.
/// \param incomingInterface  Incoming interface of packet.
void
NetworkIpCpuQueueInsert(
    Node *node,
    Message *msg,
    NodeAddress nextHopAddress,
    NodeAddress destinationAddress,
    int outgoingInterface,
    int networkType,
    BOOL *queueIsFull,
    int incomingInterface = ANY_INTERFACE);


/// Calls input packet scheduler for an interface to retrieve
/// an IP packet from a queue associated with the interface.
/// The dequeued packet, since it's already been routed,
/// has an associated next-hop IP address.  The packet's
/// priority value is also returned.
///
/// \param node  Pointer to node.
/// \param incomingInterface  interface of input queue.
/// \param msg  Pointer to message with IP packet.
/// \param nextHopAddress  Packet's next hop address.
/// \param destinationAddress  Packet's destination address.
/// \param outgoingInterface  Used to determine where packet
///    should go after passing through
///    the backplane.
/// \param networkType  Type of network packet is using (IP,
///    Link-16, ...)
/// \param queueIsFull  Storage for boolean indicator.
///    If TRUE, packet was not queued because
///    scheduler reported queue was
///    (or queues were) full.
void
NetworkIpInputQueueInsert(
    Node *node,
    int incomingInterface,
    Message *msg,
    NodeAddress nextHopAddress,
    NodeAddress destinationAddress,
    int outgoingInterface,
    int networkType,
    BOOL *queueIsFull);


/// Calls output packet scheduler for an interface to retrieve
/// an IP packet from a queue associated with the interface.
/// The dequeued packet, since it's already been routed,
/// has an associated next-hop IP address.  The packet's
/// priority value is also returned.
/// Called by QueueUpIpFragmentForMacLayer().
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface of input queue.
/// \param msg  Pointer to message with IP packet.
/// \param nextHopAddress  Packet's next hop address.
/// \param destinationAddress  Packet's destination address.
/// \param networkType  Type of network packet is using (IP,
///    Link-16, ...)
/// \param queueIsFull  Storage for boolean indicator.
///    If TRUE, packet was not queued because
///    scheduler reported queue was
///    (or queues were) full.
void
NetworkIpOutputQueueInsert(
    Node *node,
    int interfaceIndex,
    Message *msg,
    NodeAddress nextHopAddress,
    NodeAddress destinationAddress,
    int networkType,
    BOOL *queueIsFull);


//----------------------------------------------------------
// Network-layer dequeueing
//----------------------------------------------------------


/// Calls the packet scheduler for an interface to retrieve
/// an IP packet from the input queue associated with the
/// interface.
///
/// \param node  Pointer to node.
/// \param incomingInterface  interface to dequeue from.
/// \param msg  Storage for pointer to message
///    with IP packet.
/// \param nextHopAddress  Storage for Packet's
///    next hop address.
/// \param outgoingInterface  Used to determine where packet
///    should go after passing
///    through the backplane.
/// \param networkType  Type of network packet is using (IP,
///    Link-16, ...)
/// \param priority  Storage for
///    priority of packet.
///
/// \return TRUE if dequeued successfully, FALSE otherwise.
BOOL NetworkIpInputQueueDequeuePacket(
    Node *node,
    int incomingIterface,
    Message **msg,
    NodeAddress *nextHopAddress,
    MacHWAddress *nexthopmacAddr,
    int *outgoingInterface,
    int *networkType,
    QueuePriorityType *priority);


/// Calls the packet scheduler for an interface to retrieve
/// an IP packet from a queue associated with the interface.
/// The dequeued packet, since it's already been routed,
/// has an associated next-hop IP address.  The packet's
/// priority value is also returned.
/// This function is called by
///
/// MAC_OutputQueueDequeuePacket() (mac/mac.pc), which itself
/// is called from mac/mac_802_11.pc and other MAC protocol
/// source files.
///
/// This function will assert false if the scheduler cannot
/// return an IP packet for whatever reason.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  index to interface .
/// \param msg  Storage for pointer to message
///    with IP packet.
/// \param nextHopAddress  Storage for Packet's next hop address.
/// \param networkType  Type of network packet is using (IP,
///    Link-16, ...)
/// \param priority  Storage for priority
///    of packet.
///
/// \return TRUE if dequeued successfully, FALSE otherwise.
BOOL NetworkIpOutputQueueDequeuePacket(
    Node *node,
    int interfaceIndex,
    Message **msg,
    NodeAddress *nextHopAddress,
    MacHWAddress *nexthopmacAddr,
    int *networkType,
    QueuePriorityType *priority);


/// Same as NetworkIpOutputQueueDequeuePacket(), except the
/// packet dequeued is requested by a specific priority,
/// instead of leaving the priority decision up to the
/// packet scheduler.
/// This function is called by
/// MAC_OutputQueueDequeuePacketForAPriority() (mac/mac.pc),
/// which itself is called from mac/mac_802_11.pc and other
/// MAC protocol source files.
///
/// This function will assert false if the scheduler cannot
/// return an IP packet for whatever reason.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  index to interface .
/// \param priority  priority of packet.
/// \param msg  Storage for pointer to message
///    with IP packet.
/// \param nextHopAddress  Storage for Packet's next hop address.
/// \param networkType  Type of network packet is using (IP,
///    Link-16, ...)
/// \param posInQueue  Position of packet in Queue.
///    Added as part of IP-MPLS integration
///    Default - DEQUEUE_NEXT_PACKET
///
/// \return TRUE if dequeued successfully, FALSE otherwise.
BOOL NetworkIpOutputQueueDequeuePacketForAPriority(
    Node *node,
    int interfaceIndex,
    QueuePriorityType priority,
    Message **msg,
    NodeAddress *nextHopAddress,
    MacHWAddress *nexthopmacAddr,
    int *networkType,
    int posInQueue = DEQUEUE_NEXT_PACKET);

/// Same as NetworkIpOutputQueueDequeuePacket(), except the
/// packet dequeued is requested by a specific index
/// This function is called by
/// MAC_OutputQueueDequeuePacketForAPriority() (mac/mac.pc),
/// which itself is called from mac/mac_802_11.pc and other
/// MAC protocol source files.
///
/// This function will assert false if the scheduler cannot
/// return an IP packet for whatever reason.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  index to interface .
/// \param msgIndex  index of packet.
/// \param msg  Storage for pointer to message
///    with IP packet.
/// \param nextHopAddress  Storage for Packet's next hop address.
/// \param networkType  Type of network packet is using (IP,
///    Link-16, ...)
///
/// \return TRUE if dequeued successfully, FALSE otherwise.
BOOL NetworkIpOutputQueueDequeuePacketWithIndex(
    Node *node,
    int interfaceIndex,
    int msgIndex,
    Message **msg,
    NodeAddress *nextHopAddress,
    int *networkType);

/// Same as NetworkIpInputQueueDequeuePacket(), except the
/// packet is not actually dequeued.  Note that the message
/// containing the packet is not copied; the contents may
/// (inadvertently or not) be directly modified.
///
/// \param node  Pointer to node.
/// \param incomingInterface  interface to get top packet from.
/// \param msg  Storage for pointer to message
///    with IP packet.
/// \param nextHopAddress  Storage for Packet's next hop addr.
/// \param outgoingInterface  Used to determine where packet should go
///    after passing through the backplane.
/// \param networkType  Type of network packet is using (IP,
///    Link-16, ...)
/// \param priority  Storage for priority
///    of packet.
///
/// \return TRUE if there is a packet, FALSE otherwise.
BOOL NetworkIpInputQueueTopPacket(
    Node *node,
    int incomingInterface,
    Message **msg,
    NodeAddress *nextHopAddress,
    MacHWAddress *nexthopmacAddr,
    int *outgoingInterface,
    int *networkType,
    QueuePriorityType *priority);

/// Same as NetworkIpOutputQueueDequeuePacket(), except the
/// packet is not actually dequeued.  Note that the message
/// containing the packet is not copied; the contents may
/// (inadvertently or not) be directly modified.
///
/// This function is called by MAC_OutputQueueTopPacket()
/// (mac/mac.pc), which itself is called from
/// mac/mac_802_11.pc and other MAC protocol source files.
///
/// This function will assert false if the scheduler cannot
/// return an IP packet for whatever reason.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  index to interface .
/// \param msg  Storage for pointer to message
///    with IP packet.
/// \param nextHopAddress  Storage for Packet's next hop addr.
/// \param networkType  Type of network of the packet
/// \param priority  Storage for priority
///    of packet.
///
/// \return TRUE if there is a packet, FALSE otherwise.
BOOL NetworkIpOutputQueueTopPacket(
    Node *node,
    int interfaceIndex,
    Message **msg,
    NodeAddress *nextHopAddress,
    MacHWAddress *nexthopmacAddr,
    int* networkType,
    QueuePriorityType *priority);

/// Same as NetworkIpOutputQueueDequeuePacket(), except the
/// packet is not actually dequeued.  Note that the message
/// containing the packet is not copied; the contents may
/// (inadvertently or not) be directly modified.
///
/// This function is called by MAC_OutputQueueTopPacket()
/// (mac/mac.pc), which itself is called from
/// mac/mac_802_11.pc and other MAC protocol source files.
///
/// This function will assert false if the scheduler cannot
/// return an IP packet for whatever reason.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  index to interface .
/// \param msgIndex  index to message .
/// \param msg  Storage for pointer to message
///    with IP packet.
/// \param nextHopAddress  Storage for Packet's next hop addr.
/// \param priority  Storage for priority
///    of packet.
///
/// \return TRUE if there is a packet, FALSE otherwise.
BOOL NetworkIpOutputQueuePeekWithIndex(
    Node *node,
    int interfaceIndex,
    int msgIndex,
    Message **msg,
    NodeAddress *nextHopAddress,
    QueuePriorityType *priority);

/// Same as NetworkIpOutputQueueDequeuePacketForAPriority(),
/// except the packet is not actually dequeued.  Note that
/// the message containing the packet is not copied; the
/// contents may (inadvertently or not) be directly
/// modified.
///
/// This function is called by
/// MAC_OutputQueueTopPacketForAPriority() (mac/mac.pc),
/// which itself is called from mac/mac_802_11.pc and other
/// MAC protocol source files.
///
/// This function will assert false if the scheduler cannot
/// return an IP packet for whatever reason.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  index to interface .
/// \param priority  priority of packet
/// \param msg  Storage for pointer to message
///    with IP packet.
/// \param nextHopAddress  Storage for packet's next hop address.
/// \param posInQueue  Position of packet in Queue.
///    Added as part of IP-MPLS integration
///    RETURN     : BOOL : TRUE if there is a packet, FALSE otherwise.
BOOL NetworkIpOutputQueueTopPacketForAPriority(
    Node *node,
    int interfaceIndex,
    QueuePriorityType priority,
    Message **msg,
    NodeAddress *nextHopAddress,
    MacHWAddress *nexthopmacAddr,
    int posInQueue = DEQUEUE_NEXT_PACKET);

/// Calls the packet scheduler for an interface to determine
/// whether the interface's input queue is empty
///
/// \param node  Pointer to node.
/// \param incomingInterface  Index of interface.
///
/// \return TRUE if the scheduler says the interface's
/// input queue is empty.
/// FALSE if the scheduler says the interface's
/// input queue is not empty.
BOOL
NetworkIpInputQueueIsEmpty(Node *node, int incomingInterface);


/// Calls the packet scheduler for an interface to determine
/// whether the interface's output queue is empty.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Index of interface.
///
/// \return TRUE if the scheduler says the interface's output
/// queue is empty.
/// FALSE if the scheduler says the interface's output
/// queue is not empty.
BOOL
NetworkIpOutputQueueIsEmpty(Node *node, int interfaceIndex);

/// Calls the packet scheduler for an interface to determine
/// how many packets are in a queue.  There may be multiple
/// queues on an interface, so the priority of the desired
/// queue is also provided.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Index of interface.
/// \param specificPriorityOnly  Should we only get the number of packets
///    in queue for the specified priority only
///    or for all priorities.
/// \param priority  Priority of queue.
///
/// \return Number of packets in queue.
int
NetworkIpOutputQueueNumberInQueue(
    Node *node,
    int interfaceIndex,
    BOOL specificPriorityOnly,
    QueuePriorityType priority);

/// Drop a packet from the queue.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  index to interface .
/// \param msg  Storage for pointer to message
///    with IP packet.
///
/// \return Next hop of dropped packet.
NodeAddress
NetworkIpOutputQueueDropPacket(
    Node *node,
    int interfaceIndex,
    Message **msg,
    MacHWAddress *nexthopmacAddr);


/// Deletes all packets in the queue going (probably broken),
/// to the specified next hop address.   There is option
/// to return all such packets back to the routing protocols.
/// via the usual mechanism (callback).
///
/// \param node  Pointer to node.
/// \param nextHopAddress  Next hop associated with
///    outbound packets.
/// \param destinationAddress  destination associated with
///    outbound packets.
/// \param returnPacketsToRoutingProtocol  Determine whether or not
///    dropped packets should be
///    returned to the routing
///    protocol for further
///    processing.
void NetworkIpDeleteOutboundPacketsToANode(
    Node *node,
    const NodeAddress nextHopAddress,
    const NodeAddress destinationAddress,
    const BOOL returnPacketsToRoutingProtocol);


/// Get queue number through which a given user priority
/// will be forwarded.
///
/// \param userTos  user TOS.
/// \param numQueues  Number of queues.
///
/// \return Index of the queue.
unsigned GetQueueNumberFromPriority(
    TosType userTos,
    int numQueues);


//----------------------------------------------------------
// Per hop behavior (PHB) routing
//----------------------------------------------------------


/// Returns the priority queue corresponding to the
/// DS/TOS field.
///
/// \param node  Pointer to node.
/// \param tos  TOS field
///
/// \return priority queue
QueuePriorityType
ReturnPriorityForPHB(
    Node *node,
    TosType tos);

//----------------------------------------------------------
// Routing table (forwarding table)
//----------------------------------------------------------

/// Do a lookup on the routing table with a destination IP
/// address to obtain a route (index of an outgoing
/// interface and a next hop Ip address).
///
/// \param node  Pointer to node.
/// \param destinationAddress  Destination IP address.
/// \param interfaceIndex  Storage for index of outgoing
///    interface.
/// \param nextHopAddress  Storage for next hop IP address.
///    If no route can be found,
///    *nextHopAddress will be
///    set to NETWORK_UNREACHABLE.
void NetworkGetInterfaceAndNextHopFromForwardingTable(
    Node *node,
    NodeAddress destinationAddress,
    int *interfaceIndex,
    NodeAddress *nextHopAddress);

/// Do a lookup on the routing table with a destination IP
/// address to obtain a route (index of an outgoing
/// interface and a next hop Ip address).
///
/// \param node  Pointer to node.
/// \param currentInterface  Current interface in use.
/// \param destinationAddress  Destination IP address.
/// \param interfaceIndex  Storage for index of outgoing
///    interface.
/// \param nextHopAddress  Storage for next hop IP address.
///    If no route can be found,
///    *nextHopAddress will be
///    set to NETWORK_UNREACHABLE.
void NetworkGetInterfaceAndNextHopFromForwardingTable(
    Node *node,
    int currentInterface,
    NodeAddress destinationAddress,
    int *interfaceIndex,
    NodeAddress *nextHopAddress);

//-----------------------------------------------------------------------------
// FUNCTION     NetworkGetInterfaceAndNextHopFromForwardingTable()
// PURPOSE      Do a lookup on the routing table with a destination IP
//              address to obtain a route (index of an outgoing
//              interface and a next hop Ip address).
// PARAMETERS   Node *node
//                  Pointer to node.
//              NodeAddress destinationAddress
//                  Destination IP address.
//              int *interfaceIndex
//                  Storage for index of outgoing interface.
//              NodeAddress *nextHopAddress
//                  Storage for next hop IP address.
//                  If no route can be found, *nextHopAddress will be
//                  set to NETWORK_UNREACHABLE.
//              BOOL *routeType true if forwarding table has entry for
//                    particular destination false if for network.
// RETURN       None.
//-----------------------------------------------------------------------------

void NetworkGetInterfaceAndNextHopFromForwardingTable(
    Node *node,
    NodeAddress destinationAddress,
    int *interfaceIndex,
    NodeAddress *nextHopAddress,
    BOOL *routeType);


/// Do a lookup on the routing table with a destination IP
/// address to obtain a route (index of an outgoing
/// interface and a next hop Ip address).
///
/// \param node  Pointer to node.
/// \param destinationAddress  Destination IP address.
/// \param interfaceIndex  Storage for index of outgoing
///    interface.
/// \param nextHopAddress  Storage for next hop IP address.
///    If no route can be found,
///    *nextHopAddress will be
///    set to NETWORK_UNREACHABLE.
/// \param testType  Same protocol's routes if true
///    different protocol's routes else
/// \param type  routing protocol type.
void NetworkGetInterfaceAndNextHopFromForwardingTable
(
    Node *node,
    NodeAddress destinationAddress,
    int *interfaceIndex,
    NodeAddress *nextHopAddress,
    BOOL testType,
    NetworkRoutingProtocolType type);

/// Do a lookup on the routing table with a destination IP
/// address to obtain a route (index of an outgoing
/// interface and a next hop Ip address).
///
/// \param node  Pointer to node.
/// \param operatingInterface  interface currently being
///    operated on. Routes will
///    only be searched for that
///    have an outgoing interface
///    that matches the
///    operating interface.
/// \param destinationAddress  Destination IP address.
/// \param interfaceIndex  Storage for index of outgoing
///    interface.
/// \param nextHopAddress  Storage for next hop IP address.
///    If no route can be found,
///    *nextHopAddress will be
///    set to NETWORK_UNREACHABLE.
/// \param testType  Same protocol's routes if true
///    different protocol's routes else
/// \param type  routing protocol type.
void NetworkGetInterfaceAndNextHopFromForwardingTable
(
    Node *node,
    int operatingInterface,
    NodeAddress destinationAddress,
    int *interfaceIndex,
    NodeAddress *nextHopAddress,
    BOOL testType,
    NetworkRoutingProtocolType type);

//-----------------------------------------------------------------------------
// FUNCTION     NetworkGetInterfaceAndNextHopFromForwardingTable()
// PURPOSE      Do a lookup on the routing table with a destination IP
//              address to obtain a route (index of an outgoing
//              interface and a next hop Ip address).
// PARAMETERS   Node *node
//                  Pointer to node.
//              int operatingInterface
//                  interface currently being
//                  operated on. Routes will only be searched for that
//                  have an outgoing interface that matches the
//                  operating interface.
//              NodeAddress destinationAddress
//                  Destination IP address.
//              int *interfaceIndex
//                  Storage for index of outgoing interface.
//              NodeAddress *nextHopAddress
//                  Storage for next hop IP address.
//                  If no route can be found, *nextHopAddress will be
//                  set to NETWORK_UNREACHABLE.
//              BOOL testType
//                  same protocol's routes if true
//                  different protocol's routes else
//              NetworkRoutingProtocolType type
//                  routing protocol type
// RETURN       None.
//-----------------------------------------------------------------------------
void NetworkGetInterfaceAndNextHopFromForwardingTable
(
    Node *node,
    NodeAddress destinationAddress,
    int *interfaceIndex,
    NodeAddress *nextHopAddress,
    BOOL testType,
    NetworkRoutingProtocolType type,
    BOOL* routeType);


/// This function looks at the network address of each of a
/// node's network interfaces.  When nextHopAddress is
/// matched to a network, the interface index corresponding
/// to the network is returned.
/// (used by NetworkUpdateForwardingTable() and ospfv2.pc)
///
/// \param node  Pointer to node.
/// \param nextHopAddress  Destination IP address.
///
/// \return Index of outgoing interface, if nextHopAddress
/// is on a directly connected network. -1, otherwise
int
NetworkIpGetInterfaceIndexForNextHop(
    Node *node,
    NodeAddress nextHopAddress);


/// Get interface for the destination address.
///
/// \param node  Pointer to node.
/// \param destAddress  Destination IP address.
///
/// \return interface index associated with destination.
int
NetworkGetInterfaceIndexForDestAddress(
    Node *node,
    NodeAddress destAddress);

/// Get the administrative distance of a routing protocol.
/// These values don't quite match those recommended by
/// Cisco.
///
/// \param node  Pointer to node.
/// \param type  Type value of routing protocol.
///
/// \return The administrative
/// distance of the routing protocol.
NetworkRoutingAdminDistanceType
NetworkRoutingGetAdminDistance(
    Node *node,
    NetworkRoutingProtocolType type);

/// Initialize the IP fowarding table, allocate enough
/// memory for number of rows.
///
/// \param node  Pointer to node.
void
NetworkInitForwardingTable(Node *node);


/// Update or add entry to IP routing table.  Search the
/// routing table for an entry with an exact match for
/// destAddress, destAddressMask, and routing protocol.
/// Update this entry with the specified nextHopAddress
/// (the outgoing interface is automatically determined
/// from the nextHopAddress -- see code).  If no matching
/// entry found, then add a new route.
///
/// \param node  Pointer to node.
/// \param destAddress  IP address of destination
///    network or host.
/// \param destAddressMask  Netmask.
/// \param nextHopAddress  Next hop IP address.
/// \param outgoingInterfaceIndex  outgoing interface.
/// \param cost  Cost metric associated with
///    the route.
/// \param type  type value of
///    routing protocol.
void
NetworkUpdateForwardingTable(
    Node *node,
    NodeAddress destAddress,
    NodeAddress destAddressMask,
    NodeAddress nextHopAddress,
    int outgoingInterfaceIndex,
    int cost,
    NetworkRoutingProtocolType type);


/// Remove single entries in the routing table
///
/// \param node  Pointer to node.
/// \param destAddress  IP address of destination
///    network or host.
/// \param destAddressMask  Netmask.
/// \param nextHopAddress  Next hop IP address.
/// \param outgoingInterfaceIndex  outgoing interface.
void
NetworkRemoveForwardingTableEntry(
    Node *node,
    NodeAddress destAddress,
    NodeAddress destAddressMask,
    NodeAddress nextHopAddress,
    int outgoingInterfaceIndex);

/// Remove entries in the routing table corresponding to a
/// given routing protocol.
///
/// \param node  Pointer to node.
/// \param type  Type of routing protocol whose
///    entries are to be  removed.
void
NetworkEmptyForwardingTable(
    Node *node,
    NetworkRoutingProtocolType type);


/// Display all entries in node's routing table.
///
/// \param node  Pointer to node.
void
NetworkPrintForwardingTable(Node *node);

/// Get the cost metric for a destination from the
/// forwarding table.
///
/// \param node  Pointer to node.
/// \param destAddress  destination to get cost metric from.
///
/// \return Cost metric associated with destination.
int
NetworkGetMetricForDestAddress(
    Node *node,
    NodeAddress destAddress,
    NetworkRoutingAdminDistanceType *dist = NULL);

/// Set a callback fuction when a route changes from forwarding
/// table.
///
/// \param node  Pointer to node.
/// \param routeUpdateFunctionPtr  Route update
///    callback function to set.
void
NetworkIpSetRouteUpdateEventFunction(
    Node *node,
    NetworkRouteUpdateEventType routeUpdateFunctionPtr);


/// Print packet headers when packet tracing is enabled.
///
/// \param node  Pointer to node.
///
/// \return Route update callback
/// function to set.
NetworkRouteUpdateEventType
NetworkIpGetRouteUpdateEventFunction(
    Node *node);

//----------------------------------------------------------
// Interface IP addresses
//----------------------------------------------------------

/// Get the interface address on this interface
///
/// \param node  Pointer to the node
/// \param interfaceIndex  Number of interface
///
/// \return IP address associated with the interface
NodeAddress
NetworkIpGetInterfaceAddress(
    Node *node,
    int interfaceIndex);

NodeAddress
NetworkIpGetLinkLayerAddress(Node* node, int interfaceIndex);

int
NetworkIpGetInterfaceIndexFromLinkAddress(Node* node,
        NodeAddress ownMacAddr);


/// To get the interface name associated with the interface
///
/// \param node  Pointer to the node
/// \param interfaceIndex  Number of interface
///
/// \return interface name
char*
NetworkIpGetInterfaceName(
    Node *node,
    int interfaceIndex);

/// To get network address associated with interface
///
/// \param node  Pointer to the node
/// \param interfaceIndex  Number of interface
///
/// \return network address associated with interface
NodeAddress
NetworkIpGetInterfaceNetworkAddress(
    Node *node,
    int interfaceIndex);

/// To retrieve subnet mask of the node interface
///
/// \param node  Pointer to the node
/// \param interfaceIndex  Number of interface
///
/// \return subnet mask of the specified interface
NodeAddress
NetworkIpGetInterfaceSubnetMask(
    Node *node,
    int interfaceIndex);

/// Get the number of host bits on this interface
///
/// \param node  Pointer to the node
/// \param interfaceIndex  Number of interface
///
/// \return Number of host bits on the specified interface
int
NetworkIpGetInterfaceNumHostBits(
    Node *node,
    int interfaceIndex);

/// Get broadcast address on this interface
///
/// \param node  Pointer to the node
/// \param interfaceIndex  Number of interface
///
/// \return Broadcast address of specified interface
NodeAddress
NetworkIpGetInterfaceBroadcastAddress(
    Node *node,
    int interfaceIndex);

/// Checks whether IP packet's destination address is broadcast
///
/// \param node  Pointer to the node
/// \param destAddress  IP packet's destination IP address.
/// \param outgoingInterface  Outgoing interface index.
/// \param outgoingBroadcastAddress  Broadcast address
///    of Outgoing interface
///    RETURNS    :: BOOL : Returns true if destination is broadcast address

BOOL IsOutgoingBroadcast(Node *node,
                         NodeAddress destAddress,
                         int *outgoingInterface,
                         NodeAddress *outgoingBroadcastAddress);

//----------------------------------------------------------
// Miscellaneous
//----------------------------------------------------------

/// In turn calls IsMyPacket()
///
/// \param node  Pointer to the node
/// \param ipAddress  An IP packet's destination IP address.
///
/// \return Returns if it belongs to it.
BOOL
NetworkIpIsMyIP(Node *node, NodeAddress ipAddress);

//-----------------------------------------------------------------------------
// FUNCTION     NetworkIpIsLoopbackEnabled()
// PURPOSE      Returns the IP loopback status
// PARAMETERS   Node *node
//                  Pointer to node.
//
// RETURN       BOOL.
//-----------------------------------------------------------------------------

BOOL
NetworkIpIsLoopbackEnabled(Node* node);

//----------------------------------------------------------
// Debugging
//----------------------------------------------------------

/// Prints out the IP configuration error
///
/// \param node  Pointer to the node
/// \param parameterName  Error message to print
/// \param interfaceIndex  interface number
void
NetworkIpConfigurationError(
    Node *node,
    const char parameterName[],
    int interfaceIndex);

/// To print the IP header
///
/// \param msg  Pointer to Message
void
NetworkPrintIpHeader(Message *msg);

//----------------------------------------------------------
// Statistics
//----------------------------------------------------------

// */*
// API        :: NetworkIpRunTimeStat
// LAYER      :: Network
// PURPOSE    :: Print IP runtime statistics
// PARAMETERS ::
// + node      : Node* : pointer to node
// RETURN     :: void :
void
NetworkIpRunTimeStat(Node *node);

//----------------------------------------------------------
// Multicast
//----------------------------------------------------------

/// Add a specified node to a multicast group
///
/// \param node  Pointer to the node
/// \param groupAddress  address of multicast group
void
NetworkIpAddToMulticastGroupList(
    Node *node,
    NodeAddress groupAddress);

/// To remove specified node from a multicast group
///
/// \param node  Pointer to the node
/// \param groupAddress  address of multicast group
void
NetworkIpRemoveFromMulticastGroupList(
    Node *node,
    NodeAddress groupAddress);


/// To print the multicast grouplist
///
/// \param node  Pointer to the node
void
NetworkIpPrintMulticastGroupList(Node *node);

/// check if a node is part of specified multicast group
///
/// \param node  Pointer to the node
/// \param groupAddress  group to check if node is part of
///    multicast group
BOOL
NetworkIpIsPartOfMulticastGroup(Node *node,
                                NodeAddress groupAddress,
                                NodeAddress srcAddress = 0);

// FUNCTION   NetworkIpIsMyMulticastPacket
// PURPOSE    Check if I am either the receiver of the multicast packet
//            or is a forwarding router for it
// PARAMETERS node - this node.
//            srdAddr - source address who generates this packet
//            dstAddr - shall be the multicast group address
//            prevAddr - shall be the upstream node
//            incomingInterface - incoming interface index
// RETURN     TRUE if node is able to handle this packet, FALSE otherwise.
BOOL NetworkIpIsMyMulticastPacket(Node *node,
                                  NodeAddress srcAddr,
                                  NodeAddress dstAddr,
                                  NodeAddress prevAddr,
                                  int incomingInterface);

/// To join a multicast group
///
/// \param node  Pointer to the node
/// \param mcastAddr  multicast group address
/// \param delay  delay after which to join
void
NetworkIpJoinMulticastGroup(
    Node* node,
    NodeAddress mcastAddr,
    clocktype delay);

/// To join a multicast group
///
/// \param node  Pointer to the node
/// \param interfaceId  on which interface to join the group
/// \param mcastAddr  multicast group address
/// \param delay  delay after which to join
/// \param filterMode  filter mode of the interface (specific to IGMP version 3)
/// \param sourceList  list of sources from where multicast traffic
///    is to be allowed or to be stopped (specific to
///    IGMP version 3).
void
NetworkIpJoinMulticastGroup(
    Node* node,
    Int32 interfaceId,
    NodeAddress mcastAddr,
    clocktype delay,
    char filterMode[],
    vector<NodeAddress> sourceList);

/// To join a multicast group
///
/// \param node  Pointer to the node
/// \param mcastAddr  multicast group address
/// \param delay  delay after which to join
/// \param filterMode  filter mode of the interface (specific to IGMP version 3)
/// \param sourceList  list of sources from where multicast traffic
///    is to be allowed or to be stopped (specific to
///    IGMP version 3)
void
NetworkIpJoinMulticastGroup(
    Node* node,
    NodeAddress mcastAddr,
    clocktype delay,
    char filterMode[],
    vector<NodeAddress> sourceList);

/// To leave a multicast group
///
/// \param node  Pointer to the node
/// \param mcastAddr  multicast group address
/// \param delay  delay after which to leave
void
NetworkIpLeaveMulticastGroup(
    Node* node,
    NodeAddress mcastAddr,
    clocktype delay);

/// To leave a multicast group
///
/// \param node  Pointer to the node
///    +interfaceId: on which interface it was the member of the group
/// \param mcastAddr  multicast group address
/// \param delay  delay after which to leave
void
NetworkIpLeaveMulticastGroup(
    Node* node,
    Int32 interfaceId,
    NodeAddress mcastAddr,
    clocktype delay);

/// To set a multicast timer to join or leave multicast groups
///
/// \param node  Pointer to the node
/// \param eventType  the event type
/// \param mcastAddr  multicast group address
/// \param delay  delay after which to leave
void
NetworkIpSetMulticastTimer(
    Node* node,
    Int32 eventType,
    NodeAddress mcastAddr,
    clocktype delay);

/// Assign a multicast routing protocol to an interface
///
/// \param node  Pointer to the node
/// \param multicastRoutingProtocol  multicast routing protocol
/// \param interfaceIndex  interface number
void
NetworkIpSetMulticastRoutingProtocol(
    Node *node,
    void *multicastRoutingProtocol,
    int interfaceIndex);

/// To get the Multicast Routing Protocol structure
///
/// \param node  Pointer to the node
/// \param routingProtocolType  routing protocol name
void *
NetworkIpGetMulticastRoutingProtocol(
    Node *node,
    NetworkRoutingProtocolType routingProtocolType);


/// Assign a multicast protocol type to an interface
///
/// \param node  Pointer to this node
/// \param multicastProtocolType  routing protocol
/// \param interfaceIndex  interface number of the node
void
NetworkIpAddMulticastRoutingProtocolType(
    Node *node,
    NetworkRoutingProtocolType multicastProtocolType,
    int interfaceIndex);


/// Set a multicast router function to an interface
///
/// \param node  Pointer to this node
/// \param routerFunctionPtr  router Func pointer
/// \param interfaceIndex  interface number of the node
void
NetworkIpSetMulticastRouterFunction(
    Node *node,
    MulticastRouterFunctionType routerFunctionPtr,
    int interfaceIndex);


/// Get the multicast router function for an interface
///
/// \param node  Pointer to this node
/// \param interfaceIndex  interface number of the node
///
/// \return Multicast router function
/// on this interface.
MulticastRouterFunctionType
NetworkIpGetMulticastRouterFunction(
    Node *node,
    int interfaceIndex);

/// Assign multicast routing protocol structure and router
/// function to an interface.  We are only allocating
/// the multicast routing protocol structure and router function
/// once by using pointers to the original structures.
///
/// \param node  this node
/// \param routingProtocolType  multicast routing
///    protocol.
/// \param interfaceIndex  interface index.
void
NetworkIpUpdateMulticastRoutingProtocolAndRouterFunction(
    Node *node,
    NetworkRoutingProtocolType routingProtocolType,
    int interfaceIndex);


/// Assign unicast routing protocol structure and router
/// function to an interface.  We are only allocating
/// the unicast routing protocol structure and router function
/// once by using pointers to the original structures.
///
/// \param node  this node
/// \param routingProtocolType  unicast routing
///    protocol to add.
/// \param interfaceIndex  interface associated with unicast protocol.
void
NetworkIpUpdateUnicastRoutingProtocolAndRouterFunction(
    Node *node,
    NetworkRoutingProtocolType routingProtocolType,
    int interfaceIndex,
    NetworkType networkType = NETWORK_IPV4);

/// Get the interface index from an IP address.
///
/// \param node  this node
/// \param address  address to determine interface index for
///
/// \return interface index associated with specified
/// address.
int
NetworkIpGetInterfaceIndexFromAddress(
    Node *node,
    NodeAddress address);

/// Get the interface index from an IP subnet address.
///
/// \param node  this node
/// \param address  subnet address to determine interface
///    index for.
///
/// \return interface index associated with specified
/// subnet address.
int
NetworkIpGetInterfaceIndexFromSubnetAddress(
    Node *node,
    NodeAddress address);



/// Check if an address is a multicast address.
///
/// \param node  this node
/// \param address  address to determine if multicast address.
///
/// \return TRUE if address is multicast address,
/// FALSE, otherwise.
BOOL
NetworkIpIsMulticastAddress(
    Node *node,
    NodeAddress address);

/// initialize the multicast fowarding table, allocate enough
/// memory for number of rows, used by ip
///
/// \param node  this node
void
NetworkInitMulticastForwardingTable(Node *node);

/// empty out all the entries in the multicast forwarding table.
/// basically set the size of table back to 0.
///
/// \param node  this node
void
NetworkEmptyMulticastForwardingTable(Node *node);


/// get the interface Id node that lead to the
/// (source, multicast group) pair.
///
/// \param node  its own node
/// \param sourceAddress  multicast source address
///    to foward to..
/// \param groupAddress  multicast group
///
/// \return interface Id from node to (source, multicast
/// group), or NETWORK_UNREACHABLE (no such entry
/// is found)
LinkedList *
NetworkGetOutgoingInterfaceFromMulticastForwardingTable(
    Node *node,
    NodeAddress sourceAddress,
    NodeAddress groupAddress);

/// update entry with(sourceAddress,multicastGroupAddress) pair.
/// search for the row with(sourceAddress,multicastGroupAddress)
/// and update its interface.
///
/// \param node  its own node
/// \param sourceAddress  multicast source
/// \param multicastGroupAddress  multicast group
/// \param interfaceIndex  interface to use for
///    (sourceAddress, multicastGroupAddress)
void
NetworkUpdateMulticastForwardingTable(
    Node *node,
    NodeAddress sourceAddress,
    NodeAddress multicastGroupAddress,
    int interfaceIndex);

/// display all entries in multicast forwarding table of the
/// node.
///
/// \param node  this node
void
NetworkPrintMulticastForwardingTable(Node *node);

/// Print mulitcast outgoing interfaces.
///
/// \param node  this node
/// \param list  list of outgoing interfaces.
void
NetworkPrintMulticastOutgoingInterface(Node *node, LinkedList *list);


/// Determine if interface is in multicast outgoing interface
/// list.
///
/// \param node  this node
/// \param list  list of outgoing interfaces.
/// \param interfaceIndex  interface to determine if in outgoing
///    interface list.
///
/// \return TRUE if interface is in multicast outgoing interface
/// list, FALSE otherwise.
BOOL
NetworkInMulticastOutgoingInterface(
    Node *node,
    LinkedList *list,
    int interfaceIndex);

/// Print packet trace information in XML format.
///
/// \param node  this node
/// \param msg  Packet to print headers from.
void NetworkIpPrintTraceXML(Node *node, Message *msg);

/*
 * FUNCTION:   NetworkIpCheckIpAddressIsInSameSubnet()
 * PURPOSE:    This function check this argument ip address and ip address
 *             of this specified interfaceIndex are in same subnet.
 * RETURN:     BOOL
 * ASSUMPTION: None
 * PARAMETERS: node,              node in which this interfaceIndex belong.
 *             interfaceIndex,    interface index.
 *             ipAddress,         ip address.
 */

BOOL NetworkIpCheckIpAddressIsInSameSubnet(Node *node,
                                           int interfaceIndex,
                                           NodeAddress ipAddress);



void //inline//
NetworkIpSendOnBackplane(Node *node,
                         Message *msg,
                         int incomingInterface,
                         int outgoingInterface,
                         NodeAddress hopAddr);

void
NetworkIpUseBackplaneIfPossible(Node *node,
                                int incomingInterface);

BOOL
NetworkIpCheckApplicationDataPacket(Node* node,
                                    Message* msg);
void
NetworkIpReceiveFromBackplane(Node *node, Message *msg);

// FUNCTION     RoutePacketAndSendToMac()
// PURPOSE      "Route" a packet by determining the next hop IP address
//              and the outgoing interface.  It is assumed that the IP
//              packet already has an IP header at this point.  Figure
//              out the next hop in the route and send the packet.
//              First the "routing function" is checked and if that fails
//              the default source route or lookup table route is used.
//              [needs updating]
// PARAMETERS   Node *node
//                  Pointer to node.
//              Message *msg
//                  Pointer to message with IP packet.
//              int incomingInterface
//                  Index of interface on which packet arrived.
//                  [This is different if the packet originated from
//                  the network or transport layers.  This value is only
//                  for multicast packets, currently.]
//              int outgoingInterface
//                  Used only when the application specifies a specific
//                  interface to use to transmit packet.
//-----------------------------------------------------------------------------

void //inline//
RoutePacketAndSendToMac(Node *node,
                        Message *msg,
                        int incomingInterface,
                        int outgoingInterface,
                        NodeAddress previousHopAddress);

void //inline//
DeliverPacket(Node *node, Message *msg,
                int interfaceIndex, NodeAddress previousHopAddress);
/// Tries to route and send the packet using the node's
/// forwarding table.
///
/// \param node  this node
/// \param msg  Pointer to message with IP packet.
/// \param incomingInterface  incoming interface of packet
///
void RouteThePacketUsingLookupTable(Node *node,
                                    Message *msg,
                                    int incomingInterface);

/// Returns the network ip fragmentation unit.
///
/// \param node  this node
/// \param interfaceIndex  interface of node

inline int GetNetworkIPFragUnit(Node* node, int interfaceIndex)
{
    NetworkDataIp *ip = (NetworkDataIp *) node->networkData.networkVar;
    if (LlcIsEnabled(node, interfaceIndex))
    {
        return (ip->interfaceInfo[interfaceIndex]->ipFragUnit
                                            - sizeof(LlcHeader));
    }
    else
    {
        return (ip->interfaceInfo[interfaceIndex]->ipFragUnit);
    }
}


#if 0

//----------------------------------------------------------
// Fragmentation (disabled)
//----------------------------------------------------------


/// Starting position of this fragment in actual packet.
#define FragmentOffset(ipHeader) (((ipHeader)->ip_fragment_offset) * 8)


/// To set offset of fragment.
#define SetFragmentOffset(ipHeader, offset) \
    ERROR_Assert(((offset) % 8 == 0) && ((offset) < IP_MAXPACKET)); \
                 (ipHeader)->ip_fragment_offset = ((offset)/8, \
                 "Invalid fragment offset");


/// Max time data can stored in assembly buffer
#define NETWORK_IP_REASS_BUFF_TIMER (15 * SECOND)


/// Max size of fragment allowed.
#define MAX_IP_FRAGMENTS_SIMPLE_CASE    64


/// Size of reassemble buffer
#define SMALL_REASSEMBLY_BUFFER_SIZE    2048

/// Multiplier used for reassemble buffer expansion
#define REASSEMBLY_BUFFER_EXPANSION_MULTIPLIER  8


/// Structure of reassembly buffer
typedef
struct
{
    Message *packetUnderConstruction;
    int sizeLimit;
    clocktype expirationDate;
    unsigned short totalPacketLength;
    unsigned short fragmentationSize;
    unsigned char fragmentIsHereBitTable[MAX_IP_FRAGMENTS_SIMPLE_CASE / 8];
    BOOL endFragmentHasArrived;
    unsigned short endFragmentOffset;
}IpReassemblyBufferType;


/// Structure of reassembly buffer cell listing
typedef
struct IpReassemblyBufferListCellStruct
{
    struct IpReassemblyBufferListCellStruct *nextPtr;
    IpReassemblyBufferType reassemblyBuffer;
}
IpReassemblyBufferListCellType;


/// Structure of reassembly buffer list
typedef
struct
{
    IpReassemblyBufferListCellType *firstPtr;
    IpReassemblyBufferListCellType *freeListPtr;
}
IpReassemblyBufferListType;

//----------------------------------------------------------
// User functions (disabled)
//----------------------------------------------------------

/// Initialization of user protocol(disabled)
///
/// \param node  this node
/// \param nodeInput  Provides access to
///    configuration file.
/// \param routingProtocolString  routing protocol
/// \param routingProtocolType  routing protocol
///    type
/// \param routingProtocolData  Access to routing protocol data
void
NetworkIpUserProtocolInit(
    Node *node,
    const NodeInput *nodeInput,
    const char *routingProtocolString,
    NetworkRoutingProtocolType *routingProtocolType,
    void **routingProtocolData);

/// Event handler function of user protocol(disabled)
///
/// \param node  The node that is handling the event.
/// \param msg  the event that is being handled
void
NetworkIpUserHandleProtocolEvent(
    Node *node,
    Message *msg);

/// Process a user protocol generated control packet(disabled)
///
/// \param node  this node
/// \param msg  message that is being received.
/// \param ipProtocol  ip protocol
/// \param sourceAddress  source address
/// \param destinationAddress  destination address
/// \param ttl  time to live
void
NetworkIpUserHandleProtocolPacket(
    Node *node,
    Message *msg,
    unsigned char ipProtocol,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    int ttl);

/// Finalization of user protocol(disabled)
///
/// \param node  this node
/// \param userProtocolNumber  protocol number
void
NetworkIpUserProtocolFinalize(
    Node *node,
    int userProtocolNumber);


#endif // 0

/// Routing packet received at ATM node
///
/// \param node  this node
/// \param destAddr  destination Address
/// \param outIntf  this node
/// \param nextHop  nextHop address
void
Atm_RouteThePacketUsingLookupTable(Node* node,
                                   NodeAddress destAddr,
                                   int* outIntf,
                                   NodeAddress* nextHop);

// Fragmentation code.

int
IpFragmentPacket(
    Node* node,
    Message* msg,
    int mtu,
    ipFragmetedMsg** fragmentHead,
    BOOL fragmentForMpls);

Message*
IpFragmentInput(
    Node* node,
    Message* msg,
    int interfaceId,
    BOOL* isReassembled);

Message*
IpFragementReassamble(
    Node* node,
    Message* msg,
    IpFragQueue* fp,
    int interfaceId);
BOOL NetworkIpOutputQueuePeekWithIndex(
    Node *node,
    int interfaceIndex,
    int msgIndex,
    Message **msg,
    NodeAddress *nextHopAddress,
    MacHWAddress *nextHopMacAddr,
    QueuePriorityType *priority);

BOOL NetworkIpOutputQueueDequeuePacketWithIndex(
    Node *node,
    int interfaceIndex,
    int msgIndex,
    Message **msg,
    NodeAddress *nextHopAddress,
    MacHWAddress *nextHopMacAddr,
    int *networkType);

/// Tries to route the multicast packet using the
/// multicast forwarding table.
///
/// \param node  this node
/// \param msg  Pointer to Message
/// \param incomingInterface  Incomming Interface
///
void
RouteThePacketUsingMulticastForwardingTable(
    Node* node,
    Message* msg,
    int incomingInterface,
    NetworkType netType);

//-----------------------------------------------------------------------------
// FUNCTION     NetworkIpOutputQueueDequeuePacket()
// PURPOSE      Calls the packet scheduler for an interface to retrieve
//              an IP packet from a queue associated with the interface.
//              The dequeued packet, since it's already been routed,
//              has an associated next-hop IP address.  The packet's
//              priority value is also returned.
//              Addded function for IP+MPLS
// PARAMETERS   Node *node
//                  Pointer to node.
//              int interfaceIndex
//                  Index of interface.
//              Message **msg
//                  Storage for pointer to message with IP packet.
//              NodeAddress *nextHopAddress
//                  Storage for packet's next hop address.
//              QueuePriorityType *userPriority
//                  Storage for user priority of packet.
//              posInQueue
//                  Position of packet in Queue.
//                  Added as part of IP-MPLS integration
// RETURN       TRUE if dequeued successfully, FALSE otherwise.
//
// NOTES        This function is called by
//              MAC_OutputQueueDequeuePacket() (mac/mac.pc), which itself
//              is called from mac/mac_802_11.pc and other MAC protocol
//              source files.
//
//              This function will assert false if the scheduler cannot
//              return an IP packet for whatever reason.
//-----------------------------------------------------------------------------
BOOL NetworkIpOutputQueueDequeuePacket(
    Node *node,
    int interfaceIndex,
    Message **msg,
    NodeAddress *nextHopAddress,
    MacHWAddress *nexthopmacAddr,
    int *networkType,
    QueuePriorityType *userPriority,
    int posInQueue);


//-----------------------------------------------------------------------------
// FUNCTION     NetworkIpQueueTopPacket()
// PURPOSE      Same as NetworkIpQueueDequeuePacket(), except the
//              packet is not actually dequeued.  Note that the message
//              containing the packet is not copied; the contents may
//              (inadvertently or not) be directly modified.
// PARAMETERS   Node *node
//                  Pointer to node.
//              SchedulerType *scheduler
//                  queue to get top packet from.
//              Message **msg
//                  Storage for pointer to message with IP packet.
//              NodeAddress *nextHopAddress
//                  Storage for packet's next hop address.
//              int *outgoingInterface
//                  Used to determine where packet should go after passing
//                  through the backplane.
//              int *networkType
//                  Whether packet is associated with an IP network, Link-16
//                  nework, etc...
//              QueuePriorityType *priority
//                  Storage for priority of packet.
//              posInQueue
//                  Position of packet in Queue.
//                  Added as part of IP-MPLS integration
// RETURN       TRUE if there is a packet, FALSE otherwise.
//-----------------------------------------------------------------------------

BOOL NetworkIpQueueTopPacket(
    Node *node,
    Scheduler *scheduler,
    Message **msg,
    NodeAddress *nextHopAddress,
    MacHWAddress *nexthopmacAddr,
    int *outgoingInterface,
    int *networkType,
    QueuePriorityType *priority,
    int posInQueue = DEQUEUE_NEXT_PACKET);

//-----------------------------------------------------------------------------
// FUNCTION     NetworkIpOutputQueueTopPacket()
// PURPOSE      Same as NetworkIpOutputQueueDequeuePacket(), except the
//              packet is not actually dequeued.  Note that the message
//              containing the packet is not copied; the contents may
//              (inadvertently or not) be directly modified.
// PARAMETERS   Node *node
//                  Pointer to node.
//              int interfaceIndex
//                  Index of interface.
//              Message **msg
//                  Storage for pointer to message with IP packet.
//              NodeAddress *nextHopAddress
//                  Storage for packet's next hop address.
//              int *networkType
//                  Whether packet is associated with an IP network, Link-16
//                  nework, etc...
//              QueuePriorityType *priority
//                  Storage for priority of packet.
//              posInQueue
//                  Position of packet in Queue.
//                  Added as part of IP-MPLS integration
// RETURN       TRUE if there is a packet, FALSE otherwise.
//
// NOTES        This function is called by MAC_OutputQueueTopPacket()
//              (mac/mac.pc), which itself is called from
//              mac/mac_802_11.pc and other MAC protocol source files.
//
//              This function will assert false if the scheduler cannot
//              return an IP packet for whatever reason.
//-----------------------------------------------------------------------------

BOOL NetworkIpOutputQueueTopPacket(
    Node *node,
    int interfaceIndex,
    Message **msg,
    NodeAddress *nextHopAddress,
    MacHWAddress* nexthopmacAddr,
    int* networkType,
    QueuePriorityType *priority,
    int posInQueue);

//-----------------------------------------------------------------------------
// FUNCTION     NetworkIpQueueInsert()
// PURPOSE      Calls the packet scheduler for an interface to retrieve
//              an IP packet from a queue associated with the interface.
//              The dequeued packet, since it's already been routed,
//              has an associated next-hop IP address.  The packet's
//              priority value is also returned.
// PARAMETERS   Node *node
//                  Pointer to node.
//              int incomingInterface
//                  interface of input queue.
//              Message *msg
//                  Pointer to message with IP packet.
//              NodeAddress nextHopAddress
//                  Packet's next hop address.
//              NodeAddress destinationAddress
//                  Packet's destination address.
//              int outgoingInterface
//                  Used to determine where packet should go after passing
//                  through the backplane.
//              int networkType
//                  Type of network packet is using (IP, Link-16, ...)
//              BOOL *queueIsFull
//                  Storage for boolean indicator.
//                  If TRUE, packet was not queued because scheduler
//                  reported queue was (or queues were) full.
//              int incomingInterface
//                  Incoming interface, Default argument used by backplane.
//              BOOL isOutputQueue
//                      Output Queue
// RETURN       None.
//--------------------------------------------------------------------------
void
NetworkIpQueueInsert(
    Node *node,
    Scheduler *scheduler,
    Message *msg,
    NodeAddress nextHopAddress,
    NodeAddress destinationAddress,
    int outgoingInterface,
    int networkType,
    BOOL *queueIsFull,
    int incomingInterface = ANY_INTERFACE,
    BOOL isOutputQueue = FALSE);


#ifdef ADDON_NGCNMS
void NetworkIpReset(Node* node, int interfaceIndex);
#endif

/// Initialization function for network layer.
/// Initializes IP.
///
/// \param node  Pointer to node.
/// \param nodeInput  Pointer to node input.
void
IpRoutingInit(Node *node,
              const NodeInput *nodeInput);

/// getting the bandwidth information
///
/// \param node  the node who's bandwidth is needed.
/// \param interfaceIndex  interface Index.
///
/// \return inverted bandwidth
/// ASSUMPTION   :     Bandwidth read from interface is in from of bps unit.
/// To invert the bandwidth we use the equation
/// 10000000 / bandwidth. Where bandwidth is in Kbps unit.
Int64 NetworkIpGetBandwidth(Node* node, int interfaceIndex);

/// getting the propagation delay information
///
/// \param node  the node who's bandwidth is needed.
/// \param interfaceIndex  interface Index.
///
/// \return propagation delay
/// ASSUMPTION           : Array is exactly 3-byte long.
clocktype NetworkIpGetPropDelay(Node* node, int interfaceIndex);

/// To check the interface is enabled or not?
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
BOOL NetworkIpInterfaceIsEnabled(Node* node, int interfaceIndex);

/// Determines if an interface is a wired interface.
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
///
/// \return BOOL
BOOL
NetworkIpIsWiredNetwork(Node *node, int interfaceIndex);

/// Determines if an interface is a point-to-point.
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
///
/// \return BOOL
BOOL
NetworkIpIsPointToPointNetwork(Node *node, int interfaceIndex);

/// To check if IPV4 Multicast is enabled on interface?
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
BOOL IsIPV4MulticastEnabledOnInterface(Node* node,
                                       int interfaceIndex);

/// To check if IPV4 Routing is enabled on interface?
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
BOOL IsIPV4RoutingEnabledOnInterface(Node* node,
                                     int interfaceIndex);

/// Get Network Protocol Type for the node
///
/// \param node  node structure pointer.
/// \param nodeId  node id.
NetworkProtocolType NetworkIpGetNetworkProtocolType(
    Node* node,
    NodeAddress nodeId);

/// Resolve the NetworkType from source and destination node id's.
///
/// \param node  Pointer to the node.
/// \param sourceNodeId  Source node id.
/// \param destNodeId  Destination node id.
NetworkType
ResolveNetworkTypeFromSrcAndDestNodeId(
    Node* node,
    NodeId sourceNodeId,
    NodeId destNodeId);

// FUNCTION            :: NetworkIpIsUnnumberedInterface
// LAYER               :: Network
// PURPOSE             :: checking for unnumbered Interface.
// PARAMETERS          ::
// + node              :: Node*   : Pointer to node structure.
// + intIndex        ::   int     :.Interface Index
// RETURN              :: BOOL
BOOL NetworkIpIsUnnumberedInterface(Node* node, int intIndex);

// FUNCTION            :: NetworkIpGetIpAddressForUnnumberedInterface
// LAYER               :: Network
// PURPOSE             :: getting IP address for unnumbered Interface.
// PARAMETERS          ::
// + node              :: Node*   : Pointer to node structure.
// + intIndex        ::   int     :.Interface Index
// RETURN              :: NodeAddress
NodeAddress NetworkIpGetIpAddressForUnnumberedInterface(Node* node,
                                                        int intIndex);

/// Determines if an interface is a wired interface.
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
///
/// \return BOOL
BOOL
NetworkIpIsWiredBroadcastNetwork(Node *node, int interfaceIndex);


// -----------------------------------------------------------------------------
// API :: GetDefaultInterfaceIndex
// PURPOSE :: Returns Default Interface index of depending on network type
// PARAMETERS ::
// + node : Node* : Pointer to the Node
// + sourceNodeId : NodeId
// + networkType : NetoworkType
// RETURN :: int
// -----------------------------------------------------------------------------
int
GetDefaultInterfaceIndex(
    Node* node,
    NetworkType netType);

// STATS DB CODE
#ifdef ADDON_DB
void NetworkIpConvertProtocolTypeToString(NetworkRoutingProtocolType type,
        std::string* protocol);

void NetworkIpConvertAdminDistanceToString(NetworkRoutingAdminDistanceType type,
        std::string adminDistString);
void NetworkIpConvertIpProtocolNumToString(unsigned char type,
        std::string* protocol);
void NetworkIpConvertMacProtocolTypeToString(
    MAC_PROTOCOL type,
    std::string *macProtocolStr);
#endif

BOOL IsDataPacket(Message* msg, IpHeaderType* IpHeader);


// -------------------------------------------------------------------------
// API :: GetDefaultInterfaceAddress
// PURPOSE :: Returns Default ipv4 Interface address
// PARAMETERS ::
// + node : Node* : Pointer to the Node
// RETURN :: NodeAddress
// -------------------------------------------------------------------------
NodeAddress GetDefaultIPv4InterfaceAddress(Node* node);

// -------------------------------------------------------------------------
// API :: NetworkIpMibsInit
// PURPOSE :: Initializes MIBS stats
// PARAMETERS ::
// + node : Node* : Pointer to the Node
// RETURN :: None
// -------------------------------------------------------------------------
void NetworkIpMibsInit(Node* node);


//---------------------------------------------------------------------------
// FUNCTION     NetworkIpHeaderCheck()
// PURPOSE      To check the header of incoming packet for errors in case
//              ICMP is senabled
// PARAMETERS   Node *node - Pointer to node.
//              Message *msg - Message pointer
//              int incomingInterface - incoming interface
// RETURN       BOOL.
//---------------------------------------------------------------------------

BOOL NetworkIpHeaderCheck(
    Node *node,
    Message *msg,
    int incomingInterface);


/// Searches the IP header for the Traceroute option field ,
/// and returns a pointer to traceroute header.
///
/// \param ipHeader  Pointer to an IP header.
///
/// \return pointer to the header of the traceroute
/// option field. NULL if no option fields, or the desired
/// option field cannot be found.

ip_traceroute *FindTraceRouteOption(const IpHeaderType *ipHeader);

//-----------------------------------------------------------------------------
// FUNCTION     IpHeaderSourceRouteOptionField()
// PURPOSE      Returns the source route contained in IP packet.
// PARAMETERS   IpHeaderType *ipHeader
//                  Pointer to IP header.
// RETURNS      Pointer to header of source route option field, if
//              source route is present.
//              NULL, if source route is not present.
//-----------------------------------------------------------------------------

IpOptionsHeaderType *
IpHeaderSourceRouteOptionField(IpHeaderType *ipHeader);

//--------------------------------------------------------------------------
// Record route
//--------------------------------------------------------------------------

IpOptionsHeaderType * //inline//
IpHeaderRecordRouteOptionField(IpHeaderType *ipHeader);

//--------------------------------------------------------------------------
// TimeStamp Option
//--------------------------------------------------------------------------

IpOptionsHeaderType * //inline//
IpHeaderTimestampOptionField(IpHeaderType *ipHeader);


//-----------------------------------------------------------------------------
// FUNCTION     AddIpHeader()
// PURPOSE      Add an IP packet header to a message.
//              The new message has an IP packet.
// PARAMETERS   Node *node
//                  Pointer to node.
//              Message *msg
//                  Pointer to message.
//              NodeAddress sourceAddress
//                  Source IP address.
//              NodeAddress destinationAddress
//                  Destination IP address.
//              TosType priority
//                  Currently a TosType.
//                  (values are not standard for "IP type of service field"
//                  but has correct function)
//              unsigned char protocol
//                  IP protocol number.
//              unsigned ttl
//                  Time to live.
//                  If 0, uses default value IPDEFTTL, as defined in
//                  include/ip.h.
// RETURN       None.
//-----------------------------------------------------------------------------

void
AddIpHeader(
    Node *node,
    Message *msg,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    TosType priority,
    unsigned char protocol,
    unsigned ttl);


//-----------------------------------------------------------------------------
// IP header
//-----------------------------------------------------------------------------


void //inline//
ExpandOrShrinkIpHeader(
    Node *node,
    Message *msg,
    int newHeaderSize);



//-----------------------------------------------------------------------------
// FUNCTION     NetworkIpAddHeaderWithOptions()
// PURPOSE      Add an IP packet header to a message.
//              The new message has an IP packet.
// PARAMETERS   Node *node
//                  Pointer to node.
//              Message *msg
//                  Pointer to message.
//              NodeAddress sourceAddress
//                  Source IP address.
//              NodeAddress destinationAddress
//                  Destination IP address.
//              TosType priority
//                  Currently a TosType.
//                  (values are not standard for "IP type of service field"
//                  but has correct function)
//              unsigned char protocol
//                  IP protocol number.
//              unsigned ttl
//                  Time to live.
//                  If 0, uses default value IPDEFTTL, as defined in
//                  include/ip.h.
// RETURN       None.
//-----------------------------------------------------------------------------

void
NetworkIpAddHeaderWithOptions(
    Node *node,
    Message *msg,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    TosType priority,
    unsigned char protocol,
    unsigned ttl,
    int ipHeaderLength,
    char *ipOptions);




//igmp_compliance_changes_start
// -----------------------------------------------------------------------
//FUNCTION            :: IsIgmpPacket
// PURPOSE            :: Checks whether this packet is an IGMP packet and
//                       whether IGMP is enabled for this node.
// PARAMETERS ::
// + node : Node*     : Pointer to the Node
// + ipHeader->ip_p   : ip protocol in the ip header of this message.
// -----------------------------------------------------------------------
bool
IsIgmpPacket(Node* node, unsigned char ip_protocol);
//igmp_compliance_changes_end

void
IAHEPSendIGMPMessageToMacLayer(
    Node *node,
    Message *msg,
    NodeAddress sourceAddress,
    NodeAddress destinationAddress,
    TosType priority,
    unsigned char protocol,
    unsigned ttl,
    int outgoingInterface,
    NodeAddress nextHop);
//---------------------------------------------------------------------------
// FUNCTION     NetworkIpGetRoutingProtocolInstance()
// PURPOSE      Get routing protocol structure associated with routing
//              protocol running on this interface.
// PARAMETERS   Node *node
//                  Pointer to node.
//              NetworkRoutingProtocolType routingProtocolType
//                  Routing protocol to retrieve.
//              unsigned int intfIndex
//                  interface index of desired routing protocol instance
// RETURN       Routing protocol structure requested.
//---------------------------------------------------------------------------

void *
NetworkIpGetRoutingProtocolInstance(
    Node *node,
    NetworkRoutingProtocolType routingProtocolType,
    unsigned int intfIndex);

//---------------------------------------------------------------------------
// FUNCTION     NetworkIpGetRoutingProtocolInstanceByInstanceId()
// PURPOSE      Get routing protocol structure associated with routing
//              protocol with given instance id.
// PARAMETERS   Node *node
//                  Pointer to node.
//              NetworkRoutingProtocolType routingProtocolType
//                  Routing protocol to retrieve.
//              unsigned int instanceId
//                  instance id desired routing protocol.
// RETURN       Routing protocol structure requested.
//---------------------------------------------------------------------------
void *
NetworkIpGetRoutingProtocolInstanceByInstanceId(
    Node *node,
    NetworkRoutingProtocolType routingProtocolType,
    unsigned int instanceId);

BOOL
IsDuplicatePacket(Node* node,
        int interfaceIndex,
        Message* msg);

void NetworkIpAddPacketSentToMacDataPoints(
    Node* node,
    Message* msg,
    int interfaceIndex);

BOOL //inline
IsMyPacket(Node *node, NodeAddress destAddress, NodeAddress srcAddress = 0);

// Dynamic Address
/// enumeration to define address change events by DHCP
enum AddressChangeType
{
    INIT_DHCP, // init state of DHCP
    LEASE_EXPIRY, // lease expiry state
    RECEIVE_ADDRESS, // address receive
    INFORM_DHCP // inform state of DHCP
};

BOOL NetworkIpCheckMulticastRoutingProtocol(
    Node* node,
    NetworkRoutingProtocolType routingProtocolType,
    int interfaceId);
// Dynamic Address
//---------------------------------------------------------------------------
// API              :: NetworkIpProcessDHCP
// LAYER            :: Network
// PURPOSE          :: interface which handles address change event from DHCP
// PARAMETERS       ::
// + node           : Node* : Pointer to node.
// + interfaceIndex : Int32 : Interface Index
// + address        : Address* : newaddress address
// + subnetMask     : NodeAddress : subnet Mask
// + messageType    : AddressChangeType : address change event type
// +  networkType   : NetworkType : network type
// RETURN           : void      : NULL.
//---------------------------------------------------------------------------

void NetworkIpProcessDHCP(Node* node,
                          Int32 interfaceIndex,
                          Address* address,
                          NodeAddress subnetMask,
                          AddressChangeType messageType,
                          NetworkType networkType);

//---------------------------------------------------------------------------
// API              :: IpSendNotificationOfAddressChange
// LAYER            :: Network
// PURPOSE          :: Allows the ip layer to send address changed
//                     notification to all the protocols which are using old
//                     address.
// PARAMETERS       ::
// node             : Node* : Pointer to node.
// + interfaceIndex : Int32 : Interface Index
// + address        : Address* : newaddress
// + subnetMask     : NodeAddress : subnet Mask
// + networkType    : NetworkType : networkType
// RETURN           : void      : NULL.
//---------------------------------------------------------------------------

void IpSendNotificationOfAddressChange(Node* node,
                                       Int32 interfaceIndex,
                                       Address* address,
                                       NodeAddress subnetMask,
                                       NetworkType networkType);

//---------------------------------------------------------------------------
// Function Pointer Type :: IpAddressChangedHandlerFunctionType
// DESCRIPTION           :: Instance of this type is used to
//                          register address change handler Functions.
//---------------------------------------------------------------------------

typedef
void (*IpAddressChangedHandlerFunctionType)(
    Node* node,
    const int interfaceIndex,
    Address* address,
    NodeAddress subnetMask,
    NetworkType networkType);


//---------------------------------------------------------------------------
// API              :: NetworkIpAddAddressChangedHandlerFunction
// LAYER            :: Network
// PURPOSE          :: Add a address change handler function to the List,This
//                     handler will be called when address changes.
// PARAMETERS       ::
// + node           : Node* : Pointer to node.
// + addressChangeFunctionPointer :
//                  : IpAddressChangedHandlerFunctionType : function pointer
// RETURN           :: void  : NULL.
//---------------------------------------------------------------------------
void NetworkIpAddAddressChangedHandlerFunction(
           Node* node,
           IpAddressChangedHandlerFunctionType addressChangeFunctionPointer);


//--------------------------------------------------------------------------
// API              :: NetworkIpIsDhcpPacket
// LAYER            :: Network
// PURPOSE          :: Checks whether packet is a DHCP packet
// PARAMETERS       ::
// + msg           : Message* : packet
// RETURN           :: BOOL  : TRUE : if packet is DHCP packet
//                             FALSE : if packet is not DHCP packet
//--------------------------------------------------------------------------
bool NetworkIpIsDhcpPacket(Message* msg);

//--------------------------------------------------------------------------
// API              :: NetworkIpIsValidSourceAddressState
// LAYER            :: Network
// PURPOSE          :: Checks whether address state of source interface is
//                     valid
// PARAMETERS       ::
// + node           : Node* : node
// + address        : NodeAddress : address which need to be checked
// + interfaceIndex : Int32 : interface whose address state need to be
//                            checked
// RETURN           :: BOOL  : TRUE : if address state is valid
//                             FALSE : if address state is invalid
//--------------------------------------------------------------------------

bool NetworkIpIsValidSourceAddressState(Node* node,
                                        NodeAddress address,
                                        Int32 interfaceIndex);

//--------------------------------------------------------------------------
// API              :: NetworkIpCheckIfAddressLoopBack
// LAYER            :: Network
// PURPOSE          :: Checks whether an address is loopback address or not
// PARAMETERS       ::
// + node           : Node*   : node
// + address        : Address : address which need to be checked
// RETURN           :: bool  : TRUE : if address is loopback
//                             FALSE : if address is not loopback
//--------------------------------------------------------------------------

bool
NetworkIpCheckIfAddressLoopBack(Node* node, Address address);

//
// FUNCTION       :: isValidIpAddress
// PURPOSE        :: Check if a numeric string is a valid IPv4 address.
// PARAMETERS     ::
// + addressString : const char - address (without mask)
//                    to determine if valid IPv4 address.
// RETURN         :: true if address is a valid IP address, false, otherwise
//
bool isValidIpAddress(const char addressString[]);
#endif // _IP_H_
