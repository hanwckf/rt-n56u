/* Copyright (C) 1991,92,96,97,99,2000,2001 Free Software Foundation, Inc.
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

#ifndef	_STRINGS_H
#define	_STRINGS_H	1

/* We don't need and should not read this file if <string.h> was already
   read. The one exception being that if __USE_BSD isn't defined, then
   these aren't defined in string.h, so we need to define them here.  */
#if !defined _STRING_H || !defined __USE_BSD

# include <features.h>
# define __need_size_t
# include <stddef.h>

__BEGIN_DECLS

/* Compare N bytes of S1 and S2 (same as memcmp).  */
extern int bcmp (__const void *__s1, __const void *__s2, size_t __n)
     __THROW __attribute_pure__;

/* Copy N bytes of SRC to DEST (like memmove, but args reversed).  */
extern void bcopy (__const void *__src, void *__dest, size_t __n) __THROW;

/* Set N bytes of S to 0.  */
extern void bzero (void *__s, size_t __n) __THROW;

/* Return the position of the first bit set in I, or 0 if none are set.
   The least-significant bit is position 1, the most-significant 32.  */
extern int ffs (int __i) __THROW __attribute__ ((const));

/* Find the first occurrence of C in S (same as strchr).  */
extern char *index (__const char *__s, int __c) __THROW __attribute_pure__;

/* Find the last occurrence of C in S (same as strrchr).  */
extern char *rindex (__const char *__s, int __c) __THROW __attribute_pure__;

/* Compare S1 and S2, ignoring case.  */
extern int strcasecmp (__const char *__s1, __const char *__s2)
     __THROW __attribute_pure__;

/* Compare no more than N chars of S1 and S2, ignoring case.  */
extern int strncasecmp (__const char *__s1, __const char *__s2, size_t __n)
     __THROW __attribute_pure__;

__END_DECLS

#endif	/* string.h  */

#endif	/* strings.h  */
