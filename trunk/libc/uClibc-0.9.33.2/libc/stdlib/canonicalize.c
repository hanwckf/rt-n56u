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
#include <limits.h>

#ifdef __USE_GNU

#ifndef PATH_MAX
# ifdef _POSIX_VERSION
#  define PATH_MAX _POSIX_PATH_MAX
# else
#  ifdef MAXPATHLEN
#   define PATH_MAX MAXPATHLEN
#  else
#   define PATH_MAX 1024
#  endif
# endif
#endif

char * canonicalize_file_name (const char *name)
{
	char *buf = (char *) malloc(PATH_MAX);

	if(unlikely(buf == NULL))
		return NULL;

	*buf='\0';
	return realpath (name, buf);
}
#endif
