/* Copyright (C) 2013 Gentoo Foundation
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>

int issetugid(void)
{
	return _pe_secure;
}
