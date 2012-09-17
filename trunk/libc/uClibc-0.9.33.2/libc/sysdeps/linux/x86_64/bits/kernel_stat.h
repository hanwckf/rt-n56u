/* Ripped from linux/include/asm-x86_64/stat.h
 * and renamed 'struct stat' to 'struct kernel_stat' */

#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

struct kernel_stat {
	unsigned long  st_dev;
	unsigned long  st_ino;
	unsigned long  st_nlink;

	unsigned int   st_mode;
	unsigned int   st_uid;
	unsigned int   st_gid;
	unsigned int   __pad0;
	unsigned long  st_rdev;
	long           st_size;
	long           st_blksize;
	long           st_blocks;    /* Number 512-byte blocks allocated. */

	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
	long           __unused[3];
};

/* x86-64 stat64 is same as stat */
#define kernel_stat64 kernel_stat

#endif
