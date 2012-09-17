/* Machine-dependent pthreads configuration and inline functions.
   Copyright (C) 1996, 1998, 2000, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <features.h>

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

#include <asm/fixed_code.h>

/* Spinlock implementation; required.  */
/* The semantics of the TESTSET instruction cannot be guaranteed. We cannot
   easily move all locks used by linux kernel to non-cacheable memory.
   EXCPT 0x4 is used to trap into kernel to do the atomic testandset.
   It's ugly. But it's the only thing we can do now.
   The handler of EXCPT 0x4 expects the address of the lock is passed through
   R0. And the result is returned by R0.  */
PT_EI long int
testandset (int *spinlock)
{
    long int res;

    __asm__ __volatile__ (
		"CALL (%4);"
		: "=q0" (res), "=m" (*spinlock)
		: "qA" (spinlock), "m" (*spinlock), "a" (ATOMIC_XCHG32), "q1" (1)
		: "RETS", "cc", "memory");

    return res;
}

#define HAS_COMPARE_AND_SWAP
PT_EI int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
    long int readval;
    __asm__ __volatile__ (
		"CALL (%5);"
		: "=q0" (readval), "=m" (*p)
		: "qA" (p),
		  "q1" (oldval),
		  "q2" (newval),
		  "a" (ATOMIC_CAS32),
		  "m" (*p)
		: "RETS", "memory", "cc");
    return readval == oldval;
}

#ifdef SHARED
# define PTHREAD_STATIC_FN_REQUIRE(name)
#else
# define PTHREAD_STATIC_FN_REQUIRE(name) __asm__ (".globl " "_"#name);
#endif

#endif /* pt-machine.h */
