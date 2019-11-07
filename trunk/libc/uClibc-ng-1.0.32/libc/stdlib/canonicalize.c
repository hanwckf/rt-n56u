/*
 * canonicalize.c -- Return a malloc'd string containing the canonical
 * absolute name of the named file.  The last file name component need
 * not exist, and may be a symlink to a nonexistent file.
 * Copyright (C) 2009 STMicroelectronics
 * Author: Salvatore Cro <salvatore.cro@st.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>

#ifdef __USE_GNU

char * canonicalize_file_name (const char *name)
{
	return realpath (name, NULL);
}
#endif
