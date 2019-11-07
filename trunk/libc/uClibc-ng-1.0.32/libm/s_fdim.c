/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "math.h"
#include "math_private.h"
#include <errno.h>

double fdim(double x, double y)
{
  int cx = __fpclassify(x); /* need both NAN and INF */
  int cy = __fpclassify(y); /* need both NAN and INF */
  if (cx == FP_NAN || cy == NAN)
    return x - y;

  if (x <= y)
	  return .0;

  double z = x - y;
  if (isinf(z) && cx != FP_INFINITE && cy != FP_INFINITE)
	  __set_errno(ERANGE);

  return z;
}
libm_hidden_def(fdim)
