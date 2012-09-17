#if defined(__arch64__)
#include "sparc64/pt-vfork.S"
#else
#include "sparc32/pt-vfork.S"
#endif
