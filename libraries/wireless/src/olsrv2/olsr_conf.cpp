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

/*
 *
 * Copyright (c) 2006, Graduate School of Niigata University,
 *                                         Ad hoc Network Lab.
 * Developer:
 *  Yasunori Owada  [yowada@net.ie.niigata-u.ac.jp],
 *  Kenta Tsuchida  [ktsuchi@net.ie.niigata-u.ac.jp],
 *  Taka Maeno      [tmaeno@net.ie.niigata-u.ac.jp],
 *  Hiroei Imai     [imai@ie.niigata-u.ac.jp].
 * Contributor:
 *  Keita Yamaguchi [kyama@net.ie.niigata-u.ac.jp],
 *  Yuichi Murakami [ymura@net.ie.niigata-u.ac.jp],
 *  Hiraku Okada    [hiraku@ie.niigata-u.ac.jp].
 *
 * This software is available with usual "research" terms
 * with the aim of retain credits of the software.
 * Permission to use, copy, modify and distribute this software for any
 * purpose and without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies,
 * and the name of NIIGATA, or any contributor not be used in advertising
 * or publicity pertaining to this material without the prior explicit
 * permission. The software is provided "as is" without any
 * warranties, support or liabilities of any kind.
 * This product includes software developed by the University of
 * California, Berkeley and its contributors protected by copyrights.
 */

#include <stdio.h>

#include "api.h"
#include "network_ip.h"
#include "external_socket.h"
#include "olsr.h"
#include "olsr_util_inline.h"

/*static
int getPrefixLength(olsr_u32_t );*/

void
GetIpAddr(struct olsrv2* olsr, union olsr_ip_addr* addr, char* str)
{
//#if defined (WIN32) || defined (_WIN32) || defined (__WIN64)
    if (olsr->olsr_cnf->ip_version == AF_INET)
    {
    int i[4], n, j;
    sscanf(str, "%d.%d.%d.%d",&i[0],&i[1],&i[2],&i[3]);
//changed for MAC mismatch
    /*  for (n = 0; n <= 3; n++)
    {
        addr->v4 += i[n] << (n*8);
    }*/
        if (GetMachineType()== B_ENDIAN)
        {
            for (n = 3, j=0; n >= 0; n--, j++)
            {
                addr->v4 += i[n] << (j*8);
            }
        }
        else
        {
            for (n = 0; n <= 3; n++)
            {
                addr->v4 += i[n] << (n*8);
            }
        }
//changed for MAC mismatch end
    }
    else
    {
    int i[8], n;
    sscanf(str, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",&i[0],&i[1],&i[2],&i[3],&i[4],&i[5],&i[6],&i[7]);
    for (n = 0; n <= 7; n++)
    {
        //addr->v6.s6_addr_8[n] = (olsr_u8_t)(addr->v6.s6_addr_8[n] + i[n]);
        addr->v6.s6_addr_16[n] = (olsr_16_t) i[n];
        EXTERNAL_ntoh(&addr->v6.s6_addr_16[n], sizeof(olsr_u16_t));
    }
    }

/*#else
 if (olsr->olsr_cnf->ip_version == AF_INET)
 {
     int i[4], n;
     sscanf(str, "%d.%d.%d.%d",&i[0],&i[1],&i[2],&i[3]);

    for (n = 0; n <= 3; n++)
    {
        addr->v4 += i[n] << (n*8);
    }
 }
 else
 {
     int i[8], n;
     sscanf(str, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",&i[0],&i[1],&i[2],&i[3],&i[4],&i[5],&i[6],&i[7]);

     for (n = 0; n <= 7; n++)
     {
     addr->v6.s6_addr_8[n] += i[n];
     }
 }

 // inet_pton(olsr->olsr_cnf->ip_version, str, addr);
#endif*/
}

//Shifted from routing_olsrv2_niigata.cpp for MAC mismatch
MACHINE_TYPE GetMachineType(void)
{
    union type{
        char a;
        int b;
    }m_type;

    m_type.b = 0xff;

    if (m_type.a == 0)
    {
    return B_ENDIAN;
    }
    else
    {
    return L_ENDIAN;
    }
}
