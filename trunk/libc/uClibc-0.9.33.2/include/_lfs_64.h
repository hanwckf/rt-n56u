/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
#include <features.h>

#ifdef __UCLIBC_HAS_LFS__

#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS != 64
#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS   64
#endif

#ifndef __USE_LARGEFILE64
# define __USE_LARGEFILE64      1
#endif

/* We absolutely do _NOT_ want interfaces silently
 * renamed under us or very bad things will happen... */
#ifdef __USE_FILE_OFFSET64
# undef __USE_FILE_OFFSET64
#endif

#else

# error Do not include this header in files not built when LFS is disabled

#endif
