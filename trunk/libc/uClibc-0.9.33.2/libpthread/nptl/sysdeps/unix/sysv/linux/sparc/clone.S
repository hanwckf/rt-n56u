#if defined(__arch64__)
#include "./sparc64/clone.S"
#else
#include "./sparc32/clone.S"
#endif
