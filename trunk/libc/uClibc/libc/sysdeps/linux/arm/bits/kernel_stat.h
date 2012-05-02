#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks 
 * struct kernel_stat should look like...  It turns out each arch has a 
 * different opinion on the subject... */

#define STAT_HAVE_NSEC 1

struct kernel_stat {
#if defined(__ARMEB__)
	unsigned short st_dev;
	unsigned short __pad1;
#else
	unsigned long  st_dev;
#endif
	unsigned long st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
#if defined(__ARMEB__)
	unsigned short st_rdev;
	unsigned short __pad2;
#else
	unsigned long  st_rdev;
#endif
	unsigned long  st_size;
	unsigned long  st_blksize;
	unsigned long  st_blocks;
	unsigned long  st_atime;
	unsigned long  st_atime_nsec;
	unsigned long  st_mtime;
	unsigned long  st_mtime_nsec;
	unsigned long  st_ctime;
	unsigned long  st_ctime_nsec;
	unsigned long  __unused4;
	unsigned long  __unused5;
};

struct kernel_stat64 {
	unsigned long long st_dev;
	unsigned char      __pad0[4];

#define _HAVE_STAT64___ST_INO
	unsigned long      __st_ino;
	unsigned int       st_mode;
	unsigned int       st_nlink;
	unsigned long      st_uid;
	unsigned long      st_gid;

	unsigned long long st_rdev;
	unsigned char      __pad3[4];

	long long          st_size;
	unsigned long      st_blksize;
#if defined(__ARMEB__)
	unsigned long      __pad4;     /* future possible st_blocks high bits */
	unsigned long      st_blocks;  /* Number 512-byte blocks allocated. */
#else
	unsigned long      st_blocks;  /* Number 512-byte blocks allocated. */
	unsigned long      __pad4;     /* future possible st_blocks high bits */
#endif
	unsigned long      st_atime;
	unsigned long      st_atime_nsec;
	unsigned long      st_mtime;
	unsigned long      st_mtime_nsec;
	unsigned long      st_ctime;
	unsigned long      st_ctime_nsec;
	unsigned long long st_ino;
} __attribute__((packed));

#endif	/*  _BITS_STAT_STRUCT_H */
