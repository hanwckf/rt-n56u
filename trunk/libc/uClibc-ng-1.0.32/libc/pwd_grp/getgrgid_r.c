/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define GETXXKEY_R_FUNC		getgrgid_r
#define GETXXKEY_R_PARSER	__parsegrent
#define GETXXKEY_R_ENTTYPE	struct group
#define GETXXKEY_R_TEST(ENT)	((ENT)->gr_gid == key)
#define DO_GETXXKEY_R_KEYTYPE	gid_t
#define DO_GETXXKEY_R_PATHNAME  _PATH_GROUP
#include "pwd_grp.c"
