/*
 * Copyright (C) 2008-2009 Hai Zaar, Codefidence Ltd <haizaar@codefidence.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <dirent.h>

#if __WORDSIZE != 64
# include <string.h>
# include "dirstream.h"

int versionsort64(const struct dirent64 **a, const struct dirent64 **b)
{
	return strverscmp((*a)->d_name, (*b)->d_name);
}
#endif
