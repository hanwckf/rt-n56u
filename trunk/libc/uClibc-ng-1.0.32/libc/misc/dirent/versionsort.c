/*
 * Copyright (C) 2008-2009 Hai Zaar, Codefidence Ltd <haizaar@codefidence.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <dirent.h>
#include <string.h>
#include "dirstream.h"

int versionsort(const struct dirent **a, const struct dirent **b)
{
	return strverscmp((*a)->d_name, (*b)->d_name);
}
#if __WORDSIZE == 64
strong_alias_untyped(versionsort,versionsort64)
#endif
