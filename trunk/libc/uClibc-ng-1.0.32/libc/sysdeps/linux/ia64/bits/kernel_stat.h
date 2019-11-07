/* Ripped from linux/include/asm-ia64/stat.h
 * and renamed 'struct stat' to 'struct kernel_stat' */

#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/*
 * Modified 1998, 1999
 *	David Mosberger-Tang <davidm@hpl.hp.com>, Hewlett-Packard Co
 */

struct kernel_stat {
	unsigned long	st_dev;
	unsigned long	st_ino;
	unsigned long	st_nlink;
	unsigned int	st_mode;
	unsigned int	st_uid;
	unsigned int	st_gid;
	unsigned int	__pad0;
	unsigned long	st_rdev;
	unsigned long	st_size;
	struct timespec	st_atim;
	struct timespec	st_mtim;
	struct timespec	st_ctim;
	unsigned long	st_blksize;
	long		st_blocks;
	unsigned long	__unused[3];
};

/* ia64 stat64 is same as stat */
#define kernel_stat64 kernel_stat

#endif /* _BITS_STAT_STRUCT_H */
