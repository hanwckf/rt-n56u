#include "_math_inc.h"

#if defined(__CONFIG_SPARC_V9__) || defined(__CONFIG_SPARC_V9B__)
# include "sparcv9/umul.S"
#elif defined(__CONFIG_SPARC_V8__)
# include "sparcv8/umul.S"
#else
# include "sparcv7/umul.S"
#endif
