#if defined __powerpc64__
# include "powerpc64/clone.S"
#else
# include "powerpc32/clone.S"
#endif
