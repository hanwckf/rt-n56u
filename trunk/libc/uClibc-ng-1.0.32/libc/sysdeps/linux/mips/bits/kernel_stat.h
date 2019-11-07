#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks
 * struct kernel_stat should look like...  It turns out each arch has a
 * different opinion on the subject... */

#include <sgidefs.h>

#if _MIPS_SIM == _MIPS_SIM_ABI64
typedef struct {
	unsigned int tv_sec;
	unsigned int tv_nsec;
} __ktimespec_t;
#else
typedef struct {
	time_t tv_sec;
	unsigned long tv_nsec;
} __ktimespec_t;
#endif

#if _MIPS_SIM == _MIPS_SIM_ABI64
/* The memory layout is the same as of struct stat64 of the 32-bit kernel.  */
struct kernel_stat {
	__kernel_dev_t	st_dev;
	unsigned int	st_pad1[3];
	__kernel_ino_t	st_ino;
	__kernel_mode_t	st_mode;
	__kernel_nlink_t st_nlink;
	__kernel_uid_t	st_uid;
	__kernel_gid_t	st_gid;
	__kernel_dev_t	st_rdev;
	unsigned int	st_pad2[3];
	__kernel_off_t	st_size;
	__ktimespec_t	st_atim;
	__ktimespec_t	st_mtim;
	__ktimespec_t	st_ctim;
	unsigned int	st_blksize;
	unsigned int	reserved3;
	unsigned long	st_blocks;
};
#define kernel_stat64 kernel_stat
#elif _MIPS_SIM == _MIPS_SIM_NABI32
/* The memory layout is the same as of struct stat64 of the 32-bit kernel.  */
struct kernel_stat {
	unsigned int	st_dev;
	unsigned int	st_pad1[3];
	unsigned long long	st_ino;
	__kernel_mode_t	st_mode;
	__kernel_nlink_t st_nlink;
	__kernel_uid_t	st_uid;
	__kernel_gid_t	st_gid;
	unsigned int	st_rdev;
	unsigned int	st_pad2[3];
	unsigned long long	st_size;
	__ktimespec_t	st_atim;
	__ktimespec_t	st_mtim;
	__ktimespec_t	st_ctim;
	unsigned int	st_blksize;
	unsigned int	reserved3;
	unsigned long long	st_blocks;
};
#define kernel_stat64 kernel_stat
#else /* O32 */
struct kernel_stat {
	__kernel_dev_t	st_dev;
	long		st_pad1[3];
	__kernel_ino_t	st_ino;
	__kernel_mode_t	st_mode;
	__kernel_nlink_t st_nlink;
	__kernel_uid_t	st_uid;
	__kernel_gid_t	st_gid;
	__kernel_dev_t	st_rdev;
	long		st_pad2[2];
	__kernel_off_t	st_size;
	long		st_pad3;
	__ktimespec_t	st_atim;
	__ktimespec_t	st_mtim;
	__ktimespec_t	st_ctim;
	long		st_blksize;
	long		st_blocks;
	long		st_pad4[14];
};

struct kernel_stat64 {
	unsigned long	st_dev;
	unsigned long	st_pad0[3];	/* Reserved for st_dev expansion  */
	unsigned long long	st_ino;
	__kernel_mode_t	st_mode;
	__kernel_nlink_t st_nlink;
	__kernel_uid_t	st_uid;
	__kernel_gid_t	st_gid;
	unsigned long	st_rdev;
	unsigned long	st_pad1[3];	/* Reserved for st_rdev expansion  */
	long long	st_size;
	__ktimespec_t	st_atim;
	__ktimespec_t	st_mtim;
	__ktimespec_t	st_ctim;
	unsigned long	st_blksize;
	unsigned long	st_pad2;
	long long	st_blocks;
};
#endif	/* O32 */

#endif	/*  _BITS_STAT_STRUCT_H */

