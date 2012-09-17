#include <features.h>
#include <bits/wordsize.h>

#if __WORDSIZE == 32
# include "sparc32/pt-machine.h"
#else
# include "sparc64/pt-machine.h"
#endif
