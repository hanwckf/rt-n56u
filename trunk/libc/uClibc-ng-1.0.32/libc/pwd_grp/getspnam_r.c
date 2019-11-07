/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define GETXXKEY_R_FUNC		getspnam_r
#define GETXXKEY_R_PARSER	__parsespent
#define GETXXKEY_R_ENTTYPE	struct spwd
#define GETXXKEY_R_TEST(ENT)	(!strcmp((ENT)->sp_namp, key))
#define DO_GETXXKEY_R_KEYTYPE	const char *__restrict
#define DO_GETXXKEY_R_PATHNAME  _PATH_SHADOW
#include "pwd_grp.c"
