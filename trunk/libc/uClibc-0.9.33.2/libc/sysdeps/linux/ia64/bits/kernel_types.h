/* Note that we use the exact same include guard #define names
 * as asm/posix_types.h.  This will avoid gratuitous conflicts
 * with the posix_types.h kernel header, and will ensure that
 * our private content, and not the kernel header, will win.
 *  -Erik
 */
#ifndef _ASM_IA64_POSIX_TYPES_H
#define _ASM_IA64_POSIX_TYPES_H

/*
 * This file is generally used by user-level software, so you need to
 * be a little careful about namespace pollution etc.  Also, we cannot
 * assume GCC is being used.
 *
 * Based on <asm-alpha/posix_types.h>.
 *
 * Modified 1998-2000, 2003
 *	David Mosberger-Tang <davidm@hpl.hp.com>, Hewlett-Packard Co
 */

typedef unsigned long	__kernel_ino_t;
typedef unsigned int	__kernel_mode_t;
typedef unsigned int	__kernel_nlink_t;
typedef long		__kernel_off_t;
typedef long long	__kernel_loff_t;
typedef int		__kernel_pid_t;
typedef int		__kernel_ipc_pid_t;
typedef unsigned int	__kernel_uid_t;
typedef unsigned int	__kernel_gid_t;
typedef unsigned long	__kernel_size_t;
typedef long		__kernel_ssize_t;
typedef long		__kernel_ptrdiff_t;
typedef long		__kernel_time_t;
typedef long		__kernel_suseconds_t;
typedef long		__kernel_clock_t;
typedef int		__kernel_timer_t;
typedef int		__kernel_clockid_t;
typedef int		__kernel_daddr_t;
typedef char *		__kernel_caddr_t;
typedef unsigned long	__kernel_sigset_t;	/* at least 32 bits */
typedef unsigned short	__kernel_uid16_t;
typedef unsigned short	__kernel_gid16_t;

typedef struct {
	int	val[2];
} __kernel_fsid_t;

typedef __kernel_uid_t __kernel_old_uid_t;
typedef __kernel_gid_t __kernel_old_gid_t;
typedef __kernel_uid_t __kernel_uid32_t;
typedef __kernel_gid_t __kernel_gid32_t;

typedef unsigned int	__kernel_dev_t;
typedef unsigned int	__kernel_old_dev_t;

#endif /* _ASM_IA64_POSIX_TYPES_H */
