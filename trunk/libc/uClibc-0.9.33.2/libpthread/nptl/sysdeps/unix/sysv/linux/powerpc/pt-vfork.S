#if defined __powerpc64__
# include "powerpc64/pt-vfork.S"
#else
# include "powerpc32/pt-vfork.S"
#endif
