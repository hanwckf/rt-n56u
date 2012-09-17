#include <features.h>
#ifdef __UCLIBC_HAS_LFS__
#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS != 64 
#undef _FILE_OFFSET_BITS
#define	_FILE_OFFSET_BITS   64
#endif
#ifndef __USE_LARGEFILE64
# define __USE_LARGEFILE64	1
#endif
/* We absolutely do _NOT_ want interfaces silently
 * renamed under us or very bad things will happen... */
#ifdef __USE_FILE_OFFSET64
# undef __USE_FILE_OFFSET64
#endif
#include <dirent.h>
#include <string.h>
#include "dirstream.h"


int alphasort64(const void * a, const void * b)
{
    return strcmp ((*(const struct dirent64 **) a)->d_name,
	    (*(const struct dirent64 **) b)->d_name);
}
#endif /* __UCLIBC_HAS_LFS__ */

