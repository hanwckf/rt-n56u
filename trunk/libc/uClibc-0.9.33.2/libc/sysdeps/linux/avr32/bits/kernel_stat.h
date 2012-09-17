#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/*
 * This file provides struct stat, taken from kernel 2.6.4. Verified
 * to match kernel 2.6.22.
 */

struct kernel_stat {
        unsigned long		st_dev;
        unsigned long		st_ino;
        unsigned short		st_mode;
        unsigned short		st_nlink;
        unsigned short		st_uid;
        unsigned short		st_gid;
        unsigned long		st_rdev;
        unsigned long		st_size;
        unsigned long		st_blksize;
        unsigned long		st_blocks;
        struct timespec		st_atim;
        struct timespec		st_mtim;
        struct timespec		st_ctim;
        unsigned long		__unused4;
        unsigned long		__unused5;
};

struct kernel_stat64 {
	unsigned long long	st_dev;

	unsigned long long	st_ino;
	unsigned int		st_mode;
	unsigned int		st_nlink;

	unsigned long		st_uid;
	unsigned long		st_gid;

	unsigned long long	st_rdev;

	long long		st_size;
	unsigned long		__pad1;
	unsigned long		st_blksize;

	unsigned long long	st_blocks;

	struct timespec		st_atim;
	struct timespec		st_mtim;
	struct timespec		st_ctim;

	unsigned long		__unused1;
	unsigned long		__unused2;
};

#endif /* _BITS_STAT_STRUCT_H */
