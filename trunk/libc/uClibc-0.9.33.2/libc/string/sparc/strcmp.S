#include "_glibc_inc.h"

#if __WORDSIZE == 32

# if defined(__CONFIG_SPARC_V9B__)
#  include "sparc32/sparcv9b/strcmp.S"
# else
#  include "sparc32/strcmp.S"
# endif

#else
# include "sparc64/strcmp.S"
#endif

#ifndef __UCLIBC_HAS_LOCALE__
strong_alias(strcmp,strcoll)
libc_hidden_def(strcoll)
#endif
