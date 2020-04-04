/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <dirent.h>

#if __WORDSIZE != 64
# define __READDIR_R readdir64_r
# define __DIRENT_TYPE struct dirent64
# define __GETDENTS __getdents64

# include "readdir_r.c"
#endif
