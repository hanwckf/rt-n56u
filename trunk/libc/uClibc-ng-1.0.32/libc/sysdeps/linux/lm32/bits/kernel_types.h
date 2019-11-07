#ifndef __ASM_GENERIC_POSIX_TYPES_H
#define __ASM_GENERIC_POSIX_TYPES_H

typedef unsigned long   __kernel_ino_t;
typedef unsigned int    __kernel_mode_t;
typedef unsigned long   __kernel_nlink_t;
typedef long            __kernel_off_t;
typedef int             __kernel_pid_t;
typedef int             __kernel_ipc_pid_t;
typedef unsigned int    __kernel_uid_t;
typedef unsigned int    __kernel_gid_t;
typedef unsigned int    __kernel_size_t;
typedef int             __kernel_ssize_t;
typedef int             __kernel_ptrdiff_t;
typedef long            __kernel_time_t;
typedef long            __kernel_suseconds_t;
typedef long            __kernel_clock_t;
typedef int             __kernel_timer_t;
typedef int             __kernel_clockid_t;
typedef int             __kernel_daddr_t;
typedef char *          __kernel_caddr_t;
typedef unsigned short  __kernel_uid16_t;
typedef unsigned short  __kernel_gid16_t;
typedef __kernel_uid_t  __kernel_uid32_t;
typedef __kernel_gid_t  __kernel_gid32_t;

typedef __kernel_uid_t  __kernel_old_uid_t;
typedef __kernel_gid_t  __kernel_old_gid_t;
typedef unsigned int    __kernel_old_dev_t;

typedef long long       __kernel_loff_t;
typedef long            __kernel_long_t;
typedef unsigned long   __kernel_ulong_t;

typedef struct {
        int val[2];
} __kernel_fsid_t;

#endif /* __ASM_GENERIC_POSIX_TYPES_H */
