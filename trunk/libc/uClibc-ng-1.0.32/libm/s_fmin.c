/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "math.h"
#include "math_private.h"

double fmin(double x, double y)
{
  if (isnan(x))
    return y;
  if (isnan(y))
    return x;

  return x < y ? x : y;
}
libm_hidden_def(fmin)
