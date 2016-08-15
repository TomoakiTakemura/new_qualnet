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
/*
 */

#include "generate_msg.h"
#include "def.h"
#include "olsr.h"
#include "olsr_conf.h"
#include "olsr_protocol.h"
#include "address_compress.h"
#include "proc_relay_set.h"
#include "pktbuf.h"
#include "mantissa.h"


#define PULSE_MAX 4

static void build_message_tlv_validity_time(olsr_pktbuf_t *,
    const olsr_u8_t t_default);
static void build_message_tlv_interval_time(olsr_pktbuf_t *,
    const olsr_u8_t time);

static void build_message_tlv_willingness(olsr_pktbuf_t *,
    const olsr_u8_t willingness);

static void build_message_tlv_content_seq_num(olsr_pktbuf_t *,
    olsr_u16_t seq_num);
static void build_local_addr_block(struct olsrv2 *olsr,
                   olsr_pktbuf_t *local_addr_buf,
                   union olsr_ip_addr *);

void
build_hello_msg(struct olsrv2 *olsr, olsr_pktbuf_t *pktbuf,
        union olsr_ip_addr *local_iface_addr
        )
{
  //olsr_u8_t *msg, *msg_head;
  //struct packet_header *p_header;
  //static olsr_pktbuf_t *msg_tlv_buf = NULL;
  //static olsr_pktbuf_t *addr_block_buf = NULL;
  //static olsr_pktbuf_t *local_addr_block_buf = NULL;
   olsr_pktbuf_t *msg_tlv_buf = NULL;
   olsr_pktbuf_t *addr_block_buf = NULL;
   olsr_pktbuf_t *local_addr_block_buf = NULL;

  // initialize buffers
  if (!msg_tlv_buf)
    msg_tlv_buf = olsr_pktbuf_alloc_with_capa(64);
  olsr_pktbuf_clear(msg_tlv_buf);

  if (!addr_block_buf)
    addr_block_buf = olsr_pktbuf_alloc_with_capa(64);
  olsr_pktbuf_clear(addr_block_buf);

  if(!local_addr_block_buf)
    local_addr_block_buf = olsr_pktbuf_alloc_with_capa(64);
  olsr_pktbuf_clear(local_addr_block_buf);

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Building TLV for Validity Time.\n");
  }
  // build message TLVs
  build_message_tlv_validity_time(msg_tlv_buf,
                   double_to_me((double)olsr->qual_cnf->neighbor_hold_time));

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Building TLV for Interval Time.\n");
  }
  build_message_tlv_interval_time(msg_tlv_buf,
                       double_to_me((double)olsr->qual_cnf->hello_interval));

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Building TLV for Willingness.\n");
  }
  build_message_tlv_willingness(msg_tlv_buf, WILL_DEFAULT);

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Building local interface address block.\n");
  }
  //build local interface address block
  build_local_addr_block(olsr, local_addr_block_buf,
             local_iface_addr);

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Building {<addr-block> <tlv-block>}*.\n");
  }
  // build {<addr-block> <tlv-block>}*
  address_compress(olsr, addr_block_buf, HELLO_MESSAGE, SIMPLE_COMPRESS,
      local_iface_addr);

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Append <msg-header> and <msg-header-info>.\n");
  }
  // append <msg-header> and <msg-header-info>
  build_message_header(olsr,
               pktbuf,
               (olsr_u16_t)(2 + msg_tlv_buf->len +
               local_addr_block_buf->len +
               addr_block_buf->len),// 2 is <tlv-length>
               HELLO_MESSAGE,
               local_iface_addr,
               1, 0, get_msg_seqno(olsr));

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Append <tlv-block>.\n");
  }
  // append <tlv-block>
  olsr_pktbuf_append_u16(pktbuf, (olsr_u16_t)msg_tlv_buf->len);
  olsr_pktbuf_append_pktbuf(pktbuf, msg_tlv_buf);

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Append local interface address block.\n");
  }
  // append local interface address block
  olsr_pktbuf_append_pktbuf(pktbuf, local_addr_block_buf);

  if(DEBUG_OLSRV2)
  {
    olsr_printf("Append {<addr-block> <tlv-block>}*.\n");
  }
  // append {<addr-block> <tlv-block>}*
  if(addr_block_buf != NULL)
    olsr_pktbuf_append_pktbuf(pktbuf, addr_block_buf);

  free(msg_tlv_buf->data);
  free(msg_tlv_buf);
  free(addr_block_buf->data);
  free(addr_block_buf);
  free(local_addr_block_buf->data);
  free(local_addr_block_buf);
}

