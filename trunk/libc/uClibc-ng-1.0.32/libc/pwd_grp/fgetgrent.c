/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

#ifdef __USE_SVID

#define GETXXKEY_FUNC		fgetgrent
#define GETXXKEY_ENTTYPE	struct group
#define GETXXKEY_ADD_PARAMS	FILE *stream
#define GETXXKEY_ADD_VARIABLES	stream
#define GETXXKEY_BUFLEN		__UCLIBC_GRP_BUFFER_SIZE__
#include "pwd_grp.c"

#endif
