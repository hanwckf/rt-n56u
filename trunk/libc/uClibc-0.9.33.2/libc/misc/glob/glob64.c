/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>

#include <dirent.h>
#include <glob.h>
#include <sys/stat.h>

#define COMPILE_GLOB64    1

#include "glob.c"
