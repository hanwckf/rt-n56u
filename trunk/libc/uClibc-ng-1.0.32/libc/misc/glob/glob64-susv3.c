/*
 * Copyright (C) 2006 Rich Felker <dalias@aerifal.cx>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>

#include <dirent.h>
#include <glob.h>
#include <sys/stat.h>

#define glob_t glob64_t
#define glob(pattern, flags, errfunc, pglob) \
  glob64 (pattern, flags, errfunc, pglob)
#define globfree(pglob) globfree64 (pglob)

#define __GLOB64    1

#include "glob-susv3.c"
