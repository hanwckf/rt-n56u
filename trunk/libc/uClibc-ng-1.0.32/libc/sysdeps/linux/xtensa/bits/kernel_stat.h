#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks
 * struct kernel_stat should look like...  It turns out each arch has a
 * different opinion on the subject... */

struct kernel_stat {
	unsigned long	st_dev;
	unsigned long	st_ino;
	unsigned int	st_mode;
	unsigned int	st_nlink;
	unsigned int	st_uid;
	unsigned int	st_gid;
	unsigned long	st_rdev;
	long		st_size;
	unsigned long	st_blksize;
	unsigned long	st_blocks;
	struct timespec	st_atim;
	struct timespec	st_mtim;
	struct timespec	st_ctim;
	unsigned long	__unused4;
	unsigned long	__unused5;
};

struct kernel_stat64 {
	unsigned long long st_dev;	/* Device */
	unsigned long long st_ino;	/* File serial number */
	unsigned int  st_mode;		/* File mode. */
	unsigned int  st_nlink;		/* Link count. */
	unsigned int  st_uid;		/* User ID of the file's owner. */
	unsigned int  st_gid;		/* Group ID of the file's group. */
	unsigned long long st_rdev;	/* Device number, if device. */
	long long st_size;		/* Size of file, in bytes. */
	unsigned long st_blksize;	/* Optimal block size for I/O. */
	unsigned long __unused2;
	unsigned long long st_blocks;	/* Number 512-byte blocks allocated. */
	struct timespec st_atim;	/* Time of last access. */
	struct timespec st_mtim;	/* Time of last modification. */
	struct timespec st_ctim;	/* Time of last status change. */
	unsigned long __unused4;
	unsigned long __unused5;
};

#endif	/* _BITS_STAT_STRUCT_H */
