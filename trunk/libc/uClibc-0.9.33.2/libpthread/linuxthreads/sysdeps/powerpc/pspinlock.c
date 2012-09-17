#include <features.h>
#include <bits/wordsize.h>

#if __WORDSIZE == 32
# include "powerpc32/pspinlock.c"
#else
# include "powerpc64/pspinlock.c"
#endif
