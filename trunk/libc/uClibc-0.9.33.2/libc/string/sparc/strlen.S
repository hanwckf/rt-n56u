#include "_glibc_inc.h"

#if __WORDSIZE == 32

# if defined(__CONFIG_SPARC_V9B__)
#  include "sparc32/sparcv9b/strlen.S"
# else
#  include "sparc32/strlen.S"
# endif

#else
# include "sparc64/strlen.S"
#endif
