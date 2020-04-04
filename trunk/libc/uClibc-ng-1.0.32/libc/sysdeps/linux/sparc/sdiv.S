#include "_math_inc.h"

#if defined(__CONFIG_SPARC_V9__) || defined(__CONFIG_SPARC_V9B__)
# include "sparcv9/sdiv.S"
#elif defined(__CONFIG_SPARC_V8__)
# include "sparcv8/sdiv.S"
#else
# include "sparcv7/sdiv.S"
#endif
