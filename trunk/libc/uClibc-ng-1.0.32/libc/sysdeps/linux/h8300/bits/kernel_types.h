#ifndef __ASM_GENERIC_POSIX_TYPES_H
#define __ASM_GENERIC_POSIX_TYPES_H

/* Sigh.  We need to carefully wrap this one...  No guarantees
 * that the asm/posix_types.h kernel header is working.  Many
 * arches have broken headers that introduce tons of gratuitous
 * conflicts with uClibc's namespace.   See bits/kernel_types.h
 * for i386, arm, etc for examples... */

typedef unsigned short	__kernel_dev_t;
typedef unsigned long	__kernel_ino_t;
typedef unsigned short	__kernel_mode_t;
typedef unsigned short	__kernel_nlink_t;
typedef long		__kernel_off_t;
typedef int		__kernel_pid_t;
typedef unsigned short	__kernel_ipc_pid_t;
typedef unsigned short	__kernel_uid_t;
typedef unsigned short	__kernel_gid_t;
typedef unsigned long	__kernel_size_t;
typedef long		__kernel_ssize_t;
typedef long		__kernel_ptrdiff_t;
typedef long		__kernel_time_t;
typedef long		__kernel_suseconds_t;
typedef long		__kernel_clock_t;
typedef int		__kernel_daddr_t;
typedef char *		__kernel_caddr_t;
typedef unsigned short	__kernel_uid16_t;
typedef unsigned short	__kernel_gid16_t;
typedef unsigned int	__kernel_uid32_t;
typedef unsigned int	__kernel_gid32_t;
typedef unsigned short	__kernel_old_uid_t;
typedef unsigned short	__kernel_old_gid_t;
typedef long long	__kernel_loff_t;
typedef __kernel_dev_t	__kernel_old_dev_t;
typedef long		__kernel_long_t;
typedef unsigned long	__kernel_ulong_t;

#define __kernel_long_t		__kernel_long_t
#define __kernel_ino_t 		__kernel_ino_t
#define __kernel_mode_t 	__kernel_mode_t
#define __kernel_pid_t 		__kernel_pid_t
#define __kernel_ipc_pid_t 	__kernel_ipc_pid_t
#define __kernel_uid_t 		__kernel_uid_t
#define __kernel_susecond_t 	__kernel_susecond_t
#define __kernel_daddr_t 	__kernel_daddr_t
#define __kernel_uid32_t 	__kernel_uid32_t
#define __kernel_old_uid_t 	__kernel_old_uid_t
#define __kernel_old_dev_t 	__kernel_old_dev_t

typedef struct {
#ifdef __USE_ALL
	int val[2];
#else
	int __val[2];
#endif
} __kernel_fsid_t;
#define __kernel_fsid_t __kernel_fsid_t

#endif /* __ASM_GENERIC_POSIX_TYPES_H */
