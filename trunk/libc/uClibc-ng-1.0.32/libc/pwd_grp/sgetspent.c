/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define GETXXKEY_FUNC		sgetspent
#define GETXXKEY_ENTTYPE	struct spwd
#define GETXXKEY_ADD_PARAMS	const char *string
#define GETXXKEY_ADD_VARIABLES	string
#define GETXXKEY_BUFLEN		__UCLIBC_PWD_BUFFER_SIZE__
#include "pwd_grp.c"
