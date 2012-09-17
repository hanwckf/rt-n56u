#include <features.h>
#ifdef __UCLIBC_SUSV3_LEGACY__
# define memset bzero
# define __memset_chk __bzero_chk
# include "memset.S"
#endif
