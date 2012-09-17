#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks 
 * struct kernel_stat should look like...  It turns out each arch has a 
 * different opinion on the subject... */

struct kernel_stat {
	unsigned char __pad1[6];
	unsigned short st_dev;

	unsigned long __pad2;
	unsigned long st_ino;

	unsigned short __pad3;
	unsigned short st_mode;
	unsigned short __pad4;
	unsigned short st_nlink;

	unsigned short __pad5;
	unsigned short st_uid;
	unsigned short __pad6;
	unsigned short st_gid;

	unsigned char __pad7[6];
	unsigned short st_rdev;

	unsigned long __pad8;
	unsigned long st_size;

	unsigned long __pad9;		/* align 64-bit st_blocks to 2-word */
	unsigned long st_blksize;

	unsigned long __pad10;	/* future possible st_blocks high bits */
	unsigned long st_blocks;	/* Number 512-byte blocks allocated. */

	unsigned long __unused1;
	unsigned long st_atime;

	unsigned long __unused2;
	unsigned long st_mtime;

	unsigned long __unused3;
	unsigned long st_ctime;

	unsigned long long __unused4;
};

struct kernel_stat64 {
	unsigned char __pad1[6];
	unsigned short st_dev;

	unsigned long long st_ino;

	unsigned int st_mode;
	unsigned int st_nlink;

	unsigned long st_uid;
	unsigned long st_gid;

	unsigned char __pad2[6];
	unsigned short st_rdev;

	long long st_size;

	unsigned long __pad3;		/* align 64-bit st_blocks to 2-word */
	unsigned long st_blksize;

	unsigned long __pad4;		/* future possible st_blocks high bits */
	unsigned long st_blocks;	/* Number 512-byte blocks allocated. */

	unsigned long __unused1;
	unsigned long st_atime;

	unsigned long __unused2;
	unsigned long st_mtime;

	unsigned long __unused3;	/* will be high 32 bits of ctime someday */
	unsigned long st_ctime;

	unsigned long long __unused4;
};

#endif	/*  _BITS_STAT_STRUCT_H */
