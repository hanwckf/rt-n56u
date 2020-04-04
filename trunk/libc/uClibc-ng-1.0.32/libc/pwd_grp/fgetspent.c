/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define GETXXKEY_FUNC		fgetspent
#define GETXXKEY_ENTTYPE	struct spwd
#define GETXXKEY_ADD_PARAMS	FILE *stream
#define GETXXKEY_ADD_VARIABLES	stream
#define GETXXKEY_BUFLEN		__UCLIBC_PWD_BUFFER_SIZE__
#include "pwd_grp.c"
