#include <features.h>
#include <bits/wordsize.h>

#if __WORDSIZE == 32
# include "powerpc32/pt-machine.h"
#else
# include "powerpc64/pt-machine.h"
#endif
