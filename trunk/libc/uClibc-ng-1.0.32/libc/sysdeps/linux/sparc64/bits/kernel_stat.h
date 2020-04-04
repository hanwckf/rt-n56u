#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks
 * struct kernel_stat should look like...  It turns out each arch has a
 * different opinion on the subject... */

struct kernel_stat {
    unsigned int	st_dev;
    unsigned long	st_ino;
    unsigned short	st_mode;
    short		st_nlink;
    unsigned short	st_uid;
    unsigned short	st_gid;
    unsigned int	st_rdev;
    long		st_size;
    struct timespec	st_atim;
    struct timespec	st_mtim;
    struct timespec	st_ctim;
    long		st_blksize;
    long		st_blocks;
    unsigned long 	__unused4;
    unsigned long 	__unused5;
};

struct kernel_stat64 {
	unsigned long 	st_dev;
	unsigned long   st_ino;
	unsigned long	st_nlink;
	unsigned int	st_mode;
	unsigned int	st_uid;
	unsigned int	st_gid;
    	unsigned int 	__pad2;
	unsigned long	st_rdev;
	long 	st_size;
	long 	st_blksize;
	long 	st_blocks;
	struct timespec	st_atim;
	struct timespec	st_mtim;
	struct timespec	st_ctim;
	long	__unused4;
	long 	__unused5;
	long 	__unused6;
};

#endif	/*  _BITS_STAT_STRUCT_H */