/*
 *
 */
void
build_tc_msg(struct olsrv2 *olsr, olsr_pktbuf_t *pktbuf,
    union olsr_ip_addr *local_iface_addr
    )
{
  //olsr_u8_t *msg, *msg_head;
  //struct packet_header *p_header;
  //static olsr_pktbuf_t *msg_tlv_buf = NULL;
  //static olsr_pktbuf_t *addr_block_buf = NULL;
  //static olsr_pktbuf_t *local_addr_block_buf = NULL;
  olsr_pktbuf_t *msg_tlv_buf = NULL;
  olsr_pktbuf_t *addr_block_buf = NULL;
  olsr_pktbuf_t *local_addr_block_buf = NULL;

  // initialize buffers
  if (!msg_tlv_buf)
    msg_tlv_buf = olsr_pktbuf_alloc_with_capa(64);
  olsr_pktbuf_clear(msg_tlv_buf);

  if (!addr_block_buf)
    addr_block_buf = olsr_pktbuf_alloc_with_capa(64);
  olsr_pktbuf_clear(addr_block_buf);

  if(!local_addr_block_buf)
    local_addr_block_buf = olsr_pktbuf_alloc_with_capa(64);
  olsr_pktbuf_clear(local_addr_block_buf);

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Building TLV for Validity Time.\n");
  }
  // build message TLVs
  build_message_tlv_validity_time(msg_tlv_buf,
                   double_to_me((double)olsr->qual_cnf->topology_hold_time));

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Building TLV for Interval Time.\n");
  }
  build_message_tlv_interval_time(msg_tlv_buf,
                          double_to_me((double)olsr->qual_cnf->tc_interval));

  if(DEBUG_OLSRV2)
  {
    olsr_printf("Building TLV for Content Sequence Number.\n");
  }
  build_message_tlv_content_seq_num(msg_tlv_buf, get_local_assn(olsr));

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Building local interface address block.\n");
  }
  //build local interface address block
  build_local_addr_block(olsr, local_addr_block_buf,
             NULL);


  if(DEBUG_OLSRV2)
  {
      olsr_printf("Advertize attached network address.\n");
  }
  //advertize attached network address
  create_attached_network_address_block(olsr, addr_block_buf, TC_MESSAGE,
                    SIMPLE_COMPRESS,
                    local_iface_addr
                    );

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Building {<addr-block> <tlv-block>}*.\n");
  }
  // build {<addr-block> <tlv-block>}*
  address_compress(olsr, addr_block_buf, TC_MESSAGE, SIMPLE_COMPRESS,
      local_iface_addr
      );

  /* build message header */
  build_message_header(olsr,
               pktbuf,
               (olsr_u16_t)(2 + msg_tlv_buf->len +
               local_addr_block_buf->len +
               addr_block_buf->len),    // 2 is <tlv-length>
               TC_MESSAGE,
               local_iface_addr,
               MAX_TTL, 0, get_msg_seqno(olsr));

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Append <tlv-block>.\n");
  }
  // append <tlv-block>
  olsr_pktbuf_append_u16(pktbuf, (olsr_u16_t)msg_tlv_buf->len);
  olsr_pktbuf_append_pktbuf(pktbuf, msg_tlv_buf);

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Append local interface address block.\n");
  }
  // append local interface address block
  olsr_pktbuf_append_pktbuf(pktbuf, local_addr_block_buf);

  if(DEBUG_OLSRV2)
  {
      olsr_printf("Append {<addr-block> <tlv-block>}*.\n");
  }
  // append {<addr-block> <tlv-block>}*
  olsr_pktbuf_append_pktbuf(pktbuf, addr_block_buf);

  free(msg_tlv_buf->data);
  free(msg_tlv_buf);
  free(addr_block_buf->data);
  free(addr_block_buf);
  free(local_addr_block_buf->data);
  free(local_addr_block_buf);
}


