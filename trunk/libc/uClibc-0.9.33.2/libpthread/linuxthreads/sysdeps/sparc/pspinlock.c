#include <features.h>
#include <bits/wordsize.h>

#if __WORDSIZE == 32

# if defined(__CONFIG_SPARC_V9B__)
#  include "sparc32/sparcv9b/pspinlock.c"
# else
#  include "sparc32/pspinlock.c"
# endif

#else
# include "sparc64/pspinlock.c"
#endif
