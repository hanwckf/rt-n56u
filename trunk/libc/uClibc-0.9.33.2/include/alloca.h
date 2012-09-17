/* Copyright (C) 1992, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef	_ALLOCA_H
#define	_ALLOCA_H	1

#include <features.h>

#define	__need_size_t
#include <stddef.h>

__BEGIN_DECLS

/* Remove any previous definitions.  */
#undef	alloca

/* Allocate a block that will be freed when the calling function exits.  */
extern void *alloca (size_t __size) __THROW;

#ifdef	__GNUC__
# define alloca(size)	__builtin_alloca (size)
#endif /* GCC.  */

#ifdef _LIBC
# define __MAX_ALLOCA_CUTOFF	65536

# include <bits/stackinfo.h>
# ifdef _STACK_GROWS_DOWN
#  define extend_alloca(buf, len, newlen) \
   (__typeof (buf)) ({ size_t __newlen = (newlen);			      \
		      char *__newbuf = alloca (__newlen);		      \
		      if (__newbuf + __newlen == (char *) buf)		      \
			len += __newlen;				      \
		      else						      \
			len = __newlen;					      \
		      __newbuf; })
# elif defined _STACK_GROWS_UP
#  define extend_alloca(buf, len, newlen) \
   (__typeof (buf)) ({ size_t __newlen = (newlen);			      \
		      char *__newbuf = alloca (__newlen);		      \
		      char *__buf = (buf);				      \
		      if (__buf + __newlen == __newbuf)			      \
			{						      \
			  len += __newlen;				      \
			  __newbuf = __buf;				      \
			}						      \
		      else						      \
			len = __newlen;					      \
		      __newbuf; })
# else
#  error unknown stack
#  define extend_alloca(buf, len, newlen) \
   alloca (((len) = (newlen)))
# endif

extern int __libc_alloca_cutoff (size_t size);
#endif

__END_DECLS

#endif /* alloca.h */
