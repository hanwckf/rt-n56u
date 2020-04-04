#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks
 * struct kernel_stat should look like...  It turns out each arch has a
 * different opinion on the subject... */

#if __WORDSIZE == 64
#define kernel_stat kernel_stat64
#else
struct kernel_stat {
	__kernel_dev_t	st_dev;
	__kernel_ino_t	st_ino;
	__kernel_mode_t	st_mode;
	__kernel_nlink_t st_nlink;
	__kernel_uid_t	st_uid;
	__kernel_gid_t 	st_gid;
	__kernel_dev_t	st_rdev;
	__kernel_off_t	st_size;
	unsigned long  	st_blksize;
	unsigned long  	st_blocks;
	struct timespec	st_atim;
	struct timespec	st_mtim;
	struct timespec	st_ctim;
	unsigned long  	__unused4;
	unsigned long  	__unused5;
};
#endif

struct kernel_stat64 {
	unsigned long long st_dev; 	/* Device.  */
	unsigned long long st_ino;	/* File serial number.  */
	unsigned int st_mode;		/* File mode.  */
	unsigned int st_nlink;		/* Link count.  */
	unsigned int st_uid;		/* User ID of the file's owner.  */
	unsigned int st_gid;		/* Group ID of the file's group. */
	unsigned long long st_rdev; 	/* Device number, if device.  */
	unsigned short int __pad2;
	long long st_size;		/* Size of file, in bytes.  */
	long st_blksize;		/* Optimal block size for I/O.  */
	long long st_blocks;		/* Number 512-byte blocks allocated. */
	struct timespec st_atim;	/* Time of last access.  */
	struct timespec st_mtim;	/* Time of last modification.  */
	struct timespec st_ctim;	/* Time of last status change.  */
	unsigned long int __unused4;
	unsigned long int __unused5;
};

#endif	/*  _BITS_STAT_STRUCT_H */

