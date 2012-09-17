/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>

#include <dirent.h>
#include <string.h>
#include "dirstream.h"

int alphasort64(const struct dirent64 **a, const struct dirent64 **b)
{
	return strcmp((*a)->d_name, (*b)->d_name);
}
