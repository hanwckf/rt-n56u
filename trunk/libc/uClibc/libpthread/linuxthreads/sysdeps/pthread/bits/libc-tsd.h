/* libc-internal interface for thread-specific data.  LinuxThreads version.
   Copyright (C) 1997,98,99,2001,02 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _BITS_LIBC_TSD_H
#define _BITS_LIBC_TSD_H 1

/* Fast thread-specific data internal to libc.  */
enum __libc_tsd_key_t { _LIBC_TSD_KEY_MALLOC = 0,
			_LIBC_TSD_KEY_DL_ERROR,
			_LIBC_TSD_KEY_RPC_VARS,
			_LIBC_TSD_KEY_LOCALE,
			_LIBC_TSD_KEY_CTYPE_B,
			_LIBC_TSD_KEY_CTYPE_TOLOWER,
			_LIBC_TSD_KEY_CTYPE_TOUPPER,
			_LIBC_TSD_KEY_N };

#include <sys/cdefs.h>
#include <tls.h>

#if USE_TLS && HAVE___THREAD

/* When __thread works, the generic definition is what we want.  */
# include <sysdeps/generic/bits/libc-tsd.h>

#else

extern void *(*__libc_internal_tsd_get) (enum __libc_tsd_key_t) __THROW;
extern int (*__libc_internal_tsd_set) (enum __libc_tsd_key_t,
				       __const void *)  __THROW;
extern void **(*const __libc_internal_tsd_address) (enum __libc_tsd_key_t)
     __THROW __attribute__ ((__const__));

#define __libc_tsd_address(KEY) \
  (__libc_internal_tsd_address != NULL \
   ? __libc_internal_tsd_address (_LIBC_TSD_KEY_##KEY) \
   : &__libc_tsd_##KEY##_data)

#define __libc_tsd_define(CLASS, KEY)	CLASS void *__libc_tsd_##KEY##_data;
#define __libc_tsd_get(KEY) \
  (__libc_internal_tsd_get != NULL \
   ? __libc_internal_tsd_get (_LIBC_TSD_KEY_##KEY) \
   : __libc_tsd_##KEY##_data)
#define __libc_tsd_set(KEY, VALUE) \
  (__libc_internal_tsd_set != NULL \
   ? __libc_internal_tsd_set (_LIBC_TSD_KEY_##KEY, (VALUE)) \
   : ((__libc_tsd_##KEY##_data = (VALUE)), 0))

#endif

#endif	/* bits/libc-tsd.h */
