#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks
 * struct kernel_stat should look like...  It turns out each arch has a
 * different opinion on the subject... */

#if __WORDSIZE == 64
#define kernel_stat kernel_stat64
#else
struct kernel_stat {
    unsigned short	st_dev;
    unsigned long	st_ino;
    unsigned short	st_mode;
    short		st_nlink;
    unsigned short	st_uid;
    unsigned short	st_gid;
    unsigned short	st_rdev;
    long		st_size;
    struct timespec	st_atim;
    struct timespec	st_mtim;
    struct timespec	st_ctim;
    long		st_blksize;
    long		st_blocks;
    unsigned long	__unused4[2];
};

#endif

struct kernel_stat64 {
	unsigned long long	st_dev;
	unsigned long long	st_ino;
	unsigned int	st_mode;
	unsigned int	st_nlink;
	unsigned int	st_uid;
	unsigned int	st_gid;
	unsigned long long	st_rdev;
	unsigned char	__pad3[8];
	long long	st_size;
	unsigned int	st_blksize;
	unsigned char	__pad4[8];
	unsigned int	st_blocks;
	struct timespec	st_atim;
	struct timespec	st_mtim;
	struct timespec	st_ctim;
	unsigned int	__unused4;
	unsigned int	__unused5;
};

#endif	/*  _BITS_STAT_STRUCT_H */

