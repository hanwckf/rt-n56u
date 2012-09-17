/* vi: set sw=4 ts=4: */
/*
 * crypt() for uClibc
 * Copyright (C) 2008 by Erik Andersen <andersen@uclibc.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define __FORCE_GLIBC
#include <crypt.h>
#include <unistd.h>
#include "libcrypt.h"
#include <syscall.h>

char *crypt(const char *key attribute_unused, const char *salt attribute_unused)
{
	__set_errno(ENOSYS);
	return NULL;
}

void
setkey(const char *key attribute_unused)
{
	__set_errno(ENOSYS);
}

void
encrypt(char *block attribute_unused, int flag attribute_unused)
{
	__set_errno(ENOSYS);
}
