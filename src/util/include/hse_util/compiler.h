/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2020 Micron Technology, Inc.  All rights reserved.
 */

#ifndef HSE_PLATFORM_COMPILER_H
#define HSE_PLATFORM_COMPILER_H

/* Optimization barrier */
/* The "volatile" is due to gcc bugs */
#define barrier() asm volatile("" : : : "memory")

#define HSE_LIKELY(x)     __builtin_expect(!!(x), 1)
#define HSE_UNLIKELY(x)   __builtin_expect(!!(x), 0)
#define HSE_ALWAYS_INLINE inline __attribute__((always_inline))
#define HSE_PRINTF(a, b)  __attribute__((format(printf, a, b)))
#define HSE_PACKED        __attribute__((packed))
#define HSE_ALIGNED(SIZE) __attribute__((aligned(SIZE)))
#define HSE_READ_MOSTLY   __attribute__((section(".read_mostly")))
#define HSE_MAYBE_UNUSED  __attribute__((unused))
#define HSE_USED          __attribute__((used))
#define HSE_HOT           __attribute__((hot))
#define HSE_COLD          __attribute__((cold))

#if __STDC__VERSION__ <= 201112L
#define _Static_assert(...)
#endif

#ifndef _BullseyeCoverage
#define _BullseyeCoverage 0
#endif

#if _BullseyeCoverage
#define BullseyeCoverageSaveOff _Pragma("BullseyeCoverage save off")
#define BullseyeCoverageRestore _Pragma("BullseyeCoverage restore")

#ifndef _Static_assert
#define _Static_assert(...)
#endif

#else
#define BullseyeCoverageSaveOff
#define BullseyeCoverageRestore
#endif

#if __amd64__
static HSE_ALWAYS_INLINE void
cpu_relax(void)
{
    asm volatile("rep; nop" ::: "memory");
}

#else
#error cpu_relax() not implemented for this architecture
#endif

#endif /* HSE_PLATFORM_COMPILER_H */
