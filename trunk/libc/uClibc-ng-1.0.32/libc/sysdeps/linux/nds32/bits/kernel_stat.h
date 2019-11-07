/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

struct kernel_stat {
#if defined(__NDS32_EB__)
	unsigned short st_dev;
	unsigned short __pad1;
#else
	unsigned long  st_dev;
#endif
	unsigned long  st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
#if defined(__NDS32_EB__)
	unsigned short st_rdev;
	unsigned short __pad2;
#else
	unsigned long  st_rdev;
#endif
	unsigned long  st_size;
	unsigned long  st_blksize;
	unsigned long  st_blocks;
	struct timespec	st_atim;
	struct timespec	st_mtim;
	struct timespec	st_ctim;
	unsigned long  __unused4;
	unsigned long  __unused5;
};

struct kernel_stat64 {
	unsigned long long	st_dev;
	unsigned long    __pad0;
#define STAT64_HAS_BROKEN_ST_INO	1
	unsigned long	__st_ino;
	unsigned int	st_mode;
	unsigned int	st_nlink;
	unsigned long	st_uid;
	unsigned long	st_gid;
	unsigned long long	st_rdev;
	unsigned int    __pad3;
	unsigned long long	st_size;
	unsigned long	st_blksize;
	unsigned long long  st_blocks;	// Number 512-byte blocks allocated. 
	struct timespec st_atim;	/* Time of last access. */
	struct timespec st_mtim;	/* Time of last modification. */
	struct timespec st_ctim;	/* Time of last status change. */
	unsigned long long	st_ino;
};

#endif	/*  _BITS_STAT_STRUCT_H */
