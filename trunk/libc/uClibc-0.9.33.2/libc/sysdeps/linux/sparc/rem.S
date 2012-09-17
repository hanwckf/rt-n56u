#include "_math_inc.h"

#if defined(__CONFIG_SPARC_V9__) || defined(__CONFIG_SPARC_V9B__)
# include "sparcv9/rem.S"
#elif defined(__CONFIG_SPARC_V8__)
# include "sparcv8/rem.S"
#else
# include "sparcv7/rem.S"
#endif
