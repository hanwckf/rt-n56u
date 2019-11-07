/* Copyright (C) 1991,92,96,97,99,2000,2001,2009 Free Software Foundation, Inc.
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

#ifndef	_STRINGS_H
#define	_STRINGS_H	1

/* We don't need and should not read this file if <string.h> was already
   read. The one exception being that if __USE_BSD isn't defined, then
   these aren't defined in string.h, so we need to define them here.  */

/* keep this file in sync w/ string.h, the glibc version is out of date */

#if !defined _STRING_H || !defined __USE_BSD

# include <features.h>
# define __need_size_t
# include <stddef.h>

__BEGIN_DECLS

# ifdef __UCLIBC_SUSV3_LEGACY__
/* Copy N bytes of SRC to DEST (like memmove, but args reversed).  */
extern void bcopy (const void *__src, void *__dest, size_t __n)
     __THROW __nonnull ((1, 2));

/* Set N bytes of S to 0.  */
extern void bzero (void *__s, size_t __n) __THROW __nonnull ((1));

/* Compare N bytes of S1 and S2 (same as memcmp).  */
extern int bcmp (const void *__s1, const void *__s2, size_t __n)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Find the first occurrence of C in S (same as strchr).  */
extern char *index (const char *__s, int __c)
     __THROW __attribute_pure__ __nonnull ((1));

/* Find the last occurrence of C in S (same as strrchr).  */
extern char *rindex (const char *__s, int __c)
     __THROW __attribute_pure__ __nonnull ((1));
# else
#  ifdef __UCLIBC_SUSV3_LEGACY_MACROS__
/* bcopy/bzero/bcmp/index/rindex are marked LEGACY in SuSv3.
 * They are replaced as proposed by SuSv3. Don't sync this part
 * with glibc and keep it in sync with string.h.  */

#  define bcopy(src,dest,n) (memmove((dest), (src), (n)), (void) 0)
#  define bzero(s,n) (memset((s), '\0', (n)), (void) 0)
#  define bcmp(s1,s2,n) memcmp((s1), (s2), (size_t)(n))
#  define index(s,c) strchr((s), (c))
#  define rindex(s,c) strrchr((s), (c))
#  endif
# endif

/* Return the position of the first bit set in I, or 0 if none are set.
   The least-significant bit is position 1, the most-significant 32.  */
extern int ffs (int __i) __THROW __attribute__ ((__const__));
libc_hidden_proto(ffs)

/* The following two functions are non-standard but necessary for non-32 bit
   platforms.  */
# ifdef	__USE_GNU
extern int ffsl (long int __l) __THROW __attribute__ ((__const__));
#  ifdef __GNUC__
__extension__ extern int ffsll (long long int __ll)
     __THROW __attribute__ ((__const__));
#  endif
# endif

/* Compare S1 and S2, ignoring case.  */
extern int strcasecmp (const char *__s1, const char *__s2)
     __THROW __attribute_pure__ __nonnull ((1, 2));
libc_hidden_proto(strcasecmp)

/* Compare no more than N chars of S1 and S2, ignoring case.  */
extern int strncasecmp (const char *__s1, const char *__s2, size_t __n)
     __THROW __attribute_pure__ __nonnull ((1, 2));
libc_hidden_proto(strncasecmp)

#if defined __USE_XOPEN2K8 && defined __UCLIBC_HAS_XLOCALE__
/* The following functions are equivalent to the both above but they
   take the locale they use for the collation as an extra argument.
   This is not standardsized but something like will come.  */
# include <xlocale.h>

/* Again versions of a few functions which use the given locale instead
   of the global one.  */
extern int strcasecmp_l (const char *__s1, const char *__s2,
			 __locale_t __loc)
     __THROW __attribute_pure__ __nonnull ((1, 2, 3));
libc_hidden_proto(strcasecmp_l)

extern int strncasecmp_l (const char *__s1, const char *__s2,
			  size_t __n, __locale_t __loc)
     __THROW __attribute_pure__ __nonnull ((1, 2, 4));
libc_hidden_proto(strncasecmp_l)
#endif
__END_DECLS


#ifdef _LIBC
/* comment is wrong and will face this, when HAS_GNU option will be added
 * header is SuSv standard */
#error "<strings.h> should not be included from libc."
#endif


#endif	/* string.h  */

#endif	/* strings.h  */
