/* Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#define _ISOC99_SOURCE

#include <math.h>
#include <float.h>
#include <stdio.h>

static int errors = 0;


static void
check (const char *testname, int result)
{
  if (!result) {
    printf ("Failure: %s\n", testname);
    errors++;
  }
}

#define TEST_FUNC(NAME, FLOAT, NANFUNC, EPSILON, HUGEVAL) \
static void								      \
NAME (void)								      \
{									      \
  /* Variables are declared volatile to forbid some compiler		      \
     optimizations.  */							      \
  volatile FLOAT Inf_var, NaN_var, zero_var, one_var;			      \
  FLOAT x1, x2;								      \
									      \
  zero_var = 0.0;							      \
  one_var = 1.0;							      \
  NaN_var = zero_var/zero_var;						      \
  Inf_var = one_var / zero_var;						      \
									      \
  (void) &zero_var;							      \
  (void) &one_var;							      \
  (void) &NaN_var;							      \
  (void) &Inf_var;							      \
									      \
									      \
  check (#FLOAT " isinf (inf) == 1", isinf (Inf_var) == 1);		      \
  check (#FLOAT " isinf (-inf) == -1", isinf (-Inf_var) == -1);		      \
  check (#FLOAT " !isinf (1)", !(isinf (one_var)));			      \
  check (#FLOAT " !isinf (NaN)", !(isinf (NaN_var)));			      \
									      \
  check (#FLOAT " isnan (NaN)", isnan (NaN_var));			      \
  check (#FLOAT " isnan (-NaN)", isnan (-NaN_var));			      \
  check (#FLOAT " !isnan (1)", !(isnan (one_var)));			      \
  check (#FLOAT " !isnan (inf)", !(isnan (Inf_var)));			      \
									      \
  check (#FLOAT " inf == inf", Inf_var == Inf_var);			      \
  check (#FLOAT " -inf == -inf", -Inf_var == -Inf_var);			      \
  check (#FLOAT " inf != -inf", Inf_var != -Inf_var);			      \
  check (#FLOAT " NaN != NaN", NaN_var != NaN_var);			      \
									      \
  /*									      \
     the same tests but this time with NAN from <bits/nan.h>		      \
     NAN is a double const						      \
   */									      \
  check (#FLOAT " isnan (NAN)", isnan (NAN));				      \
  check (#FLOAT " isnan (-NAN)", isnan (-NAN));				      \
  check (#FLOAT " !isinf (NAN)", !(isinf (NAN)));			      \
  check (#FLOAT " !isinf (-NAN)", !(isinf (-NAN)));			      \
  check (#FLOAT " NAN != NAN", NAN != NAN);				      \
									      \
  /*									      \
     And again with the value returned by the `nan' function.		      \
   */									      \
  check (#FLOAT " isnan (NAN)", isnan (NANFUNC ("")));			      \
  check (#FLOAT " isnan (-NAN)", isnan (-NANFUNC ("")));		      \
  check (#FLOAT " !isinf (NAN)", !(isinf (NANFUNC (""))));		      \
  check (#FLOAT " !isinf (-NAN)", !(isinf (-NANFUNC (""))));		      \
  check (#FLOAT " NAN != NAN", NANFUNC ("") != NANFUNC (""));		      \
									      \
  /* test if EPSILON is ok */						      \
  x1 = 1.0;								      \
  x2 = x1 + EPSILON;							      \
  check (#FLOAT " 1 != 1+EPSILON", x1 != x2);				      \
									      \
  x1 = 1.0;								      \
  x2 = x1 - EPSILON;							      \
  check (#FLOAT " 1 != 1-EPSILON", x1 != x2);				      \
									      \
  /* test if HUGE_VALx is ok */						      \
  x1 = HUGEVAL;								      \
  check (#FLOAT " isinf (HUGE_VALx) == +1", isinf (x1) == +1);		      \
  x1 = - HUGEVAL;							      \
  check (#FLOAT " isinf (-HUGE_VALx) == -1", isinf (x1) == -1);		      \
}

TEST_FUNC (float_test, float, nanf, FLT_EPSILON, HUGE_VALF)
TEST_FUNC (double_test, double, nan, DBL_EPSILON, HUGE_VAL)
#ifndef NO_LONG_DOUBLE
TEST_FUNC (ldouble_test, long double, nanl, LDBL_EPSILON, HUGE_VALL)
#endif

int
main (void)
{
  float_test ();
  double_test ();

#ifndef NO_LONG_DOUBLE
  ldouble_test ();
#endif

  return errors != 0;
}
