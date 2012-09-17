#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks
 * struct kernel_stat should look like...  It turns out each arch has a
 * different opinion on the subject... */
#include <endian.h>

struct kernel_stat {
	unsigned short st_dev;
	unsigned short __pad1;
	unsigned long st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned short st_rdev;
	unsigned short __pad2;
	unsigned long  st_size;
	unsigned long  st_blksize;
	unsigned long  st_blocks;
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
	unsigned long  __unused4;
	unsigned long  __unused5;
};

struct kernel_stat64 {
#if (__BYTE_ORDER == __BIG_ENDIAN)
	unsigned char   __pad0b[6];
	unsigned short	st_dev;
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
	unsigned short	st_dev;
	unsigned char	__pad0b[6];
#else
#error Must know endian to build stat64 structure!
#endif
	unsigned char	__pad0[4];

	unsigned long	st_ino;
	unsigned int	st_mode;
	unsigned int	st_nlink;

	unsigned long	st_uid;
	unsigned long	st_gid;

#if (__BYTE_ORDER == __BIG_ENDIAN)
	unsigned char	__pad3b[6];
	unsigned short	st_rdev;
#else /* Must be little */
	unsigned short	st_rdev;
	unsigned char	__pad3b[6];
#endif
	unsigned char	__pad3[4];

	long long	st_size;
	unsigned long	st_blksize;

#if (__BYTE_ORDER == __BIG_ENDIAN)
	unsigned long	__pad4;		/* Future possible st_blocks hi bits */
	unsigned long	st_blocks;	/* Number 512-byte blocks allocated. */
#else /* Must be little */
	unsigned long	st_blocks;	/* Number 512-byte blocks allocated. */
	unsigned long	__pad4;		/* Future possible st_blocks hi bits */
#endif

	struct timespec	st_atim;
	struct timespec	st_mtim;
	struct timespec	st_ctim;

	unsigned long	__unused1;
	unsigned long	__unused2;
};

#endif	/*  _BITS_STAT_STRUCT_H */

