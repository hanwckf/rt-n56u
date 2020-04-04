/* Copyright (C) 1991,92,93,95,96,97,98,99,2000,2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LIBC_INTERNAL_H
#define _LIBC_INTERNAL_H 1

#include <features.h>

#ifdef __UCLIBC_BUILD_RELRO__
# define attribute_relro __attribute__ ((section (".data.rel.ro")))
#else
# define attribute_relro
#endif

#ifdef __UCLIBC_HAS_TLS__
# define attribute_tls_model_ie __attribute__ ((tls_model ("initial-exec")))
#else
# define attribute_tls_model_ie
# define __thread
#endif

/* Pull in things like __attribute_used__ */
#include <sys/cdefs.h>

/* --- this is added to integrate linuxthreads */
/*#define __USE_UNIX98            1*/

#ifndef __ASSEMBLER__
# ifdef IS_IN_libc

#  define __need_size_t
#  include <stddef.h>

/* sources are built w/ _GNU_SOURCE, this gets undefined */
#if defined __USE_XOPEN2K && !defined  __USE_GNU
extern char *__glibc_strerror_r (int __errnum, char *__buf, size_t __buflen);
libc_hidden_proto(__glibc_strerror_r)
#else
extern int __xpg_strerror_r (int __errnum, char *__buf, size_t __buflen);
libc_hidden_proto(__xpg_strerror_r)
#endif

/* #include <pthread.h> */
#  ifndef __UCLIBC_HAS_THREADS__
#   define __pthread_mutex_init(mutex, mutexattr)         ((void)0)
#   define __pthread_mutex_lock(mutex)                    ((void)0)
#   define __pthread_mutex_trylock(mutex)                 ((void)0)
#   define __pthread_mutex_unlock(mutex)                  ((void)0)
#   define _pthread_cleanup_push_defer(mutex)             ((void)0)
#   define _pthread_cleanup_pop_restore(mutex)            ((void)0)
#  endif

/* internal access to program name */
extern const char *__uclibc_progname attribute_hidden;

#  ifdef __UCLIBC_HAS_SSP__
extern void __stack_chk_fail(void) attribute_noreturn __cold;
#  endif

# endif /* IS_IN_libc */

#endif /* __ASSEMBLER__ */

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Some people like to build up uClibc with *-elf toolchains, so
 * a little grease here until we drop '#ifdef __linux__' checks
 * from our source code.
 */
#ifndef __linux__
# define __linux__ 1
#endif

#endif /* _LIBC_INTERNAL_H */
