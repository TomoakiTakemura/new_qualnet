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
#ifndef OLSR_DEBUG_OLSRV2_H
#define OLSR_DEBUG_OLSRV2_H

#include <stdio.h>
#include <stdlib.h>
/* for syslog */
#include <stdarg.h>
#if defined (WIN32) || defined (_WIN32) || defined (__WIN64)

    //#include <varargs.h>
    #ifndef __func__
        #define __func__ __FUNCTION__
    #endif

    #define vsnprintf _vsnprintf

#else

    #include <syslog.h>

#endif

#define STDOUT stdout
#define STDERR stderr
#define DEF_LEVEL -1

/* for syslog */
#define OLSR_LOG_INFO   1
#define OLSR_LOG_WARNING 2
#define OLSR_LOG_ERROR  3

/* function prototype */

  void init_olsr_debug(void);
  void set_output_level(int);
  void olsr_error(const char *, ...);

  void olsr_printf(const char *, ...);

/*#define olsr_debug(level,format,...)\
{} */

#define debug_code(x)   {}

extern int d_level;
void olsr_die(const char *, ...);


void set_debug_handler(const char *);
void set_std_handler(const char *);
int get_output_level();
FILE *get_debug_handler();

/* for syslog */
void olsr_openlog(const char *);
void olsr_syslog(int, char *, ...);
void olsr_closelog(void);
void olsr_shutdown(int);
#endif /* ifndef OLSR_DEBUG_OLSRV2_H */
