/* include/platform_defs.h.  Generated automatically by configure.  */
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/SGIGPLNoticeExplan/
 *
 * @configure_input@
 */
#ifndef __XFS_PLATFORM_DEFS_H__
#define __XFS_PLATFORM_DEFS_H__

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <endian.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>

#if (__GLIBC__ <= 2) && (__GLIBC_MINOR__ <= 1)
# define constpp	const char * const *
#else
# define constpp	char * const *
#endif

typedef loff_t		xfs_off_t;
typedef uint64_t	xfs_ino_t;
typedef uint32_t	xfs_dev_t;
typedef int64_t         xfs_daddr_t;
typedef char*		xfs_caddr_t;

/* long and pointer must be either 32 bit or 64 bit */
/* #undef HAVE_64BIT_LONG */
#define HAVE_32BIT_LONG 1
#define HAVE_32BIT_PTR 1
/* #undef HAVE_64BIT_PTR */

/* Check if __psint_t is set to something meaningful */
/* #undef HAVE___PSINT_T */
#ifndef HAVE___PSINT_T
# ifdef HAVE_32BIT_PTR
typedef int __psint_t;
# elif defined HAVE_64BIT_PTR
#  ifdef HAVE_64BIT_LONG
typedef long __psint_t;
#  else
/* This is a very strange architecture, which has 64 bit pointers but
 * not 64 bit longs. So, I'd just punt here and assume long long is Ok */
typedef long long __psint_t;
#  endif
# else
#  error Unknown pointer size
# endif
#endif

/* Check if __psunsigned_t is set to something meaningful */
/* #undef HAVE___PSUNSIGNED_T */
#ifndef HAVE___PSUNSIGNED_T
# ifdef HAVE_32BIT_PTR
typedef unsigned int __psunsigned_t;
# elif defined HAVE_64BIT_PTR
#  ifdef HAVE_64BIT_LONG
typedef long __psunsigned_t;
#  else
/* This is a very strange architecture, which has 64 bit pointers but
 * not 64 bit longs. So, I'd just punt here and assume long long is Ok */
typedef unsigned long long __psunsigned_t;
#  endif
# else
#  error Unknown pointer size
# endif
#endif

#ifdef DEBUG
# define ASSERT		assert
#else
# define ASSERT(EX)	((void) 0)
#endif

#endif	/* __XFS_PLATFORM_DEFS_H__ */
