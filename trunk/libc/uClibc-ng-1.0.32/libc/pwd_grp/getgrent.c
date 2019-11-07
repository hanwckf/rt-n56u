/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define GETXXKEY_FUNC		getgrent
#define GETXXKEY_ENTTYPE	struct group
#define GETXXKEY_ADD_PARAMS	void
#define GETXXKEY_BUFLEN		__UCLIBC_GRP_BUFFER_SIZE__
#include "pwd_grp.c"
