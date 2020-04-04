/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _DIRENT_H
# error "Never use <bits/dirent.h> directly; include <dirent.h> instead."
#endif

#include <bits/align64bit.h>
#include <endian.h>

struct dirent
  {
#ifndef __USE_FILE_OFFSET64
# if __BYTE_ORDER == __LITTLE_ENDIAN
    __U32_TYPE d_ino;
    __U32_TYPE __pad1;
    __S32_TYPE d_off;
    __S32_TYPE __pad2;
# else
    __U32_TYPE __pad1;
    __U32_TYPE d_ino;
    __S32_TYPE __pad2;
    __S32_TYPE d_off;
# endif /* __LITTLE_ENDIAN */
#else
    __U64_TYPE d_ino;
    __S64_TYPE d_off;
#endif
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];		/* We must not include limits.h! */
  } __ARCH_64BIT_ALIGNMENT__;

#ifdef __USE_LARGEFILE64
struct dirent64
  {
    __U64_TYPE d_ino;
    __S64_TYPE d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];		/* We must not include limits.h! */
  };
#endif

#define d_fileno	d_ino	/* Backwards compatibility.  */

#undef  _DIRENT_HAVE_D_NAMLEN
#define _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_OFF
#define _DIRENT_HAVE_D_TYPE
