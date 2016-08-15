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
#ifndef __PROC_ROUTING_SET_H
#define __PROC_ROUTING_SET_H

#include <time.h>

#include "olsr_types.h"
#include "olsr_common.h"
#include "olsr_list.h"

#include "olsr.h"

typedef struct olsr_routing_tuple
{
  union olsr_ip_addr R_dest_iface_addr;
  union olsr_ip_addr R_next_iface_addr;
  olsr_u8_t R_prefix_length;
  olsr_u16_t R_dist;
  union olsr_ip_addr R_iface_addr;
}OLSR_ROUTING_TUPLE;


/* function prototypes */

#if defined __cplusplus
extern "C" {
#endif
void print_routing_set(struct olsrv2 *);

OLSR_LIST *search_routing_set_for_next_and_dest(
                   struct olsrv2 *,
                   union olsr_ip_addr *,
                   const union olsr_ip_addr *);

OLSR_LIST *search_routing_set_old_for_next_and_dest(
                   struct olsrv2 *,
                   union olsr_ip_addr *,
                   const union olsr_ip_addr *);

void insert_routing_set_old(
                struct olsrv2 *,
                const union olsr_ip_addr *,
                const union olsr_ip_addr *,
                olsr_u8_t,  //prefix length
                olsr_u16_t,
                const union olsr_ip_addr *);

void delete_routing_set_old(struct olsrv2 *olsr,
                  union olsr_ip_addr *,
                  const union olsr_ip_addr *);

void insert_routing_set(struct olsrv2 *,
            union olsr_ip_addr *,
            const union olsr_ip_addr *,
            olsr_u8_t,
            olsr_u16_t,
            const union olsr_ip_addr *);

#if defined __cplusplus
}
#endif

void init_routing_set(struct olsrv2 *);

olsr_bool check_recalculate_routing_set(struct olsrv2 *);
void reset_recalculate_routing_set(struct olsrv2 *);

void proc_routing_set(struct olsrv2 *);


#endif
