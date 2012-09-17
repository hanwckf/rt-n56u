#if defined __powerpc64__
# include "powerpc64/vfork.S"
#else
# include "powerpc32/vfork.S"
#endif
