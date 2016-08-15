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


#include "proxy_util_atomic.h"

#if defined(USE_ATOMIC_WIN)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// #define WIN32_USE_ASM


void UTIL_AtomicSet(UTIL_AtomicInteger *v, int x)
{
#if defined(WIN32_USE_ASM)
    long *p = (long*)&v->value;
    __asm
    {
        mov EAX, x
        mov EBX, p
        lock mov [EBX], EAX
    }
#else
    InterlockedExchange((long*)&v->value, x);
#endif
}

int UTIL_AtomicRead(UTIL_AtomicInteger *v)
{
    int t;
#if defined(WIN32_USE_ASM)
    long *p = (long*)&v->value;
    __asm
    {
        mov EAX, p
        lock mov EBX, [EAX]
        mov t, EBX
    }
    return t;
#else
    return v->value;
#endif
}

int UTIL_AtomicDecrementAndTest(UTIL_AtomicInteger *v)
{
#if defined(WIN32_USE_ASM)
    long *p = (long*)&v->value;
    unsigned char r;
    __asm
    {
        mov EAX, p
        lock dec [EAX]
        setz r
    }
    return (r == 1) ? 1 : 0;
#else
    int tmp = InterlockedDecrement((long*)&v->value);
    return tmp == 0 ? 1 : 0;
#endif
}

#endif /* USE_ATOMIC_WIN */
