/*
 * sendfile64 syscall.  Copes with 64 bit and 32 bit machines
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
#include <sys/sendfile.h>
#include <sys/syscall.h>

#if defined __NR_sendfile64

#if __WORDSIZE == 64 || (defined(__powerpc__) && defined (__UCLIBC_HAS_LFS__))
/* For a 64 bit machine, life is simple... */
_syscall4(ssize_t,sendfile64, int, out_fd, int, in_fd, __off64_t *, offset, size_t, count);

#elif __WORDSIZE == 32

#if defined __UCLIBC_HAS_LFS__
_syscall4(ssize_t,sendfile64, int, out_fd, int, in_fd, __off64_t *, offset, size_t, count);
#endif /* __UCLIBC_HAS_LFS__ */

#else /* __WORDSIZE */
#error Your machine is not 64 bit or 32 bit, I am dazed and confused.
#endif /* __WORDSIZE */


#else /* ! defined __NR_sendfile64 */

ssize_t sendfile64 (int out_fd, int in_fd, __off64_t *offset, size_t count)
{
  __set_errno (ENOSYS);
  return -1;
}

#endif
