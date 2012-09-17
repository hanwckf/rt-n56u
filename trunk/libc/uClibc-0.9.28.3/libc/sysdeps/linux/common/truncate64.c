/*
 * truncate64 syscall.  Copes with 64 bit and 32 bit machines
 * and on 32 bit machines this sends things into the kernel as
 * two 32-bit arguments (high and low 32 bits of length) that 
 * are ordered based on endianess.  It turns out endian.h has
 * just the macro we need to order things, __LONG_LONG_PAIR.
 *
 *  Copyright (C) 2002  Erik Andersen <andersen@codepoet.org>
 *
 * This file is subject to the terms and conditions of the GNU
 * Lesser General Public License.  See the file COPYING.LIB in
 * the main directory of this archive for more details.
 */

#include <features.h>
#include <unistd.h>
#include <errno.h>
#include <endian.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/syscall.h>

#if defined __UCLIBC_HAS_LFS__

#if defined __NR_truncate64

#if __WORDSIZE == 64

/* For a 64 bit machine, life is simple... */
_syscall2(int, truncate64, const char *, path, __off64_t, length);

#elif __WORDSIZE == 32

#ifndef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...) __syscall_truncate64 (args)
#define __NR___syscall_truncate64 __NR_truncate64
#if defined(__powerpc__) || defined(__mips__)
static inline _syscall4(int, __syscall_truncate64, const char *, path,
	uint32_t, pad, unsigned long, high_length, unsigned long, low_length);
#else
static inline _syscall3(int, __syscall_truncate64, const char *, path,
	unsigned long, high_length, unsigned long, low_length);
#endif
#endif


/* The exported truncate64 function.  */
int truncate64 (const char * path, __off64_t length)
{
    uint32_t low = length & 0xffffffff;
    uint32_t high = length >> 32;
#if defined(__powerpc__) || defined(__mips__)
    return INLINE_SYSCALL(truncate64, 4, path, 0,
	    __LONG_LONG_PAIR (high, low));
#else
    return INLINE_SYSCALL(truncate64, 3, path,
	    __LONG_LONG_PAIR (high, low));
#endif
}

#else /* __WORDSIZE */
#error Your machine is not 64 bit or 32 bit, I am dazed and confused.
#endif /* __WORDSIZE */

#else  /* __NR_truncate64 */

int truncate64 (const char * path, __off64_t length)
{
	__off_t x = (__off_t) length;

	if (x == length) {
		return truncate(path, x);
	}

	__set_errno((x < 0) ? EINVAL : EFBIG);

	return -1;
}

#endif /* __NR_truncate64 */

#endif /* __UCLIBC_HAS_LFS__ */

