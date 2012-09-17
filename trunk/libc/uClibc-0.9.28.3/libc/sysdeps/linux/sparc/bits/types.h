/* Copyright (C) 1991,92,1994-1999,2000,2001 Free Software Foundation, Inc.
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

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

#ifndef	_BITS_TYPES_H
#define	_BITS_TYPES_H	1

#include <features.h>

#define __need_size_t
#include <stddef.h>
#include <bits/wordsize.h>
#include <bits/kernel_types.h>

/* Convenience types.  */
typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;
#if __WORDSIZE == 64
typedef unsigned long int __u_quad_t;
typedef long int __quad_t;
#else
# ifdef __GNUC__
__extension__ typedef unsigned long long int __u_quad_t;
__extension__ typedef long long int __quad_t;
# else
typedef struct
  {
    long int __val[2];
  } __quad_t;
typedef struct
  {
    __u_long __val[2];
  } __u_quad_t;
# endif
#endif
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
#if __WORDSIZE == 64
typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;
#else
# ifdef __GNUC__
__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;
# endif
#endif
typedef __quad_t *__qaddr_t;

typedef __u_quad_t __dev_t;		/* Type of device numbers.  */
typedef __u_int __uid_t;		/* Type of user identifications.  */
typedef __u_int __gid_t;		/* Type of group identifications.  */
typedef __u_long __ino_t;		/* Type of file serial numbers.  */
typedef __u_quad_t __ino64_t;		/* Type of file serial numbers.  */
typedef __u_int __mode_t;		/* Type of file attribute bitmasks.  */
typedef __u_int __nlink_t; 		/* Type of file link counts.  */
typedef long int __off_t;		/* Type of file sizes and offsets.  */
typedef __quad_t  __off64_t;		/*  "" (LFS) */
typedef __quad_t __loff_t;		/* Type of file sizes and offsets.  */
typedef int __pid_t;			/* Type of process identifications.  */
#if __WORDSIZE == 64
typedef long int __ssize_t;		/* Type of a byte count, or error.  */
#else
typedef int __ssize_t;			/* Type of a byte count, or error.  */
#endif
typedef __u_long __rlim_t;		/* Type of resource counts.  */
typedef __u_quad_t __rlim64_t;		/* Type of resource counts (LFS).  */
typedef __u_int __id_t;			/* General type for IDs.  */

typedef struct
  {
    int __val[2];
  } __fsid_t;				/* Type of file system IDs.  */

/* Everythin' else.  */
typedef int __daddr_t;			/* The type of a disk address.  */
typedef char *__caddr_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef int __suseconds_t;
typedef long int __swblk_t;		/* Type of a swap block maybe?  */

typedef long int __clock_t;

/* Clock ID used in clock and timer functions.  */
typedef int __clockid_t;

/* Timer ID returned by `timer_create'.  */
typedef int __timer_t;

/* Number of descriptors that can fit in an `fd_set'.  */
#define __FD_SETSIZE	1024


typedef int __key_t;

/* Used in `struct shmid_ds'.  */
#if __WORDSIZE == 64
typedef int		   __ipc_pid_t;
#else
typedef unsigned short int __ipc_pid_t;
#endif


/* Type to represent block size.  */
typedef long int __blksize_t;

/* Types from the Large File Support interface.  */

/* Type to count number os disk blocks.  */
typedef long int __blkcnt_t;
typedef __quad_t __blkcnt64_t;

/* Type to count file system blocks.  */
typedef __u_long     __fsblkcnt_t;
typedef __u_quad_t __fsblkcnt64_t;

/* Type to count file system inodes.  */
typedef unsigned long int __fsfilcnt_t;
typedef __u_quad_t __fsfilcnt64_t;

/* Used in XTI.  */
typedef long int __t_scalar_t;
typedef unsigned long int __t_uscalar_t;

/* Duplicates info from stdint.h but this is used in unistd.h.  */
#if __WORDSIZE == 64
typedef long int __intptr_t;
#else
typedef int      __intptr_t;
#endif

/* Duplicate info from sys/socket.h.  */
typedef unsigned int __socklen_t;


/* Now add the thread types.  */
#if defined __UCLIBC_HAS_THREADS__ && (defined __USE_POSIX199506 || defined __USE_UNIX98)
# include <bits/pthreadtypes.h>
#endif

#endif /* bits/types.h */
