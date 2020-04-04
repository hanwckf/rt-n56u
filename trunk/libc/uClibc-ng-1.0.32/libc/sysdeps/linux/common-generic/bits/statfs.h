/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _SYS_STATFS_H
# error "Never include <bits/statfs.h> directly; use <sys/statfs.h> instead."
#endif

#include <endian.h>
#include <bits/align64bit.h>
#include <bits/types.h>
#include <bits/wordsize.h>

/* 64-bit libc uses the kernel's 'struct statfs', accessed via the
   statfs() syscall; 32-bit libc uses the kernel's 'struct statfs64'
   and accesses it via the statfs64() syscall.  All the various
   APIs offered by libc use the kernel shape for their struct statfs
   structure; the only difference is that 32-bit programs not
   using __USE_FILE_OFFSET64 only see the low 32 bits of some
   of the fields (the __fsblkcnt_t and __fsfilcnt_t fields).  */

#if defined __USE_FILE_OFFSET64
# define __field64(type, type64, name) type64 name
#elif __WORDSIZE == 64
# define __field64(type, type64, name) type name
#elif __BYTE_ORDER == __LITTLE_ENDIAN
# define __field64(type, type64, name) \
  type name __attribute__((__aligned__ (__alignof__ (type64)))); int __##name##_pad
#else
# define __field64(type, type64, name) \
  int __##name##_pad __attribute__((__aligned__ (__alignof__ (type64)))); type name
#endif

struct statfs
  {
    __SWORD_TYPE f_type;
    __SWORD_TYPE f_bsize;
    __field64(__fsblkcnt_t, __fsblkcnt64_t, f_blocks);
    __field64(__fsblkcnt_t, __fsblkcnt64_t, f_bfree);
    __field64(__fsblkcnt_t, __fsblkcnt64_t, f_bavail);
    __field64(__fsfilcnt_t, __fsfilcnt64_t, f_files);
    __field64(__fsfilcnt_t, __fsfilcnt64_t, f_ffree);
    __fsid_t f_fsid;
    __SWORD_TYPE f_namelen;
    __SWORD_TYPE f_frsize;
    __SWORD_TYPE f_flags;
    __SWORD_TYPE f_spare[4];
  };

#undef __field64

#ifdef __USE_LARGEFILE64
struct statfs64
  {
    __SWORD_TYPE f_type;
    __SWORD_TYPE f_bsize;
    __U64_TYPE f_blocks;
    __U64_TYPE f_bfree;
    __U64_TYPE f_bavail;
    __U64_TYPE f_files;
    __U64_TYPE f_ffree;
    __fsid_t f_fsid;
    __SWORD_TYPE f_namelen;
    __SWORD_TYPE f_frsize;
    __SWORD_TYPE f_flags;
    __SWORD_TYPE f_spare[4];
  };
#endif

/* Tell code we have these members.  */
#define _STATFS_F_NAMELEN
#define _STATFS_F_FRSIZE
