#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks 
 * struct kernel_stat should look like...  It turns out each arch has a 
 * different opinion on the subject... */
#warning "Please verify struct kernel_stat for your architecture actually matches struct kernel_stat for x86  If it doesn't, then you will need to add a proper kernel_stat.h for your architecture..."

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
	unsigned long  st_atime;
	unsigned long  __unused1;
	unsigned long  st_mtime;
	unsigned long  __unused2;
	unsigned long  st_ctime;
	unsigned long  __unused3;
	unsigned long  __unused4;
	unsigned long  __unused5;
};

struct kernel_stat64 {
	unsigned short	st_dev;
	unsigned char	__pad0[10];
#define _HAVE_STAT64___ST_INO
	unsigned long	__st_ino;
	unsigned int	st_mode;
	unsigned int	st_nlink;
	unsigned long	st_uid;
	unsigned long	st_gid;
	unsigned short	st_rdev;
	unsigned char	__pad3[10];
	long long	st_size;
	unsigned long	st_blksize;
	unsigned long	st_blocks;	/* Number 512-byte blocks allocated. */
	unsigned long	__pad4;		/* future possible st_blocks high bits */
	unsigned long	st_atime;
	unsigned long	__pad5;
	unsigned long	st_mtime;
	unsigned long	__pad6;
	unsigned long	st_ctime;
	unsigned long	__pad7;		/* will be high 32 bits of ctime someday */
	unsigned long long	st_ino;
};

#endif	/*  _BITS_STAT_STRUCT_H */

