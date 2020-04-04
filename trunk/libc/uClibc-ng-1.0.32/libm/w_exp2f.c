/*
 * wrapper exp2f(x)
 */

#include <math.h>
#include "math_private.h"

float
exp2f (float x)
{
#if defined(__UCLIBC_HAS_FENV__)
  float z = (float) pow(2.0, (double) x);
  if (__builtin_expect (!isfinite (z) || z == 0, 0)
      && isfinite (x) && _LIB_VERSION != _IEEE_)
    /* exp2 overflow: 144, exp2 underflow: 145 */
    return __kernel_standard_f (x, x, 144 + !!signbit (x));

  return z;
#else
  return (float) pow(2.0, (double) x);
#endif
}
