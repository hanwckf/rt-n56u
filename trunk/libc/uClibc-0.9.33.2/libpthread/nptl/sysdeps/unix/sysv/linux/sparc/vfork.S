#if defined(__arch64__)
#include "sparc64/vfork.S"
#else
#include "sparc32/vfork.S"
#endif
