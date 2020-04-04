/*
 * wrapper exp2l(x)
 */

#include <math.h>
#include "math_private.h"

#if !defined __NO_LONG_DOUBLE_MATH
long double
exp2l (long double x)
{
# if defined(__UCLIBC_HAS_FENV__)
  long double z = (long double) pow(2.0, (double) x);
  if (__builtin_expect (!isfinite (z) || z == 0, 0)
      && isfinite (x) && _LIB_VERSION != _IEEE_)
    /* exp2 overflow: 244, exp2 underflow: 245 */
    return __kernel_standard_l (x, x, 244 + !!signbit (x));

  return z;
# else
  return (long double) pow(2.0, (double) x);
# endif /* __UCLIBC_HAS_FENV__ */
}
#endif /* __NO_LONG_DOUBLE_MATH */
