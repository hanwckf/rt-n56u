/* Note that we use the exact same include guard #define names
 * as asm/posix_types.h.  This will avoid gratuitous conflicts
 * with the posix_types.h kernel header, and will ensure that
 * our private content, and not the kernel header, will win.
 *  -Erik
 */
#ifndef _XTENSA_POSIX_TYPES_H
#define _XTENSA_POSIX_TYPES_H

typedef unsigned long	__kernel_ino_t;
typedef unsigned int	__kernel_mode_t;
typedef unsigned long	__kernel_nlink_t;
typedef long		__kernel_off_t;
typedef int		__kernel_pid_t;
typedef unsigned short	__kernel_ipc_pid_t;
typedef unsigned int	__kernel_uid_t;
typedef unsigned int	__kernel_gid_t;
typedef unsigned int	__kernel_size_t;
typedef int		__kernel_ssize_t;
typedef long		__kernel_ptrdiff_t;
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

typedef unsigned short	__kernel_old_uid_t;
typedef unsigned short	__kernel_old_gid_t;
typedef unsigned short	__kernel_old_dev_t;
typedef long long	__kernel_loff_t;

/* Beginning in 2.6 kernels, which is the first version that includes the
   Xtensa port, __kernel_dev_t is defined in "linux/types.h" and is no longer
   architecture-specific.  It is defined here in uClibc for consistency with
   other uClibc ports and for lack of a better place.  */
typedef unsigned int	__kernel_dev_t;

typedef struct {
	int	val[2];
} __kernel_fsid_t;

#endif /* _XTENSA_POSIX_TYPES_H */