/**
 * Create <msg-header> <msg-header-info> for ...
 *
 * <msg-header>
 * = <type> <msg-semantics> <msg-size> <msg-header-info>
 *
 * <msg-header-info>
 * = <originator-address>? <ttl>? <hop-count>? <msg-seq-number>?
 *
 * XXX XXX
 */
void
build_message_header(struct olsrv2 *olsr,
    olsr_pktbuf_t *pktbuf,
    olsr_u16_t msg_body_size,
    const int msg_type, const union olsr_ip_addr* originator,
    const int ttl, const int hopcount, const int message_seq_no)
{
  size_t top = pktbuf->len;
  size_t index_for_size;

  olsr_pktbuf_append_u8(pktbuf, (olsr_u8_t)msg_type);   // <type>
  olsr_pktbuf_append_u8(pktbuf, 0); // <semantics>
  // XXX MUST CHECK bit2 and bit3.
  index_for_size = pktbuf->len;
  olsr_pktbuf_append_u16(pktbuf, 0);    // <msg-size>

  olsr_pktbuf_append_ip(olsr, pktbuf, originator);  // <originator>

  olsr_pktbuf_append_u8(pktbuf, (olsr_u8_t)ttl);        // <ttl>
  olsr_pktbuf_append_u8(pktbuf, (olsr_u8_t)hopcount);   // <hop_count>
  olsr_pktbuf_append_u16(pktbuf, (olsr_u16_t) message_seq_no);

  olsr_pktbuf_put_u16(pktbuf, index_for_size,
      (olsr_u16_t)(msg_body_size + (pktbuf->len - top)));   // <msg-size>
}

/*
 *
 */
static void
build_message_tlv_validity_time(olsr_pktbuf_t *buf, const olsr_u8_t t_default)
{
  olsr_pktbuf_append_u8(buf, Validity_Time);    // type
  olsr_pktbuf_append_u8(buf, NO_INDEX); // semantics
  olsr_pktbuf_append_u8(buf, 1);    // length
  olsr_pktbuf_append_u8(buf, t_default);    // value XXX
}

static void
build_message_tlv_interval_time(olsr_pktbuf_t *buf, const olsr_u8_t time)
{
  olsr_pktbuf_append_u8(buf, Interval_Time);    // type
  olsr_pktbuf_append_u8(buf, NO_INDEX); // semantics
  olsr_pktbuf_append_u8(buf, 1);    // length
  olsr_pktbuf_append_u8(buf, time); // value XXX
}

static void
build_message_tlv_willingness(olsr_pktbuf_t *buf, olsr_u8_t willingness)
{
  olsr_pktbuf_append_u8(buf, Willingness);  // type
  olsr_pktbuf_append_u8(buf, NO_INDEX); // semantics
  olsr_pktbuf_append_u8(buf, 1);    // length
  olsr_pktbuf_append_u8(buf, willingness);  // value XXX
}

static void
build_message_tlv_content_seq_num(olsr_pktbuf_t *buf, olsr_u16_t seq_num)
{
  olsr_pktbuf_append_u8(buf, Content_Sequence_Number);  // type
  olsr_pktbuf_append_u8(buf, NO_INDEX); // semantics
  olsr_pktbuf_append_u8(buf, 2);    // length (= 2)
  olsr_pktbuf_append_u16(buf, seq_num);
}

// vim:sw=2:

static void build_local_addr_block(struct olsrv2 *olsr,
                   olsr_pktbuf_t *local_addr_block_buf,
                   union olsr_ip_addr *sender_iface_addr){
  address_compress(olsr, local_addr_block_buf, LOCAL_ADDRESS_BLOCK, SIMPLE_COMPRESS, sender_iface_addr );
}
