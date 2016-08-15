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

#if !defined(__PPC_ATOMIC_H__)
  #define __PPC_ATOMIC_H__

typedef struct { volatile int value; } UTIL_AtomicInteger;

#if defined(CONFIG_SMP)
  #define SYNC "sync\n\t"
#else
  #define SYNC ""
#endif

#define UTIL_AtomicValue(x) { (x) }

static void UTIL_AtomicSet(UTIL_AtomicInteger *v, register int x) { 
    int tmp;

    __asm__ __volatile__(
    "0: lwarx %0,0,%1\n\t"
    SYNC
    "stwcx. %2,0,%1\n\t"
    "bne- 0b"
    : "=&b" (tmp)
    : "r" (&v->value), "r" (x)
    : "cr0", "r4"
    );
}
static int UTIL_AtomicRead(UTIL_AtomicInteger *v) { 
    int tmp;

    __asm__ __volatile__(
    "0: lwarx %0,0,%1\n\t"
    SYNC
    "stwcx. %0,0,%1\n\t"
    "bne- 0b"
    : "=&b" (tmp)
    : "r" (&v->value)
    : "cr0"
    );

    return tmp;
} 

static void UTIL_AtomicAdd(UTIL_AtomicInteger *v, int i) {
    int tmp;

    __asm__ __volatile__(
        "0: lwarx %0,0,%1\n\t"
        "add %0, %0, %2\n\t"
        SYNC
        "stwcx. %0,0,%1\n\t"
        "bne- 0b\n\t"
        : "=&b" (tmp)
        : "r" (&v->value), "Ir"(i)
        : "cr0"
    );
}
static void UTIL_AtomicSubtract(UTIL_AtomicInteger *v, int i) {
    int tmp;

    __asm__ __volatile__(
        "0: lwarx %0,0,%1\n\t"
        "subf %0, %0, %2\n\t"
        SYNC
        "stwcx. %0,0,%1\n\t"
        "bne- 0b\n\t"
        : "=&r" (tmp)
        : "r" (&v->value), "Ir" (i)
        : "cr0"
    );
}

static void UTIL_AtomicIncrement(UTIL_AtomicInteger *v) {
    int tmp;

    __asm__ __volatile__(
        "0: lwarx %0, 0, %1\n\t"
        "addic %0, %0, 1\n\t"
        SYNC
        "stwcx. %0, 0, %1\n\t"
        "bne- 0b"
        : "=&r" (tmp)
        : "r" (&v->value)
        : "cr0"
    );
}

static void UTIL_AtomicDecrement(UTIL_AtomicInteger *v) {
    register int tmp;

    __asm__ __volatile__(
        "0: lwarx %0, 0, %1\n\t"
        "addic %0, %0, -1\n\t"
        SYNC
        "stwcx. %0, 0, %1\n\t"
        "bne- 0b"
        : "=&r" (tmp)
        : "r" (&v->value)
        : "cr0"
    );
}

static int UTIL_AtomicDecrementAndTest(UTIL_AtomicInteger *v) {
    int tmp;

    __asm__ __volatile__(
        "0: lwarx %0, 0, %1\n\t"
        "addic %0, %0, -1\n\t"
        SYNC
        "stwcx. %0, 0, %1\n\t"
        "bne- 0b"
        : "=&r" (tmp)
        : "r" (&v->value)
        : "cr0"
    );

    return tmp == 0 ? 1 : 0;
}

#endif
