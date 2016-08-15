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
#ifndef __OLSR_TLV_COMPLESS_H
#define __OLSR_TLV_COMPLESS_H

#include "pktbuf.h"

typedef struct base_tlv
{
  olsr_u8_t tlv_type;
  olsr_u8_t tlv_semantics;
  olsr_u8_t tlv_length;
  olsr_u8_t index_start;
  olsr_u8_t index_stop;
  olsr_u8_t *value;
} BASE_TLV;

/* function prototypes */
void build_tlv_block_tc(olsr_pktbuf_t *);
void build_tlv_block_local(olsr_pktbuf_t *);
void build_tlv_block_for_index_hello(olsr_pktbuf_t *, BASE_ADDRESS *,
    olsr_u32_t);
void build_tlv_block_for_index_local(olsr_pktbuf_t *, BASE_ADDRESS *,
                     olsr_u32_t);
void build_tlv_local_by_list(void *,olsr_pktbuf_t *, OLSR_LIST *);
void build_tlv_hello_by_list(void *, olsr_pktbuf_t *, OLSR_LIST *);

void print_other_neigh_status(olsr_u8_t);
void print_link_status(olsr_u8_t);
void print_interface(olsr_u8_t);
void print_mpr_selection(olsr_bool);

void
build_tlv_attached_network_by_list(void *olsr, olsr_pktbuf_t *, OLSR_LIST *);
void
build_tlv_block_for_index_attached_network(olsr_pktbuf_t *,
                 BASE_ADDRESS *,
                 olsr_u32_t );


#endif /* __OLSR_TLV_COMPLESS_H */
