/*
 * sysdeps/linux/microblaze/bits/kernel_types.h -- Kernel versions of standard types
 *
 *  Copyright (C) 2003       John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#ifndef _ASM_MICROBLAZE_POSIX_TYPES_H
#define _ASM_MICROBLAZE_POSIX_TYPES_H

typedef unsigned long	__kernel_dev_t;
typedef unsigned long	__kernel_ino_t;
//typedef unsigned long long __kernel_ino64_t;
typedef unsigned short	__kernel_mode_t;
typedef unsigned long	__kernel_nlink_t;
typedef long		__kernel_off_t;
typedef int		__kernel_pid_t;
typedef int		__kernel_ipc_pid_t;
typedef unsigned int	__kernel_uid_t;
typedef unsigned int	__kernel_gid_t;
typedef unsigned int	__kernel_size_t;
typedef int		__kernel_ssize_t;
typedef int		__kernel_ptrdiff_t;
typedef long		__kernel_time_t;
typedef long		__kernel_suseconds_t;
typedef long		__kernel_clock_t;
typedef int		__kernel_timer_t;
typedef int		__kernel_clockid_t;
typedef int		__kernel_daddr_t;
typedef char *		__kernel_caddr_t;
typedef unsigned short	__kernel_uid16_t;
typedef unsigned short	__kernel_gid16_t;
typedef unsigned int	__kernel_uid32_t;
typedef unsigned int	__kernel_gid32_t;

typedef unsigned int	__kernel_old_uid_t;
typedef unsigned int	__kernel_old_gid_t;
typedef unsigned int	__kernel_old_dev_t;

#ifdef __GNUC__
typedef long long	__kernel_loff_t;
#endif

typedef struct {
#ifdef __USE_ALL
	int val[2];
#else
	int __val[2];
#endif
} __kernel_fsid_t;

#endif /* _ASM_MICROBLAZE_POSIX_TYPES_H */
