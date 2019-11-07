/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _SYS_STAT_H
# error "Never include <bits/stat.h> directly; use <sys/stat.h> instead."
#endif

#include <bits/align64bit.h>
#include <bits/wordsize.h>
#include <endian.h>

/* Versions of the `struct stat' data structure.  */
#define _STAT_VER_LINUX_OLD	1
#define _STAT_VER_KERNEL	1
#define _STAT_VER_SVR4		2
#define _STAT_VER_LINUX		3
#define _STAT_VER		_STAT_VER_LINUX	/* The one defined below.  */

/* Versions of the `xmknod' interface.  */
#define _MKNOD_VER_LINUX	1
#define _MKNOD_VER_SVR4		2
#define _MKNOD_VER		_MKNOD_VER_LINUX /* The bits defined below.  */

/*
 * For 32-bit architectures, this struct is similar to the stat64 but it
 * uses 32-bit members along with 32-bit padding. For 64-bit architectures
 * this struct is exactly the same with the stat64 one
 */
struct stat
  {
    unsigned long long st_dev;			/* Device. */
    unsigned long long st_ino;			/* 32bit file serial number. */
    unsigned int st_mode;			/* File mode.  */
    unsigned int st_nlink;			/* Link count.  */
    unsigned int st_uid;			/* User ID of the file's owner.	*/
    unsigned int st_gid;			/* Group ID of the file's group.*/
    unsigned long long st_rdev;			/* Device number, if device.  */
    unsigned long long _pad1;
#if __WORDSIZE == 64
    __off_t st_size;				/* SIze of file, in bytes. */
#else
    long long st_size;				/* SIze of file, in bytes. */
#endif
    int st_blksize;				/* Optimal block size for I/O. */
    int __pad2;
    long long st_blocks;			/* Number 512-byte blocks allocated */
#ifdef __USE_MISC
    /* Nanosecond resolution timestamps are stored in a format
       equivalent to 'struct timespec'.  This is the type used
       whenever possible but the Unix namespace rules do not allow the
       identifier 'timespec' to appear in the <sys/stat.h> header.
       Therefore we have to handle the use of this header in strictly
       standard-compliant sources special.  */
    struct timespec st_atim;			/* Time of last access.  */
    struct timespec st_mtim;			/* Time of last modification.  */
    struct timespec st_ctim;			/* Time of last status change.  */
# define st_atime st_atim.tv_sec		/* Backward compatibility.  */
# define st_mtime st_mtim.tv_sec
# define st_ctime st_ctim.tv_sec
#else
    int st_atime;				/* Time of last access. */
    unsigned int st_atime_nsec;
    int st_mtime;				/* Time of last modification. */
    unsigned int st_mtime_nsec;
    int st_ctime;				/* Time of last status change. */
    unsigned int st_ctime_nsec;
#endif
    unsigned int __unused4;
    unsigned int __unused5;
  } __ARCH_64BIT_ALIGNMENT__;


#ifdef __USE_LARGEFILE64
struct stat64
  {
    unsigned long long st_dev;			/* Device. */
    unsigned long long st_ino;			/* 32bit file serial number. */
    unsigned int st_mode;			/* File mode.  */
    unsigned int st_nlink;			/* Link count.  */
    unsigned int st_uid;			/* User ID of the file's owner.	*/
    unsigned int st_gid;			/* Group ID of the file's group.*/
    unsigned long long st_rdev;			/* Device number, if device.  */
    unsigned long long __pad3;
    long long st_size;				/* Size of file, in bytes. */
    int st_blksize;				/* Optimal block size for I/O. */
    int __pad4;
    long long st_blocks;			/* Number 512-byte blocks allocated */
# ifdef __USE_MISC
    /* Nanosecond resolution timestamps are stored in a format
       equivalent to 'struct timespec'.  This is the type used
       whenever possible but the Unix namespace rules do not allow the
       identifier 'timespec' to appear in the <sys/stat.h> header.
       Therefore we have to handle the use of this header in strictly
       standard-compliant sources special.  */
    struct timespec st_atim; 			/* Time of last access.  */
    struct timespec st_mtim; 			/* Time of last modification.  */
    struct timespec st_ctim; 			/* Time of last status change.  */
# else
    int st_atime;				/* Time of last access. */
    unsigned int st_atime_nsec;
    int st_mtime;				/* Time of last modification. */
    unsigned int st_mtime_nsec;
    int st_ctime;				/* Time of last status change. */
    unsigned int st_ctime_nsec;
# endif
    unsigned int __unused4;
    unsigned int __unused5;
};
#endif

/* Tell code we have these members.  */
#define	_STATBUF_ST_BLKSIZE
#define _STATBUF_ST_RDEV
/* Nanosecond resolution time values are supported.  */
#define _STATBUF_ST_NSEC

/* Encoding of the file mode.  */

#define	__S_IFMT	0170000	/* These bits determine file type.  */

/* File types.  */
#define	__S_IFDIR	0040000	/* Directory.  */
#define	__S_IFCHR	0020000	/* Character device.  */
#define	__S_IFBLK	0060000	/* Block device.  */
#define	__S_IFREG	0100000	/* Regular file.  */
#define	__S_IFIFO	0010000	/* FIFO.  */
#define	__S_IFLNK	0120000	/* Symbolic link.  */
#define	__S_IFSOCK	0140000	/* Socket.  */

/* POSIX.1b objects.  Note that these macros always evaluate to zero.  But
   they do it by enforcing the correct use of the macros.  */
#define __S_TYPEISMQ(buf)  ((buf)->st_mode - (buf)->st_mode)
#define __S_TYPEISSEM(buf) ((buf)->st_mode - (buf)->st_mode)
#define __S_TYPEISSHM(buf) ((buf)->st_mode - (buf)->st_mode)

/* Protection bits.  */

#define	__S_ISUID	04000	/* Set user ID on execution.  */
#define	__S_ISGID	02000	/* Set group ID on execution.  */
#define	__S_ISVTX	01000	/* Save swapped text after use (sticky).  */
#define	__S_IREAD	0400	/* Read by owner.  */
#define	__S_IWRITE	0200	/* Write by owner.  */
#define	__S_IEXEC	0100	/* Execute by owner.  */

#ifdef __USE_ATFILE
# define UTIME_NOW	((1l << 30) - 1l)
# define UTIME_OMIT	((1l << 30) - 2l)
#endif
