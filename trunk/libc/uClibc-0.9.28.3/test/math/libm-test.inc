/* Copyright (C) 1997, 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@arthur.rhein-neckar.de>, 1997.

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

/* Part of testsuite for libm.

   This file is processed by a perl script.  The resulting file has to
   be included by a master file that defines:

   Makros:
   FUNC(function): converts general function name (like cos) to
   name with correct suffix (e.g. cosl or cosf)
   MATHCONST(x):   like FUNC but for constants (e.g convert 0.0 to 0.0L)
   FLOAT:	   floating point type to test
   - TEST_MSG:	   informal message to be displayed
   CHOOSE(Clongdouble,Cdouble,Cfloat,Cinlinelongdouble,Cinlinedouble,Cinlinefloat):
   chooses one of the parameters as delta for testing
   equality
   PRINTF_EXPR	   Floating point conversion specification to print a variable
   of type FLOAT with printf.  PRINTF_EXPR just contains
   the specifier, not the percent and width arguments,
   e.g. "f".
   PRINTF_XEXPR	   Like PRINTF_EXPR, but print in hexadecimal format.
   PRINTF_NEXPR Like PRINTF_EXPR, but print nice.  */

/* This testsuite has currently tests for:
   acos, acosh, asin, asinh, atan, atan2, atanh,
   cbrt, ceil, copysign, cos, cosh, erf, erfc, exp, exp10, exp2, expm1,
   fabs, fdim, floor, fma, fmax, fmin, fmod, fpclassify,
   frexp, gamma, hypot,
   ilogb, isfinite, isinf, isnan, isnormal,
   isless, islessequal, isgreater, isgreaterequal, islessgreater, isunordered,
   j0, j1, jn,
   ldexp, lgamma, log, log10, log1p, log2, logb,
   modf, nearbyint, nextafter,
   pow, remainder, remquo, rint, lrint, llrint,
   round, lround, llround,
   scalb, scalbn, scalbln, signbit, sin, sincos, sinh, sqrt, tan, tanh, tgamma, trunc,
   y0, y1, yn

   and for the following complex math functions:
   cabs, cacos, cacosh, carg, casin, casinh, catan, catanh,
   ccos, ccosh, cexp, clog, cpow, cproj, csin, csinh, csqrt, ctan, ctanh.

   At the moment the following functions aren't tested:
   drem, significand, nan

   Parameter handling is primitive in the moment:
   --verbose=[0..3] for different levels of output:
   0: only error count
   1: basic report on failed tests (default)
   2: full report on all tests
   -v for full output (equals --verbose=3)
   -u for generation of an ULPs file
 */

/* "Philosophy":

   This suite tests some aspects of the correct implementation of
   mathematical functions in libm.  Some simple, specific parameters
   are tested for correctness but there's no exhaustive
   testing.  Handling of specific inputs (e.g. infinity, not-a-number)
   is also tested.  Correct handling of exceptions is checked
   against.  These implemented tests should check all cases that are
   specified in ISO C99.

   Exception testing: At the moment only divide-by-zero and invalid
   exceptions are tested.  Overflow/underflow and inexact exceptions
   aren't checked at the moment.

   NaN values: There exist signalling and quiet NaNs.  This implementation
   only uses signalling NaN as parameter but does not differenciate
   between the two kinds of NaNs as result.

   Inline functions: Inlining functions should give an improvement in
   speed - but not in precission.  The inlined functions return
   reasonable values for a reasonable range of input values.  The
   result is not necessarily correct for all values and exceptions are
   not correctly raised in all cases.  Problematic input and return
   values are infinity, not-a-number and minus zero.  This suite
   therefore does not check these specific inputs and the exception
   handling for inlined mathematical functions - just the "reasonable"
   values are checked.

   Beware: The tests might fail for any of the following reasons:
   - Tests are wrong
   - Functions are wrong
   - Floating Point Unit not working properly
   - Compiler has errors

   With e.g. gcc 2.7.2.2 the test for cexp fails because of a compiler error.


   To Do: All parameter should be numbers that can be represented as
   exact floating point values.  Currently some values cannot be represented
   exactly and therefore the result is not the expected result.
*/

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include "libm-test-ulps.h"
#include <complex.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

//#include <fenv.h>
#define feclearexcept(X)
#define fetestexcept(X)	    0

/* Possible exceptions */
#define NO_EXCEPTION			0x0
#define INVALID_EXCEPTION		0x1
#define DIVIDE_BY_ZERO_EXCEPTION	0x2
/* The next flags signals that those exceptions are allowed but not required.   */
#define INVALID_EXCEPTION_OK		0x4
#define DIVIDE_BY_ZERO_EXCEPTION_OK	0x8
#define EXCEPTIONS_OK INVALID_EXCEPTION_OK+DIVIDE_BY_ZERO_EXCEPTION_OK
/* Some special test flags, passed togther with exceptions.  */
#define IGNORE_ZERO_INF_SIGN		0x10

/* Various constants (we must supply them precalculated for accuracy).  */
#define M_PI_6l			.52359877559829887307710723054658383L
#define M_E2l			7.389056098930650227230427460575008L
#define M_E3l			20.085536923187667740928529654581719L
#define M_2_SQRT_PIl		3.5449077018110320545963349666822903L	/* 2 sqrt (M_PIl)  */
#define M_SQRT_PIl		1.7724538509055160272981674833411451L	/* sqrt (M_PIl)  */
#define M_LOG_SQRT_PIl		0.57236494292470008707171367567652933L	/* log(sqrt(M_PIl))  */
#define M_LOG_2_SQRT_PIl	1.265512123484645396488945797134706L	/* log(2*sqrt(M_PIl))  */
#define M_PI_34l		(M_PIl - M_PI_4l)		/* 3*pi/4 */
#define M_PI_34_LOG10El		(M_PIl - M_PI_4l) * M_LOG10El
#define M_PI2_LOG10El		M_PI_2l * M_LOG10El
#define M_PI4_LOG10El		M_PI_4l * M_LOG10El
#define M_PI_LOG10El		M_PIl * M_LOG10El

static FILE *ulps_file;	/* File to document difference.  */
static int output_ulps;	/* Should ulps printed?  */

static int noErrors;	/* number of errors */
static int noTests;	/* number of tests (without testing exceptions) */
static int noExcTests;	/* number of tests for exception flags */
static int noXFails;	/* number of expected failures.  */
static int noXPasses;	/* number of unexpected passes.  */

static int verbose;
static int output_max_error;	/* Should the maximal errors printed?  */
static int output_points;	/* Should the single function results printed?  */
static int ignore_max_ulp;	/* Should we ignore max_ulp?  */

static FLOAT minus_zero, plus_zero;
static FLOAT plus_infty, minus_infty, nan_value;

static FLOAT max_error, real_max_error, imag_max_error;


#define BUILD_COMPLEX(real, imag) \
  ({ __complex__ FLOAT __retval;					      \
     __real__ __retval = (real);					      \
     __imag__ __retval = (imag);					      \
     __retval; })

#define BUILD_COMPLEX_INT(real, imag) \
  ({ __complex__ int __retval;						      \
     __real__ __retval = (real);					      \
     __imag__ __retval = (imag);					      \
     __retval; })


#define MANT_DIG CHOOSE ((LDBL_MANT_DIG-1), (DBL_MANT_DIG-1), (FLT_MANT_DIG-1),  \
                         (LDBL_MANT_DIG-1), (DBL_MANT_DIG-1), (FLT_MANT_DIG-1))

static void
init_max_error (void)
{
  max_error = 0;
  real_max_error = 0;
  imag_max_error = 0;
  feclearexcept (FE_ALL_EXCEPT);
}

static void
set_max_error (FLOAT current, FLOAT *curr_max_error)
{
  if (current > *curr_max_error)
    *curr_max_error = current;
}


/* Should the message print to screen?  This depends on the verbose flag,
   and the test status.  */
static int
print_screen (int ok, int xfail)
{
  if (output_points
      && (verbose > 1
	  || (verbose == 1 && ok == xfail)))
    return 1;
  return 0;
}


/* Should the message print to screen?  This depends on the verbose flag,
   and the test status.  */
static int
print_screen_max_error (int ok, int xfail)
{
  if (output_max_error
      && (verbose > 1
	  || ((verbose == 1) && (ok == xfail))))
    return 1;
  return 0;
}

/* Update statistic counters.  */
static void
update_stats (int ok, int xfail)
{
  ++noTests;
  if (ok && xfail)
    ++noXPasses;
  else if (!ok && xfail)
    ++noXFails;
  else if (!ok && !xfail)
    ++noErrors;
}

static void
print_ulps (const char *test_name, FLOAT ulp)
{
  if (output_ulps)
    {
      fprintf (ulps_file, "Test \"%s\":\n", test_name);
      fprintf (ulps_file, "%s: %.0" PRINTF_NEXPR "\n",
	       CHOOSE("ldouble", "double", "float",
		      "ildouble", "idouble", "ifloat"),
	       FUNC(ceil) (ulp));
    }
}

static void
print_function_ulps (const char *function_name, FLOAT ulp)
{
  if (output_ulps)
    {
      fprintf (ulps_file, "Function: \"%s\":\n", function_name);
      fprintf (ulps_file, "%s: %.0" PRINTF_NEXPR "\n",
	       CHOOSE("ldouble", "double", "float",
		      "ildouble", "idouble", "ifloat"),
	       FUNC(ceil) (ulp));
    }
}


static void
print_complex_function_ulps (const char *function_name, FLOAT real_ulp,
			     FLOAT imag_ulp)
{
  if (output_ulps)
    {
      if (real_ulp != 0.0)
	{
	  fprintf (ulps_file, "Function: Real part of \"%s\":\n", function_name);
	  fprintf (ulps_file, "%s: %.0" PRINTF_NEXPR "\n",
		   CHOOSE("ldouble", "double", "float",
			  "ildouble", "idouble", "ifloat"),
		   FUNC(ceil) (real_ulp));
	}
      if (imag_ulp != 0.0)
	{
	  fprintf (ulps_file, "Function: Imaginary part of \"%s\":\n", function_name);
	  fprintf (ulps_file, "%s: %.0" PRINTF_NEXPR "\n",
		   CHOOSE("ldouble", "double", "float",
			  "ildouble", "idouble", "ifloat"),
		   FUNC(ceil) (imag_ulp));
	}


    }
}



/* Test if Floating-Point stack hasn't changed */
static void
fpstack_test (const char *test_name)
{
#ifdef i386
  static int old_stack;
  int sw;

  asm ("fnstsw" : "=a" (sw));
  sw >>= 11;
  sw &= 7;

  if (sw != old_stack)
    {
      printf ("FP-Stack wrong after test %s (%d, should be %d)\n",
	      test_name, sw, old_stack);
      ++noErrors;
      old_stack = sw;
    }
#endif
}


static void
print_max_error (const char *func_name, FLOAT allowed, int xfail)
{
  int ok = 0;

  if (max_error == 0.0 || (max_error <= allowed && !ignore_max_ulp))
    {
      ok = 1;
    }

  if (!ok)
    print_function_ulps (func_name, max_error);


  if (print_screen_max_error (ok, xfail))
    {
      printf ("Maximal error of `%s'\n", func_name);
      printf (" is      : %.0" PRINTF_NEXPR " ulp\n", FUNC(ceil) (max_error));
      printf (" accepted: %.0" PRINTF_NEXPR " ulp\n", FUNC(ceil) (allowed));
    }

  update_stats (ok, xfail);
}


static void
print_complex_max_error (const char *func_name, __complex__ FLOAT allowed,
			 __complex__ int xfail)
{
  int ok = 0;

  if ((real_max_error == 0 && imag_max_error == 0)
      || (real_max_error <= __real__ allowed
	  && imag_max_error <= __imag__ allowed
	  && !ignore_max_ulp))
    {
      ok = 1;
    }

  if (!ok)
    print_complex_function_ulps (func_name, real_max_error, imag_max_error);


  if (print_screen_max_error (ok, xfail))
    {
      printf ("Maximal error of real part of: %s\n", func_name);
      printf (" is      : %.0" PRINTF_NEXPR " ulp\n",
	      FUNC(ceil) (real_max_error));
      printf (" accepted: %.0" PRINTF_NEXPR " ulp\n",
	      FUNC(ceil) (__real__ allowed));
      printf ("Maximal error of imaginary part of: %s\n", func_name);
      printf (" is      : %.0" PRINTF_NEXPR " ulp\n",
	      FUNC(ceil) (imag_max_error));
      printf (" accepted: %.0" PRINTF_NEXPR " ulp\n",
	      FUNC(ceil) (__imag__ allowed));
    }

  update_stats (ok, xfail);
}


/* Test whether a given exception was raised.  */
static void
test_single_exception (const char *test_name,
		       int exception,
		       int exc_flag,
		       int fe_flag,
		       const char *flag_name)
{
#ifndef TEST_INLINE
  int ok = 1;
  if (exception & exc_flag)
    {
      if (fetestexcept (fe_flag))
	{
	  if (print_screen (1, 0))
	    printf ("Pass: %s: Exception \"%s\" set\n", test_name, flag_name);
	}
      else
	{
	  ok = 0;
	  if (print_screen (0, 0))
	    printf ("Failure: %s: Exception \"%s\" not set\n",
		    test_name, flag_name);
	}
    }
  else
    {
      if (fetestexcept (fe_flag))
	{
	  ok = 0;
	  if (print_screen (0, 0))
	    printf ("Failure: %s: Exception \"%s\" set\n",
		    test_name, flag_name);
	}
      else
	{
	  if (print_screen (1, 0))
	    printf ("%s: Exception \"%s\" not set\n", test_name,
		    flag_name);
	}
    }
  if (!ok)
    ++noErrors;

#endif
}


/* Test whether exceptions given by EXCEPTION are raised.  Ignore thereby
   allowed but not required exceptions.
*/
static void
test_exceptions (const char *test_name, int exception)
{
  ++noExcTests;
#ifdef FE_DIVBYZERO
  if ((exception & DIVIDE_BY_ZERO_EXCEPTION_OK) == 0)
    test_single_exception (test_name, exception,
			   DIVIDE_BY_ZERO_EXCEPTION, FE_DIVBYZERO,
			   "Divide by zero");
#endif
#ifdef FE_INVALID
  if ((exception & INVALID_EXCEPTION_OK) == 0)
    test_single_exception (test_name, exception, INVALID_EXCEPTION, FE_INVALID,
			 "Invalid operation");
#endif
  feclearexcept (FE_ALL_EXCEPT);
}


static void
check_float_internal (const char *test_name, FLOAT computed, FLOAT expected,
		      FLOAT max_ulp, int xfail, int exceptions,
		      FLOAT *curr_max_error)
{
  int ok = 0;
  int print_diff = 0;
  FLOAT diff = 0;
  FLOAT ulp = 0;

  test_exceptions (test_name, exceptions);
  if (isnan (computed) && isnan (expected))
    ok = 1;
  else if (isinf (computed) && isinf (expected))
    {
      /* Test for sign of infinities.  */
      if ((exceptions & IGNORE_ZERO_INF_SIGN) == 0
	  && signbit (computed) != signbit (expected))
	{
	  ok = 0;
	  printf ("infinity has wrong sign.\n");
	}
      else
	ok = 1;
    }
  /* Don't calc ulp for NaNs or infinities.  */
  else if (isinf (computed) || isnan (computed) || isinf (expected) || isnan (expected))
    ok = 0;
  else
    {
      diff = FUNC(fabs) (computed - expected);
      /* ilogb (0) isn't allowed.  */
      if (expected == 0.0)
	ulp = diff / FUNC(ldexp) (1.0, - MANT_DIG);
      else
	ulp = diff / FUNC(ldexp) (1.0, FUNC(ilogb) (expected) - MANT_DIG);
      set_max_error (ulp, curr_max_error);
      print_diff = 1;
      if ((exceptions & IGNORE_ZERO_INF_SIGN) == 0
	  && computed == 0.0 && expected == 0.0
	  && signbit(computed) != signbit (expected))
	ok = 0;
      else if (ulp == 0.0 || (ulp <= max_ulp && !ignore_max_ulp))
	ok = 1;
      else
	{
	  ok = 0;
	  print_ulps (test_name, ulp);
	}

    }
  if (print_screen (ok, xfail))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
	      computed, computed);
      printf (" should be:  % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
	      expected, expected);
      if (print_diff)
	{
	  printf (" difference: % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR
		  "\n", diff, diff);
	  printf (" ulp       : % .4" PRINTF_NEXPR "\n", ulp);
	  printf (" max.ulp   : % .4" PRINTF_NEXPR "\n", max_ulp);
	}
    }
  update_stats (ok, xfail);

  fpstack_test (test_name);
}


static void
check_float (const char *test_name, FLOAT computed, FLOAT expected,
	     FLOAT max_ulp, int xfail, int exceptions)
{
  check_float_internal (test_name, computed, expected, max_ulp, xfail,
			exceptions, &max_error);
}


static void
check_complex (const char *test_name, __complex__ FLOAT computed,
	       __complex__ FLOAT expected,
	       __complex__ FLOAT max_ulp, __complex__ int xfail,
	       int exception)
{
  FLOAT part_comp, part_exp, part_max_ulp;
  int part_xfail;
  char str[200];

  sprintf (str, "Real part of: %s", test_name);
  part_comp = __real__ computed;
  part_exp = __real__ expected;
  part_max_ulp = __real__ max_ulp;
  part_xfail = __real__ xfail;

  check_float_internal (str, part_comp, part_exp, part_max_ulp, part_xfail,
			exception, &real_max_error);

  sprintf (str, "Imaginary part of: %s", test_name);
  part_comp = __imag__ computed;
  part_exp = __imag__ expected;
  part_max_ulp = __imag__ max_ulp;
  part_xfail = __imag__ xfail;

  /* Don't check again for exceptions, just pass through the
     zero/inf sign test.  */
  check_float_internal (str, part_comp, part_exp, part_max_ulp, part_xfail,
			exception & IGNORE_ZERO_INF_SIGN,
			&imag_max_error);
}


/* Check that computed and expected values are equal (int values).  */
static void
check_int (const char *test_name, int computed, int expected, int max_ulp,
	   int xfail, int exceptions)
{
  int diff = computed - expected;
  int ok = 0;

  test_exceptions (test_name, exceptions);
  noTests++;
  if (abs (diff) <= max_ulp)
    ok = 1;

  if (!ok)
    print_ulps (test_name, diff);

  if (print_screen (ok, xfail))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %d\n", computed);
      printf (" should be:  %d\n", expected);
    }

  update_stats (ok, xfail);
  fpstack_test (test_name);
}


/* Check that computed and expected values are equal (long int values).  */
static void
check_long (const char *test_name, long int computed, long int expected,
	    long int max_ulp, int xfail, int exceptions)
{
  long int diff = computed - expected;
  int ok = 0;

  test_exceptions (test_name, exceptions);
  noTests++;
  if (labs (diff) <= max_ulp)
    ok = 1;

  if (!ok)
    print_ulps (test_name, diff);

  if (print_screen (ok, xfail))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %ld\n", computed);
      printf (" should be:  %ld\n", expected);
    }

  update_stats (ok, xfail);
  fpstack_test (test_name);
}


/* Check that computed value is true/false.  */
static void
check_bool (const char *test_name, int computed, int expected,
	    long int max_ulp, int xfail, int exceptions)
{
  int ok = 0;

  test_exceptions (test_name, exceptions);
  noTests++;
  if ((computed == 0) == (expected == 0))
    ok = 1;

  if (print_screen (ok, xfail))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %d\n", computed);
      printf (" should be:  %d\n", expected);
    }

  update_stats (ok, xfail);
  fpstack_test (test_name);
}


/* check that computed and expected values are equal (long int values) */
static void
check_longlong (const char *test_name, long long int computed,
		long long int expected,
		long long int max_ulp, int xfail,
		int exceptions)
{
  long long int diff = computed - expected;
  int ok = 0;

  test_exceptions (test_name, exceptions);
  noTests++;
  if (llabs (diff) <= max_ulp)
    ok = 1;

  if (!ok)
    print_ulps (test_name, diff);

  if (print_screen (ok, xfail))
    {
      if (!ok)
	printf ("Failure:");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %lld\n", computed);
      printf (" should be:  %lld\n", expected);
    }

  update_stats (ok, xfail);
  fpstack_test (test_name);
}



/* This is to prevent messages from the SVID libm emulation.  */
int
matherr (struct exception *x __attribute__ ((unused)))
{
  return 1;
}


/****************************************************************************
  Tests for single functions of libm.
  Please keep them alphabetically sorted!
****************************************************************************/

static void
acos_test (void)
{
  errno = 0;
  FUNC(acos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acos);

  TEST_f_f (acos, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (acos, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (acos, nan_value, nan_value);

  /* |x| > 1: */
  TEST_f_f (acos, 1.1L, nan_value, INVALID_EXCEPTION);
  TEST_f_f (acos, -1.1L, nan_value, INVALID_EXCEPTION);

  TEST_f_f (acos, 0, M_PI_2l);
  TEST_f_f (acos, minus_zero, M_PI_2l);
  TEST_f_f (acos, 1, 0);
  TEST_f_f (acos, -1, M_PIl);
  TEST_f_f (acos, 0.5, M_PI_6l*2.0);
  TEST_f_f (acos, -0.5, M_PI_6l*4.0);
  TEST_f_f (acos, 0.7L, 0.79539883018414355549096833892476432L);

  END (acos);
}

static void
acosh_test (void)
{
  errno = 0;
  FUNC(acosh) (7);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acosh);

  TEST_f_f (acosh, plus_infty, plus_infty);
  TEST_f_f (acosh, minus_infty, nan_value, INVALID_EXCEPTION);

  /* x < 1:  */
  TEST_f_f (acosh, -1.1L, nan_value, INVALID_EXCEPTION);

  TEST_f_f (acosh, 1, 0);
  TEST_f_f (acosh, 7, 2.633915793849633417250092694615937L);

  END (acosh);
}

static void
asin_test (void)
{
  errno = 0;
  FUNC(asin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asin);

  TEST_f_f (asin, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (asin, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (asin, nan_value, nan_value);

  /* asin x == NaN plus invalid exception for |x| > 1.  */
  TEST_f_f (asin, 1.1L, nan_value, INVALID_EXCEPTION);
  TEST_f_f (asin, -1.1L, nan_value, INVALID_EXCEPTION);

  TEST_f_f (asin, 0, 0);
  TEST_f_f (asin, minus_zero, minus_zero);
  TEST_f_f (asin, 0.5, M_PI_6l);
  TEST_f_f (asin, -0.5, -M_PI_6l);
  TEST_f_f (asin, 1.0, M_PI_2l);
  TEST_f_f (asin, -1.0, -M_PI_2l);
  TEST_f_f (asin, 0.7L, 0.77539749661075306374035335271498708L);

  END (asin);
}

static void
asinh_test (void)
{
  errno = 0;
  FUNC(asinh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asinh);

  TEST_f_f (asinh, 0, 0);
  TEST_f_f (asinh, minus_zero, minus_zero);
#ifndef TEST_INLINE
  TEST_f_f (asinh, plus_infty, plus_infty);
  TEST_f_f (asinh, minus_infty, minus_infty);
#endif
  TEST_f_f (asinh, nan_value, nan_value);
  TEST_f_f (asinh, 0.7L, 0.652666566082355786L);

  END (asinh);
}

static void
atan_test (void)
{
  errno = 0;
  FUNC(atan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (atan);

  TEST_f_f (atan, 0, 0);
  TEST_f_f (atan, minus_zero, minus_zero);

  TEST_f_f (atan, plus_infty, M_PI_2l);
  TEST_f_f (atan, minus_infty, -M_PI_2l);
  TEST_f_f (atan, nan_value, nan_value);

  TEST_f_f (atan, 1, M_PI_4l);
  TEST_f_f (atan, -1, -M_PI_4l);

  TEST_f_f (atan, 0.7L, 0.61072596438920861654375887649023613L);

  END (atan);
}



static void
atanh_test (void)
{
  errno = 0;
  FUNC(atanh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (atanh);


  TEST_f_f (atanh, 0, 0);
  TEST_f_f (atanh, minus_zero, minus_zero);

  TEST_f_f (atanh, 1, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (atanh, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (atanh, nan_value, nan_value);

  /* atanh (x) == NaN plus invalid exception if |x| > 1.  */
  TEST_f_f (atanh, 1.1L, nan_value, INVALID_EXCEPTION);
  TEST_f_f (atanh, -1.1L, nan_value, INVALID_EXCEPTION);

  TEST_f_f (atanh, 0.7L, 0.8673005276940531944L);

  END (atanh);
}

static void
atan2_test (void)
{
  errno = 0;
  FUNC(atan2) (-0, 1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (atan2);

  /* atan2 (0,x) == 0 for x > 0.  */
  TEST_ff_f (atan2, 0, 1, 0);

  /* atan2 (-0,x) == -0 for x > 0.  */
  TEST_ff_f (atan2, minus_zero, 1, minus_zero);

  TEST_ff_f (atan2, 0, 0, 0);
  TEST_ff_f (atan2, minus_zero, 0, minus_zero);

  /* atan2 (+0,x) == +pi for x < 0.  */
  TEST_ff_f (atan2, 0, -1, M_PIl);

  /* atan2 (-0,x) == -pi for x < 0.  */
  TEST_ff_f (atan2, minus_zero, -1, -M_PIl);

  TEST_ff_f (atan2, 0, minus_zero, M_PIl);
  TEST_ff_f (atan2, minus_zero, minus_zero, -M_PIl);

  /* atan2 (y,+0) == pi/2 for y > 0.  */
  TEST_ff_f (atan2, 1, 0, M_PI_2l);

  /* atan2 (y,-0) == pi/2 for y > 0.  */
  TEST_ff_f (atan2, 1, minus_zero, M_PI_2l);

  /* atan2 (y,+0) == -pi/2 for y < 0.  */
  TEST_ff_f (atan2, -1, 0, -M_PI_2l);

  /* atan2 (y,-0) == -pi/2 for y < 0.  */
  TEST_ff_f (atan2, -1, minus_zero, -M_PI_2l);

  /* atan2 (y,inf) == +0 for finite y > 0.  */
  TEST_ff_f (atan2, 1, plus_infty, 0);

  /* atan2 (y,inf) == -0 for finite y < 0.  */
  TEST_ff_f (atan2, -1, plus_infty, minus_zero);

  /* atan2(+inf, x) == pi/2 for finite x.  */
  TEST_ff_f (atan2, plus_infty, -1, M_PI_2l);

  /* atan2(-inf, x) == -pi/2 for finite x.  */
  TEST_ff_f (atan2, minus_infty, 1, -M_PI_2l);

  /* atan2 (y,-inf) == +pi for finite y > 0.  */
  TEST_ff_f (atan2, 1, minus_infty, M_PIl);

  /* atan2 (y,-inf) == -pi for finite y < 0.  */
  TEST_ff_f (atan2, -1, minus_infty, -M_PIl);

  TEST_ff_f (atan2, plus_infty, plus_infty, M_PI_4l);
  TEST_ff_f (atan2, minus_infty, plus_infty, -M_PI_4l);
  TEST_ff_f (atan2, plus_infty, minus_infty, M_PI_34l);
  TEST_ff_f (atan2, minus_infty, minus_infty, -M_PI_34l);
  TEST_ff_f (atan2, nan_value, nan_value, nan_value);

  TEST_ff_f (atan2, 0.7L, 1, 0.61072596438920861654375887649023613L);
  TEST_ff_f (atan2, -0.7L, 1.0L, -0.61072596438920861654375887649023613L);
  TEST_ff_f (atan2, 0.7L, -1.0L, 2.530866689200584621918884506789267L);
  TEST_ff_f (atan2, -0.7L, -1.0L, -2.530866689200584621918884506789267L);
  TEST_ff_f (atan2, 0.4L, 0.0003L, 1.5700463269355215717704032607580829L);
  TEST_ff_f (atan2, 1.4L, -0.93L, 2.1571487668237843754887415992772736L);

  END (atan2);
}


static void
cabs_test (void)
{
  errno = 0;
  FUNC(cabs) (BUILD_COMPLEX (0.7L, 12.4L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cabs);

  /* cabs (x + iy) is specified as hypot (x,y) */

  /* cabs (+inf + i x) == +inf.  */
  TEST_c_f (cabs, plus_infty, 1.0, plus_infty);
  /* cabs (-inf + i x) == +inf.  */
  TEST_c_f (cabs, minus_infty, 1.0, plus_infty);

  TEST_c_f (cabs, minus_infty, nan_value, plus_infty);
  TEST_c_f (cabs, minus_infty, nan_value, plus_infty);

  TEST_c_f (cabs, nan_value, nan_value, nan_value);

  /* cabs (x,y) == cabs (y,x).  */
  TEST_c_f (cabs, 0.7L, 12.4L, 12.419742348374220601176836866763271L);
  /* cabs (x,y) == cabs (-x,y).  */
  TEST_c_f (cabs, -12.4L, 0.7L, 12.419742348374220601176836866763271L);
  /* cabs (x,y) == cabs (-y,x).  */
  TEST_c_f (cabs, -0.7L, 12.4L, 12.419742348374220601176836866763271L);
  /* cabs (x,y) == cabs (-x,-y).  */
  TEST_c_f (cabs, -12.4L, -0.7L, 12.419742348374220601176836866763271L);
  /* cabs (x,y) == cabs (-y,-x).  */
  TEST_c_f (cabs, -0.7L, -12.4L, 12.419742348374220601176836866763271L);
  /* cabs (x,0) == fabs (x).  */
  TEST_c_f (cabs, -0.7L, 0, 0.7L);
  TEST_c_f (cabs, 0.7L, 0, 0.7L);
  TEST_c_f (cabs, -1.0L, 0, 1.0L);
  TEST_c_f (cabs, 1.0L, 0, 1.0L);
  TEST_c_f (cabs, -5.7e7L, 0, 5.7e7L);
  TEST_c_f (cabs, 5.7e7L, 0, 5.7e7L);

  TEST_c_f (cabs, 0.7L, 1.2L, 1.3892443989449804508432547041028554L);

  END (cabs);
}

#if 0
static void
cacos_test (void)
{
  errno = 0;
  FUNC(cacos) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cacos);


  TEST_c_c (cacos, 0, 0, M_PI_2l, minus_zero);
  TEST_c_c (cacos, minus_zero, 0, M_PI_2l, minus_zero);
  TEST_c_c (cacos, minus_zero, minus_zero, M_PI_2l, 0.0);
  TEST_c_c (cacos, 0, minus_zero, M_PI_2l, 0.0);

  TEST_c_c (cacos, minus_infty, plus_infty, M_PI_34l, minus_infty);
  TEST_c_c (cacos, minus_infty, minus_infty, M_PI_34l, plus_infty);

  TEST_c_c (cacos, plus_infty, plus_infty, M_PI_4l, minus_infty);
  TEST_c_c (cacos, plus_infty, minus_infty, M_PI_4l, plus_infty);

  TEST_c_c (cacos, -10.0, plus_infty, M_PI_2l, minus_infty);
  TEST_c_c (cacos, -10.0, minus_infty, M_PI_2l, plus_infty);
  TEST_c_c (cacos, 0, plus_infty, M_PI_2l, minus_infty);
  TEST_c_c (cacos, 0, minus_infty, M_PI_2l, plus_infty);
  TEST_c_c (cacos, 0.1L, plus_infty, M_PI_2l, minus_infty);
  TEST_c_c (cacos, 0.1L, minus_infty, M_PI_2l, plus_infty);

  TEST_c_c (cacos, minus_infty, 0, M_PIl, minus_infty);
  TEST_c_c (cacos, minus_infty, minus_zero, M_PIl, plus_infty);
  TEST_c_c (cacos, minus_infty, 100, M_PIl, minus_infty);
  TEST_c_c (cacos, minus_infty, -100, M_PIl, plus_infty);

  TEST_c_c (cacos, plus_infty, 0, 0.0, minus_infty);
  TEST_c_c (cacos, plus_infty, minus_zero, 0.0, plus_infty);
  TEST_c_c (cacos, plus_infty, 0.5, 0.0, minus_infty);
  TEST_c_c (cacos, plus_infty, -0.5, 0.0, plus_infty);

  TEST_c_c (cacos, plus_infty, nan_value, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (cacos, minus_infty, nan_value, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (cacos, 0, nan_value, M_PI_2l, nan_value);
  TEST_c_c (cacos, minus_zero, nan_value, M_PI_2l, nan_value);

  TEST_c_c (cacos, nan_value, plus_infty, nan_value, minus_infty);
  TEST_c_c (cacos, nan_value, minus_infty, nan_value, plus_infty);

  TEST_c_c (cacos, 10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cacos, -10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (cacos, nan_value, 0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cacos, nan_value, -0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (cacos, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (cacos, 0.7L, 1.2L, 1.1351827477151551088992008271819053L, -1.0927647857577371459105272080819308L);
  TEST_c_c (cacos, -2, -3, 2.1414491111159960199416055713254211L, 1.9833870299165354323470769028940395L);

  END (cacos, complex);
}


static void
cacosh_test (void)
{
  errno = 0;
  FUNC(cacosh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cacosh);


  TEST_c_c (cacosh, 0, 0, 0.0, M_PI_2l);
  TEST_c_c (cacosh, minus_zero, 0, 0.0, M_PI_2l);
  TEST_c_c (cacosh, 0, minus_zero, 0.0, -M_PI_2l);
  TEST_c_c (cacosh, minus_zero, minus_zero, 0.0, -M_PI_2l);
  TEST_c_c (cacosh, minus_infty, plus_infty, plus_infty, M_PI_34l);
  TEST_c_c (cacosh, minus_infty, minus_infty, plus_infty, -M_PI_34l);

  TEST_c_c (cacosh, plus_infty, plus_infty, plus_infty, M_PI_4l);
  TEST_c_c (cacosh, plus_infty, minus_infty, plus_infty, -M_PI_4l);

  TEST_c_c (cacosh, -10.0, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (cacosh, -10.0, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (cacosh, 0, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (cacosh, 0, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (cacosh, 0.1L, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (cacosh, 0.1L, minus_infty, plus_infty, -M_PI_2l);

  TEST_c_c (cacosh, minus_infty, 0, plus_infty, M_PIl);
  TEST_c_c (cacosh, minus_infty, minus_zero, plus_infty, -M_PIl);
  TEST_c_c (cacosh, minus_infty, 100, plus_infty, M_PIl);
  TEST_c_c (cacosh, minus_infty, -100, plus_infty, -M_PIl);

  TEST_c_c (cacosh, plus_infty, 0, plus_infty, 0.0);
  TEST_c_c (cacosh, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (cacosh, plus_infty, 0.5, plus_infty, 0.0);
  TEST_c_c (cacosh, plus_infty, -0.5, plus_infty, minus_zero);

  TEST_c_c (cacosh, plus_infty, nan_value, plus_infty, nan_value);
  TEST_c_c (cacosh, minus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (cacosh, 0, nan_value, nan_value, nan_value);
  TEST_c_c (cacosh, minus_zero, nan_value, nan_value, nan_value);

  TEST_c_c (cacosh, nan_value, plus_infty, plus_infty, nan_value);
  TEST_c_c (cacosh, nan_value, minus_infty, plus_infty, nan_value);

  TEST_c_c (cacosh, 10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cacosh, -10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (cacosh, nan_value, 0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cacosh, nan_value, -0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (cacosh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (cacosh, 0.7L, 1.2L, 1.0927647857577371459105272080819308L, 1.1351827477151551088992008271819053L);
  TEST_c_c (cacosh, -2, -3, -1.9833870299165354323470769028940395L, 2.1414491111159960199416055713254211L);

  END (cacosh, complex);
}

static void
carg_test (void)
{
  START (carg);

  /* carg (x + iy) is specified as atan2 (y, x) */

  /* carg (x + i 0) == 0 for x > 0.  */
  TEST_c_f (carg, 2.0, 0, 0);
  /* carg (x - i 0) == -0 for x > 0.  */
  TEST_c_f (carg, 2.0, minus_zero, minus_zero);

  TEST_c_f (carg, 0, 0, 0);
  TEST_c_f (carg, 0, minus_zero, minus_zero);

  /* carg (x + i 0) == +pi for x < 0.  */
  TEST_c_f (carg, -2.0, 0, M_PIl);

  /* carg (x - i 0) == -pi for x < 0.  */
  TEST_c_f (carg, -2.0, minus_zero, -M_PIl);

  TEST_c_f (carg, minus_zero, 0, M_PIl);
  TEST_c_f (carg, minus_zero, minus_zero, -M_PIl);

  /* carg (+0 + i y) == pi/2 for y > 0.  */
  TEST_c_f (carg, 0, 2.0, M_PI_2l);

  /* carg (-0 + i y) == pi/2 for y > 0.  */
  TEST_c_f (carg, minus_zero, 2.0, M_PI_2l);

  /* carg (+0 + i y) == -pi/2 for y < 0.  */
  TEST_c_f (carg, 0, -2.0, -M_PI_2l);

  /* carg (-0 + i y) == -pi/2 for y < 0.  */
  TEST_c_f (carg, minus_zero, -2.0, -M_PI_2l);

  /* carg (inf + i y) == +0 for finite y > 0.  */
  TEST_c_f (carg, plus_infty, 2.0, 0);

  /* carg (inf + i y) == -0 for finite y < 0.  */
  TEST_c_f (carg, plus_infty, -2.0, minus_zero);

  /* carg(x + i inf) == pi/2 for finite x.  */
  TEST_c_f (carg, 10.0, plus_infty, M_PI_2l);

  /* carg(x - i inf) == -pi/2 for finite x.  */
  TEST_c_f (carg, 10.0, minus_infty, -M_PI_2l);

  /* carg (-inf + i y) == +pi for finite y > 0.  */
  TEST_c_f (carg, minus_infty, 10.0, M_PIl);

  /* carg (-inf + i y) == -pi for finite y < 0.  */
  TEST_c_f (carg, minus_infty, -10.0, -M_PIl);

  TEST_c_f (carg, plus_infty, plus_infty, M_PI_4l);

  TEST_c_f (carg, plus_infty, minus_infty, -M_PI_4l);

  TEST_c_f (carg, minus_infty, plus_infty, 3 * M_PI_4l);

  TEST_c_f (carg, minus_infty, minus_infty, -3 * M_PI_4l);

  TEST_c_f (carg, nan_value, nan_value, nan_value);

  END (carg);
}

static void
casin_test (void)
{
  errno = 0;
  FUNC(casin) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (casin);

  TEST_c_c (casin, 0, 0, 0.0, 0.0);
  TEST_c_c (casin, minus_zero, 0, minus_zero, 0.0);
  TEST_c_c (casin, 0, minus_zero, 0.0, minus_zero);
  TEST_c_c (casin, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (casin, plus_infty, plus_infty, M_PI_4l, plus_infty);
  TEST_c_c (casin, plus_infty, minus_infty, M_PI_4l, minus_infty);
  TEST_c_c (casin, minus_infty, plus_infty, -M_PI_4l, plus_infty);
  TEST_c_c (casin, minus_infty, minus_infty, -M_PI_4l, minus_infty);

  TEST_c_c (casin, -10.0, plus_infty, minus_zero, plus_infty);
  TEST_c_c (casin, -10.0, minus_infty, minus_zero, minus_infty);
  TEST_c_c (casin, 0, plus_infty, 0.0, plus_infty);
  TEST_c_c (casin, 0, minus_infty, 0.0, minus_infty);
  TEST_c_c (casin, minus_zero, plus_infty, minus_zero, plus_infty);
  TEST_c_c (casin, minus_zero, minus_infty, minus_zero, minus_infty);
  TEST_c_c (casin, 0.1L, plus_infty, 0.0, plus_infty);
  TEST_c_c (casin, 0.1L, minus_infty, 0.0, minus_infty);

  TEST_c_c (casin, minus_infty, 0, -M_PI_2l, plus_infty);
  TEST_c_c (casin, minus_infty, minus_zero, -M_PI_2l, minus_infty);
  TEST_c_c (casin, minus_infty, 100, -M_PI_2l, plus_infty);
  TEST_c_c (casin, minus_infty, -100, -M_PI_2l, minus_infty);

  TEST_c_c (casin, plus_infty, 0, M_PI_2l, plus_infty);
  TEST_c_c (casin, plus_infty, minus_zero, M_PI_2l, minus_infty);
  TEST_c_c (casin, plus_infty, 0.5, M_PI_2l, plus_infty);
  TEST_c_c (casin, plus_infty, -0.5, M_PI_2l, minus_infty);

  TEST_c_c (casin, nan_value, plus_infty, nan_value, plus_infty);
  TEST_c_c (casin, nan_value, minus_infty, nan_value, minus_infty);

  TEST_c_c (casin, 0.0, nan_value, 0.0, nan_value);
  TEST_c_c (casin, minus_zero, nan_value, minus_zero, nan_value);

  TEST_c_c (casin, plus_infty, nan_value, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (casin, minus_infty, nan_value, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (casin, nan_value, 10.5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (casin, nan_value, -10.5, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (casin, 0.75, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (casin, -0.75, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (casin, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (casin, 0.7L, 1.2L, 0.4356135790797415103321208644578462L, 1.0927647857577371459105272080819308L);
  TEST_c_c (casin, -2, -3, -0.57065278432109940071028387968566963L, -1.9833870299165354323470769028940395L);

  END (casin, complex);
}


static void
casinh_test (void)
{
  errno = 0;
  FUNC(casinh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (casinh);

  TEST_c_c (casinh, 0, 0, 0.0, 0.0);
  TEST_c_c (casinh, minus_zero, 0, minus_zero, 0);
  TEST_c_c (casinh, 0, minus_zero, 0.0, minus_zero);
  TEST_c_c (casinh, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (casinh, plus_infty, plus_infty, plus_infty, M_PI_4l);
  TEST_c_c (casinh, plus_infty, minus_infty, plus_infty, -M_PI_4l);
  TEST_c_c (casinh, minus_infty, plus_infty, minus_infty, M_PI_4l);
  TEST_c_c (casinh, minus_infty, minus_infty, minus_infty, -M_PI_4l);

  TEST_c_c (casinh, -10.0, plus_infty, minus_infty, M_PI_2l);
  TEST_c_c (casinh, -10.0, minus_infty, minus_infty, -M_PI_2l);
  TEST_c_c (casinh, 0, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (casinh, 0, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (casinh, minus_zero, plus_infty, minus_infty, M_PI_2l);
  TEST_c_c (casinh, minus_zero, minus_infty, minus_infty, -M_PI_2l);
  TEST_c_c (casinh, 0.1L, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (casinh, 0.1L, minus_infty, plus_infty, -M_PI_2l);

  TEST_c_c (casinh, minus_infty, 0, minus_infty, 0.0);
  TEST_c_c (casinh, minus_infty, minus_zero, minus_infty, minus_zero);
  TEST_c_c (casinh, minus_infty, 100, minus_infty, 0.0);
  TEST_c_c (casinh, minus_infty, -100, minus_infty, minus_zero);

  TEST_c_c (casinh, plus_infty, 0, plus_infty, 0.0);
  TEST_c_c (casinh, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (casinh, plus_infty, 0.5, plus_infty, 0.0);
  TEST_c_c (casinh, plus_infty, -0.5, plus_infty, minus_zero);

  TEST_c_c (casinh, plus_infty, nan_value, plus_infty, nan_value);
  TEST_c_c (casinh, minus_infty, nan_value, minus_infty, nan_value);

  TEST_c_c (casinh, nan_value, 0, nan_value, 0.0);
  TEST_c_c (casinh, nan_value, minus_zero, nan_value, minus_zero);

  TEST_c_c (casinh, nan_value, plus_infty, plus_infty, nan_value, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (casinh, nan_value, minus_infty, plus_infty, nan_value, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (casinh, 10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (casinh, -10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (casinh, nan_value, 0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (casinh, -0.75, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (casinh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (casinh, 0.7L, 1.2L, 0.97865459559367387689317593222160964L, 0.91135418953156011567903546856170941L);
  TEST_c_c (casinh, -2, -3, -1.9686379257930962917886650952454982L, -0.96465850440760279204541105949953237L);

  END (casinh, complex);
}


static void
catan_test (void)
{
  errno = 0;
  FUNC(catan) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (catan);

  TEST_c_c (catan, 0, 0, 0, 0);
  TEST_c_c (catan, minus_zero, 0, minus_zero, 0);
  TEST_c_c (catan, 0, minus_zero, 0, minus_zero);
  TEST_c_c (catan, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (catan, plus_infty, plus_infty, M_PI_2l, 0);
  TEST_c_c (catan, plus_infty, minus_infty, M_PI_2l, minus_zero);
  TEST_c_c (catan, minus_infty, plus_infty, -M_PI_2l, 0);
  TEST_c_c (catan, minus_infty, minus_infty, -M_PI_2l, minus_zero);


  TEST_c_c (catan, plus_infty, -10.0, M_PI_2l, minus_zero);
  TEST_c_c (catan, minus_infty, -10.0, -M_PI_2l, minus_zero);
  TEST_c_c (catan, plus_infty, minus_zero, M_PI_2l, minus_zero);
  TEST_c_c (catan, minus_infty, minus_zero, -M_PI_2l, minus_zero);
  TEST_c_c (catan, plus_infty, 0.0, M_PI_2l, 0);
  TEST_c_c (catan, minus_infty, 0.0, -M_PI_2l, 0);
  TEST_c_c (catan, plus_infty, 0.1L, M_PI_2l, 0);
  TEST_c_c (catan, minus_infty, 0.1L, -M_PI_2l, 0);

  TEST_c_c (catan, 0.0, minus_infty, M_PI_2l, minus_zero);
  TEST_c_c (catan, minus_zero, minus_infty, -M_PI_2l, minus_zero);
  TEST_c_c (catan, 100.0, minus_infty, M_PI_2l, minus_zero);
  TEST_c_c (catan, -100.0, minus_infty, -M_PI_2l, minus_zero);

  TEST_c_c (catan, 0.0, plus_infty, M_PI_2l, 0);
  TEST_c_c (catan, minus_zero, plus_infty, -M_PI_2l, 0);
  TEST_c_c (catan, 0.5, plus_infty, M_PI_2l, 0);
  TEST_c_c (catan, -0.5, plus_infty, -M_PI_2l, 0);

  TEST_c_c (catan, nan_value, 0.0, nan_value, 0);
  TEST_c_c (catan, nan_value, minus_zero, nan_value, minus_zero);

  TEST_c_c (catan, nan_value, plus_infty, nan_value, 0);
  TEST_c_c (catan, nan_value, minus_infty, nan_value, minus_zero);

  TEST_c_c (catan, 0.0, nan_value, nan_value, nan_value);
  TEST_c_c (catan, minus_zero, nan_value, nan_value, nan_value);

  TEST_c_c (catan, plus_infty, nan_value, M_PI_2l, 0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (catan, minus_infty, nan_value, -M_PI_2l, 0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (catan, nan_value, 10.5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (catan, nan_value, -10.5, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (catan, 0.75, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (catan, -0.75, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (catan, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (catan, 0.7L, 1.2L, 1.0785743834118921877443707996386368L, 0.57705737765343067644394541889341712L);

  TEST_c_c (catan, -2, -3, -1.4099210495965755225306193844604208L, -0.22907268296853876629588180294200276L);

  END (catan, complex);
}

static void
catanh_test (void)
{
  errno = 0;
  FUNC(catanh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (catanh);

  TEST_c_c (catanh, 0, 0, 0.0, 0.0);
  TEST_c_c (catanh, minus_zero, 0, minus_zero, 0.0);
  TEST_c_c (catanh, 0, minus_zero, 0.0, minus_zero);
  TEST_c_c (catanh, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (catanh, plus_infty, plus_infty, 0.0, M_PI_2l);
  TEST_c_c (catanh, plus_infty, minus_infty, 0.0, -M_PI_2l);
  TEST_c_c (catanh, minus_infty, plus_infty, minus_zero, M_PI_2l);
  TEST_c_c (catanh, minus_infty, minus_infty, minus_zero, -M_PI_2l);

  TEST_c_c (catanh, -10.0, plus_infty, minus_zero, M_PI_2l);
  TEST_c_c (catanh, -10.0, minus_infty, minus_zero, -M_PI_2l);
  TEST_c_c (catanh, minus_zero, plus_infty, minus_zero, M_PI_2l);
  TEST_c_c (catanh, minus_zero, minus_infty, minus_zero, -M_PI_2l);
  TEST_c_c (catanh, 0, plus_infty, 0.0, M_PI_2l);
  TEST_c_c (catanh, 0, minus_infty, 0.0, -M_PI_2l);
  TEST_c_c (catanh, 0.1L, plus_infty, 0.0, M_PI_2l);
  TEST_c_c (catanh, 0.1L, minus_infty, 0.0, -M_PI_2l);

  TEST_c_c (catanh, minus_infty, 0, minus_zero, M_PI_2l);
  TEST_c_c (catanh, minus_infty, minus_zero, minus_zero, -M_PI_2l);
  TEST_c_c (catanh, minus_infty, 100, minus_zero, M_PI_2l);
  TEST_c_c (catanh, minus_infty, -100, minus_zero, -M_PI_2l);

  TEST_c_c (catanh, plus_infty, 0, 0.0, M_PI_2l);
  TEST_c_c (catanh, plus_infty, minus_zero, 0.0, -M_PI_2l);
  TEST_c_c (catanh, plus_infty, 0.5, 0.0, M_PI_2l);
  TEST_c_c (catanh, plus_infty, -0.5, 0.0, -M_PI_2l);

  TEST_c_c (catanh, 0, nan_value, 0.0, nan_value);
  TEST_c_c (catanh, minus_zero, nan_value, minus_zero, nan_value);

  TEST_c_c (catanh, plus_infty, nan_value, 0.0, nan_value);
  TEST_c_c (catanh, minus_infty, nan_value, minus_zero, nan_value);

  TEST_c_c (catanh, nan_value, 0, nan_value, nan_value);
  TEST_c_c (catanh, nan_value, minus_zero, nan_value, nan_value);

  TEST_c_c (catanh, nan_value, plus_infty, 0.0, M_PI_2l, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (catanh, nan_value, minus_infty, 0.0, -M_PI_2l, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (catanh, 10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (catanh, -10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (catanh, nan_value, 0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (catanh, nan_value, -0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (catanh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (catanh, 0.7L, 1.2L, 0.2600749516525135959200648705635915L, 0.97024030779509898497385130162655963L);
  TEST_c_c (catanh, -2, -3, -0.14694666622552975204743278515471595L, -1.3389725222944935611241935759091443L);

  END (catanh, complex);
}
#endif

static void
cbrt_test (void)
{
  errno = 0;
  FUNC(cbrt) (8);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cbrt);

  TEST_f_f (cbrt, 0.0, 0.0);
  TEST_f_f (cbrt, minus_zero, minus_zero);

  TEST_f_f (cbrt, plus_infty, plus_infty);
  TEST_f_f (cbrt, minus_infty, minus_infty);
  TEST_f_f (cbrt, nan_value, nan_value);

  TEST_f_f (cbrt, -0.001L, -0.1L);
  TEST_f_f (cbrt, 8, 2);
  TEST_f_f (cbrt, -27.0, -3.0);
  TEST_f_f (cbrt, 0.970299L, 0.99L);
  TEST_f_f (cbrt, 0.7L, 0.8879040017426007084L);

  END (cbrt);
}

#if 0
static void
ccos_test (void)
{
  errno = 0;
  FUNC(ccos) (BUILD_COMPLEX (0, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ccos);

  TEST_c_c (ccos, 0.0, 0.0, 1.0, minus_zero);
  TEST_c_c (ccos, minus_zero, 0.0, 1.0, 0.0);
  TEST_c_c (ccos, 0.0, minus_zero, 1.0, 0.0);
  TEST_c_c (ccos, minus_zero, minus_zero, 1.0, minus_zero);

  TEST_c_c (ccos, plus_infty, 0.0, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccos, plus_infty, minus_zero, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccos, minus_infty, 0.0, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccos, minus_infty, minus_zero, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccos, 0.0, plus_infty, plus_infty, minus_zero);
  TEST_c_c (ccos, 0.0, minus_infty, plus_infty, 0.0);
  TEST_c_c (ccos, minus_zero, plus_infty, plus_infty, 0.0);
  TEST_c_c (ccos, minus_zero, minus_infty, plus_infty, minus_zero);

  TEST_c_c (ccos, plus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, minus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, plus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, minus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ccos, 4.625, plus_infty, minus_infty, plus_infty);
  TEST_c_c (ccos, 4.625, minus_infty, minus_infty, minus_infty);
  TEST_c_c (ccos, -4.625, plus_infty, minus_infty, minus_infty);
  TEST_c_c (ccos, -4.625, minus_infty, minus_infty, plus_infty);

  TEST_c_c (ccos, plus_infty, 6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, plus_infty, -6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, minus_infty, 6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, minus_infty, -6.75, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ccos, nan_value, 0.0, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccos, nan_value, minus_zero, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccos, nan_value, plus_infty, plus_infty, nan_value);
  TEST_c_c (ccos, nan_value, minus_infty, plus_infty, nan_value);

  TEST_c_c (ccos, nan_value, 9.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccos, nan_value, -9.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccos, 0.0, nan_value, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccos, minus_zero, nan_value, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccos, 10.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccos, -10.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccos, plus_infty, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccos, minus_infty, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccos, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (ccos, 0.7L, 1.2L, 1.3848657645312111080L, -0.97242170335830028619L);

  TEST_c_c (ccos, -2, -3, -4.1896256909688072301L, -9.1092278937553365979L);

  END (ccos, complex);
}


static void
ccosh_test (void)
{
  errno = 0;
  FUNC(ccosh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ccosh);

  TEST_c_c (ccosh, 0.0, 0.0, 1.0, 0.0);
  TEST_c_c (ccosh, minus_zero, 0.0, 1.0, minus_zero);
  TEST_c_c (ccosh, 0.0, minus_zero, 1.0, minus_zero);
  TEST_c_c (ccosh, minus_zero, minus_zero, 1.0, 0.0);

  TEST_c_c (ccosh, 0.0, plus_infty, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccosh, minus_zero, plus_infty, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccosh, 0.0, minus_infty, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccosh, minus_zero, minus_infty, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccosh, plus_infty, 0.0, plus_infty, 0.0);
  TEST_c_c (ccosh, minus_infty, 0.0, plus_infty, minus_zero);
  TEST_c_c (ccosh, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (ccosh, minus_infty, minus_zero, plus_infty, 0.0);

  TEST_c_c (ccosh, plus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, minus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, plus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, minus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ccosh, plus_infty, 4.625, minus_infty, minus_infty);
  TEST_c_c (ccosh, minus_infty, 4.625, minus_infty, plus_infty);
  TEST_c_c (ccosh, plus_infty, -4.625, minus_infty, plus_infty);
  TEST_c_c (ccosh, minus_infty, -4.625, minus_infty, minus_infty);

  TEST_c_c (ccosh, 6.75, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, -6.75, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, 6.75, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, -6.75, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ccosh, 0.0, nan_value, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccosh, minus_zero, nan_value, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccosh, plus_infty, nan_value, plus_infty, nan_value);
  TEST_c_c (ccosh, minus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (ccosh, 9.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccosh, -9.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccosh, nan_value, 0.0, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccosh, nan_value, minus_zero, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccosh, nan_value, 10.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccosh, nan_value, -10.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccosh, nan_value, plus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccosh, nan_value, minus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccosh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (ccosh, 0.7L, 1.2L, 0.4548202223691477654L, 0.7070296600921537682L);

  TEST_c_c (ccosh, -2, -3, -3.7245455049153225654L, 0.5118225699873846088L);

  END (ccosh, complex);
}
#endif

static void
ceil_test (void)
{
  START (ceil);

  TEST_f_f (ceil, 0.0, 0.0);
  TEST_f_f (ceil, minus_zero, minus_zero);
  TEST_f_f (ceil, plus_infty, plus_infty);
  TEST_f_f (ceil, minus_infty, minus_infty);
  TEST_f_f (ceil, nan_value, nan_value);

  TEST_f_f (ceil, M_PIl, 4.0);
  TEST_f_f (ceil, -M_PIl, -3.0);

  END (ceil);
}

#if 0
static void
cexp_test (void)
{
  errno = 0;
  FUNC(cexp) (BUILD_COMPLEX (0, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cexp);

  TEST_c_c (cexp, plus_zero, plus_zero, 1, 0.0);
  TEST_c_c (cexp, minus_zero, plus_zero, 1, 0.0);
  TEST_c_c (cexp, plus_zero, minus_zero, 1, minus_zero);
  TEST_c_c (cexp, minus_zero, minus_zero, 1, minus_zero);

  TEST_c_c (cexp, plus_infty, plus_zero, plus_infty, 0.0);
  TEST_c_c (cexp, plus_infty, minus_zero, plus_infty, minus_zero);

  TEST_c_c (cexp, minus_infty, plus_zero, 0.0, 0.0);
  TEST_c_c (cexp, minus_infty, minus_zero, 0.0, minus_zero);

  TEST_c_c (cexp, 0.0, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (cexp, minus_zero, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (cexp, 0.0, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (cexp, minus_zero, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (cexp, 100.0, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (cexp, -100.0, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (cexp, 100.0, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (cexp, -100.0, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (cexp, minus_infty, 2.0, minus_zero, 0.0);
  TEST_c_c (cexp, minus_infty, 4.0, minus_zero, minus_zero);
  TEST_c_c (cexp, plus_infty, 2.0, minus_infty, plus_infty);
  TEST_c_c (cexp, plus_infty, 4.0, minus_infty, minus_infty);

  TEST_c_c (cexp, plus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (cexp, plus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (cexp, minus_infty, plus_infty, 0.0, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (cexp, minus_infty, minus_infty, 0.0, minus_zero, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (cexp, minus_infty, nan_value, 0, 0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (cexp, plus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (cexp, nan_value, 0.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cexp, nan_value, 1.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (cexp, nan_value, plus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cexp, 0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cexp, 1, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cexp, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (cexp, 0.7L, 1.2L, 0.72969890915032360123451688642930727L, 1.8768962328348102821139467908203072L);
  TEST_c_c (cexp, -2.0, -3.0, -0.13398091492954261346140525546115575L, -0.019098516261135196432576240858800925L);

  END (cexp, complex);
}

static void
cimag_test (void)
{
  START (cimag);
  TEST_c_f (cimag, 1.0, 0.0, 0.0);
  TEST_c_f (cimag, 1.0, minus_zero, minus_zero);
  TEST_c_f (cimag, 1.0, nan_value, nan_value);
  TEST_c_f (cimag, nan_value, nan_value, nan_value);
  TEST_c_f (cimag, 1.0, plus_infty, plus_infty);
  TEST_c_f (cimag, 1.0, minus_infty, minus_infty);
  TEST_c_f (cimag, 2.0, 3.0, 3.0);

  END (cimag);
}

static void
clog_test (void)
{
  errno = 0;
  FUNC(clog) (BUILD_COMPLEX (-2, -3));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (clog);

  TEST_c_c (clog, minus_zero, 0, minus_infty, M_PIl, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_c_c (clog, minus_zero, minus_zero, minus_infty, -M_PIl, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_c_c (clog, 0, 0, minus_infty, 0.0, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_c_c (clog, 0, minus_zero, minus_infty, minus_zero, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_c_c (clog, minus_infty, plus_infty, plus_infty, M_PI_34l);
  TEST_c_c (clog, minus_infty, minus_infty, plus_infty, -M_PI_34l);

  TEST_c_c (clog, plus_infty, plus_infty, plus_infty, M_PI_4l);
  TEST_c_c (clog, plus_infty, minus_infty, plus_infty, -M_PI_4l);

  TEST_c_c (clog, 0, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (clog, 3, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (clog, minus_zero, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (clog, -3, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (clog, 0, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (clog, 3, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (clog, minus_zero, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (clog, -3, minus_infty, plus_infty, -M_PI_2l);

  TEST_c_c (clog, minus_infty, 0, plus_infty, M_PIl);
  TEST_c_c (clog, minus_infty, 1, plus_infty, M_PIl);
  TEST_c_c (clog, minus_infty, minus_zero, plus_infty, -M_PIl);
  TEST_c_c (clog, minus_infty, -1, plus_infty, -M_PIl);

  TEST_c_c (clog, plus_infty, 0, plus_infty, 0.0);
  TEST_c_c (clog, plus_infty, 1, plus_infty, 0.0);
  TEST_c_c (clog, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (clog, plus_infty, -1, plus_infty, minus_zero);

  TEST_c_c (clog, plus_infty, nan_value, plus_infty, nan_value);
  TEST_c_c (clog, minus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (clog, nan_value, plus_infty, plus_infty, nan_value);
  TEST_c_c (clog, nan_value, minus_infty, plus_infty, nan_value);

  TEST_c_c (clog, 0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, 3, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, minus_zero, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, -3, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (clog, nan_value, 0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, nan_value, 5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, nan_value, minus_zero, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, nan_value, -5, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (clog, nan_value, nan_value, nan_value, nan_value);
  TEST_c_c (clog, -2, -3, 1.2824746787307683680267437207826593L, -2.1587989303424641704769327722648368L);

  END (clog, complex);
}


static void
clog10_test (void)
{
  errno = 0;
  FUNC(clog10) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (clog10);

  TEST_c_c (clog10, minus_zero, 0, minus_infty, M_PIl, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_c_c (clog10, minus_zero, minus_zero, minus_infty, -M_PIl, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_c_c (clog10, 0, 0, minus_infty, 0.0, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_c_c (clog10, 0, minus_zero, minus_infty, minus_zero, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_c_c (clog10, minus_infty, plus_infty, plus_infty, M_PI_34_LOG10El);

  TEST_c_c (clog10, plus_infty, plus_infty, plus_infty, M_PI4_LOG10El);
  TEST_c_c (clog10, plus_infty, minus_infty, plus_infty, -M_PI4_LOG10El);

  TEST_c_c (clog10, 0, plus_infty, plus_infty, M_PI2_LOG10El);
  TEST_c_c (clog10, 3, plus_infty, plus_infty, M_PI2_LOG10El);
  TEST_c_c (clog10, minus_zero, plus_infty, plus_infty, M_PI2_LOG10El);
  TEST_c_c (clog10, -3, plus_infty, plus_infty, M_PI2_LOG10El);
  TEST_c_c (clog10, 0, minus_infty, plus_infty, -M_PI2_LOG10El);
  TEST_c_c (clog10, 3, minus_infty, plus_infty, -M_PI2_LOG10El);
  TEST_c_c (clog10, minus_zero, minus_infty, plus_infty, -M_PI2_LOG10El);
  TEST_c_c (clog10, -3, minus_infty, plus_infty, -M_PI2_LOG10El);

  TEST_c_c (clog10, minus_infty, 0, plus_infty, M_PI_LOG10El);
  TEST_c_c (clog10, minus_infty, 1, plus_infty, M_PI_LOG10El);
  TEST_c_c (clog10, minus_infty, minus_zero, plus_infty, -M_PI_LOG10El);
  TEST_c_c (clog10, minus_infty, -1, plus_infty, -M_PI_LOG10El);

  TEST_c_c (clog10, plus_infty, 0, plus_infty, 0.0);
  TEST_c_c (clog10, plus_infty, 1, plus_infty, 0.0);
  TEST_c_c (clog10, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (clog10, plus_infty, -1, plus_infty, minus_zero);

  TEST_c_c (clog10, plus_infty, nan_value, plus_infty, nan_value);
  TEST_c_c (clog10, minus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (clog10, nan_value, plus_infty, plus_infty, nan_value);
  TEST_c_c (clog10, nan_value, minus_infty, plus_infty, nan_value);

  TEST_c_c (clog10, 0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, 3, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, minus_zero, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, -3, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (clog10, nan_value, 0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, nan_value, 5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, nan_value, minus_zero, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, nan_value, -5, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (clog10, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (clog10, 0.7L, 1.2L, 0.1427786545038868803L, 0.4528483579352493248L);
  TEST_c_c (clog10, -2, -3, 0.5569716761534183846L, -0.9375544629863747085L);

  END (clog10, complex);
}
#endif

static void
conj_test (void)
{
  START (conj);
  TEST_c_c (conj, 0.0, 0.0, 0.0, minus_zero);
  TEST_c_c (conj, 0.0, minus_zero, 0.0, 0.0);
  TEST_c_c (conj, nan_value, nan_value, nan_value, nan_value);
  TEST_c_c (conj, plus_infty, minus_infty, plus_infty, plus_infty);
  TEST_c_c (conj, plus_infty, plus_infty, plus_infty, minus_infty);
  TEST_c_c (conj, 1.0, 2.0, 1.0, -2.0);
  TEST_c_c (conj, 3.0, -4.0, 3.0, 4.0);

  END (conj, complex);
}


static void
copysign_test (void)
{
  START (copysign);

  TEST_ff_f (copysign, 0, 4, 0);
  TEST_ff_f (copysign, 0, -4, minus_zero);
  TEST_ff_f (copysign, minus_zero, 4, 0);
  TEST_ff_f (copysign, minus_zero, -4, minus_zero);

  TEST_ff_f (copysign, plus_infty, 0, plus_infty);
  TEST_ff_f (copysign, plus_infty, minus_zero, minus_infty);
  TEST_ff_f (copysign, minus_infty, 0, plus_infty);
  TEST_ff_f (copysign, minus_infty, minus_zero, minus_infty);

  TEST_ff_f (copysign, 0, plus_infty, 0);
  TEST_ff_f (copysign, 0, minus_zero, minus_zero);
  TEST_ff_f (copysign, minus_zero, plus_infty, 0);
  TEST_ff_f (copysign, minus_zero, minus_zero, minus_zero);

  /* XXX More correctly we would have to check the sign of the NaN.  */
  TEST_ff_f (copysign, nan_value, 0, nan_value);
  TEST_ff_f (copysign, nan_value, minus_zero, nan_value);
  TEST_ff_f (copysign, -nan_value, 0, nan_value);
  TEST_ff_f (copysign, -nan_value, minus_zero, nan_value);

  END (copysign);
}

static void
cos_test (void)
{
  errno = 0;
  FUNC(cos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cos);

  TEST_f_f (cos, 0, 1);
  TEST_f_f (cos, minus_zero, 1);
  TEST_f_f (cos, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (cos, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (cos, nan_value, nan_value);

  TEST_f_f (cos, M_PI_6l * 2.0, 0.5);
  TEST_f_f (cos, M_PI_6l * 4.0, -0.5);
  TEST_f_f (cos, M_PI_2l, 0);

  TEST_f_f (cos, 0.7L, 0.76484218728448842625585999019186495L);

  END (cos);
}

static void
cosh_test (void)
{
  errno = 0;
  FUNC(cosh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cosh);
  TEST_f_f (cosh, 0, 1);
  TEST_f_f (cosh, minus_zero, 1);

#ifndef TEST_INLINE
  TEST_f_f (cosh, plus_infty, plus_infty);
  TEST_f_f (cosh, minus_infty, plus_infty);
#endif
  TEST_f_f (cosh, nan_value, nan_value);

  TEST_f_f (cosh, 0.7L, 1.255169005630943018L);
  END (cosh);
}

#if 0
static void
cpow_test (void)
{
  errno = 0;
  FUNC(cpow) (BUILD_COMPLEX (1, 0), BUILD_COMPLEX (0, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cpow);

  TEST_cc_c (cpow, 1, 0, 0, 0, 1.0, 0.0);
  TEST_cc_c (cpow, 2, 0, 10, 0, 1024.0, 0.0);

  TEST_cc_c (cpow, M_El, 0, 0, 2 * M_PIl, 1.0, 0.0);
  TEST_cc_c (cpow, 2, 3, 4, 0, -119.0, -120.0);

  TEST_cc_c (cpow, nan_value, nan_value, nan_value, nan_value, nan_value, nan_value);

  END (cpow, complex);
}

static void
cproj_test (void)
{
  START (cproj);
  TEST_c_c (cproj, 0.0, 0.0, 0.0, 0.0);
  TEST_c_c (cproj, minus_zero, minus_zero, minus_zero, minus_zero);
  TEST_c_c (cproj, 0.0, minus_zero, 0.0, minus_zero);
  TEST_c_c (cproj, minus_zero, 0.0, minus_zero, 0.0);

  TEST_c_c (cproj, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (cproj, plus_infty, plus_infty, plus_infty, 0.0);
  TEST_c_c (cproj, plus_infty, minus_infty, plus_infty, minus_zero);
  TEST_c_c (cproj, minus_infty, plus_infty, plus_infty, 0.0);
  TEST_c_c (cproj, minus_infty, minus_infty, plus_infty, minus_zero);

  TEST_c_c (cproj, 1.0, 0.0, 1.0, 0.0);
  TEST_c_c (cproj, 2.0, 3.0, 0.2857142857142857142857142857142857L, 0.42857142857142857142857142857142855L);

  END (cproj, complex);
}

static void
creal_test (void)
{
  START (creal);
  TEST_c_f (creal, 0.0, 1.0, 0.0);
  TEST_c_f (creal, minus_zero, 1.0, minus_zero);
  TEST_c_f (creal, nan_value, 1.0, nan_value);
  TEST_c_f (creal, nan_value, nan_value, nan_value);
  TEST_c_f (creal, plus_infty, 1.0, plus_infty);
  TEST_c_f (creal, minus_infty, 1.0, minus_infty);
  TEST_c_f (creal, 2.0, 3.0, 2.0);

  END (creal);
}

static void
csin_test (void)
{
  errno = 0;
  FUNC(csin) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (csin);

  TEST_c_c (csin, 0.0, 0.0, 0.0, 0.0);
  TEST_c_c (csin, minus_zero, 0.0, minus_zero, 0.0);
  TEST_c_c (csin, 0.0, minus_zero, 0, minus_zero);
  TEST_c_c (csin, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (csin, 0.0, plus_infty, 0.0, plus_infty);
  TEST_c_c (csin, minus_zero, plus_infty, minus_zero, plus_infty);
  TEST_c_c (csin, 0.0, minus_infty, 0.0, minus_infty);
  TEST_c_c (csin, minus_zero, minus_infty, minus_zero, minus_infty);

  TEST_c_c (csin, plus_infty, 0.0, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, minus_infty, 0.0, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, plus_infty, minus_zero, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, minus_infty, minus_zero, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csin, plus_infty, plus_infty, nan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, minus_infty, plus_infty, nan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, plus_infty, minus_infty, nan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, minus_infty, minus_infty, nan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csin, plus_infty, 6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csin, plus_infty, -6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csin, minus_infty, 6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csin, minus_infty, -6.75,  nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (csin, 4.625, plus_infty, minus_infty, minus_infty);
  TEST_c_c (csin, 4.625, minus_infty, minus_infty, plus_infty);
  TEST_c_c (csin, -4.625, plus_infty, plus_infty, minus_infty);
  TEST_c_c (csin, -4.625, minus_infty, plus_infty, plus_infty);

  TEST_c_c (csin, nan_value, 0.0, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, nan_value, minus_zero, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csin, nan_value, plus_infty, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, nan_value, minus_infty, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csin, nan_value, 9.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csin, nan_value, -9.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csin, 0.0, nan_value, 0.0, nan_value);
  TEST_c_c (csin, minus_zero, nan_value, minus_zero, nan_value);

  TEST_c_c (csin, 10.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csin, nan_value, -10.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csin, plus_infty, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csin, minus_infty, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csin, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (csin, 0.7L, 1.2L, 1.1664563419657581376L, 1.1544997246948547371L);

  TEST_c_c (csin, -2, -3, -9.1544991469114295734L, 4.1689069599665643507L);

  END (csin, complex);
}


static void
csinh_test (void)
{
  errno = 0;
  FUNC(csinh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (csinh);

  TEST_c_c (csinh, 0.0, 0.0, 0.0, 0.0);
  TEST_c_c (csinh, minus_zero, 0.0, minus_zero, 0.0);
  TEST_c_c (csinh, 0.0, minus_zero, 0.0, minus_zero);
  TEST_c_c (csinh, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (csinh, 0.0, plus_infty, 0.0, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_zero, plus_infty, 0.0, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, 0.0, minus_infty, 0.0, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_zero, minus_infty, 0.0, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csinh, plus_infty, 0.0, plus_infty, 0.0);
  TEST_c_c (csinh, minus_infty, 0.0, minus_infty, 0.0);
  TEST_c_c (csinh, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (csinh, minus_infty, minus_zero, minus_infty, minus_zero);

  TEST_c_c (csinh, plus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, plus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csinh, plus_infty, 4.625, minus_infty, minus_infty);
  TEST_c_c (csinh, minus_infty, 4.625, plus_infty, minus_infty);
  TEST_c_c (csinh, plus_infty, -4.625, minus_infty, plus_infty);
  TEST_c_c (csinh, minus_infty, -4.625, plus_infty, plus_infty);

  TEST_c_c (csinh, 6.75, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csinh, -6.75, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csinh, 6.75, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csinh, -6.75, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (csinh, 0.0, nan_value, 0.0, nan_value, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_zero, nan_value, 0.0, nan_value, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csinh, plus_infty, nan_value, plus_infty, nan_value, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_infty, nan_value, plus_infty, nan_value, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csinh, 9.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csinh, -9.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csinh, nan_value, 0.0, nan_value, 0.0);
  TEST_c_c (csinh, nan_value, minus_zero, nan_value, minus_zero);

  TEST_c_c (csinh, nan_value, 10.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csinh, nan_value, -10.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csinh, nan_value, plus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csinh, nan_value, minus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csinh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (csinh, 0.7L, 1.2L, 0.27487868678117583582L, 1.1698665727426565139L);
  TEST_c_c (csinh, -2, -3, 3.5905645899857799520L, -0.5309210862485198052L);

  END (csinh, complex);
}

static void
csqrt_test (void)
{
  errno = 0;
  FUNC(csqrt) (BUILD_COMPLEX (-1, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (csqrt);

  TEST_c_c (csqrt, 0, 0, 0.0, 0.0);
  TEST_c_c (csqrt, 0, minus_zero, 0, minus_zero);
  TEST_c_c (csqrt, minus_zero, 0, 0.0, 0.0);
  TEST_c_c (csqrt, minus_zero, minus_zero, 0.0, minus_zero);

  TEST_c_c (csqrt, minus_infty, 0, 0.0, plus_infty);
  TEST_c_c (csqrt, minus_infty, 6, 0.0, plus_infty);
  TEST_c_c (csqrt, minus_infty, minus_zero, 0.0, minus_infty);
  TEST_c_c (csqrt, minus_infty, -6, 0.0, minus_infty);

  TEST_c_c (csqrt, plus_infty, 0, plus_infty, 0.0);
  TEST_c_c (csqrt, plus_infty, 6, plus_infty, 0.0);
  TEST_c_c (csqrt, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (csqrt, plus_infty, -6, plus_infty, minus_zero);

  TEST_c_c (csqrt, 0, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, 4, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, plus_infty, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, minus_zero, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, -4, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, minus_infty, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, 0, minus_infty, plus_infty, minus_infty);
  TEST_c_c (csqrt, 4, minus_infty, plus_infty, minus_infty);
  TEST_c_c (csqrt, plus_infty, minus_infty, plus_infty, minus_infty);
  TEST_c_c (csqrt, minus_zero, minus_infty, plus_infty, minus_infty);
  TEST_c_c (csqrt, -4, minus_infty, plus_infty, minus_infty);
  TEST_c_c (csqrt, minus_infty, minus_infty, plus_infty, minus_infty);

  TEST_c_c (csqrt, minus_infty, nan_value, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csqrt, plus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (csqrt, 0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, 1, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, minus_zero, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, -1, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csqrt, nan_value, 0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, nan_value, 8, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, nan_value, minus_zero, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, nan_value, -8, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csqrt, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (csqrt, 16.0, -30.0, 5.0, -3.0);
  TEST_c_c (csqrt, -1, 0, 0.0, 1.0);
  TEST_c_c (csqrt, 0, 2, 1.0, 1.0);
  TEST_c_c (csqrt, 119, 120, 12.0, 5.0);
  TEST_c_c (csqrt, 0.7L, 1.2L, 1.022067610030026450706487883081139L, 0.58704531296356521154977678719838035L);
  TEST_c_c (csqrt, -2, -3, 0.89597747612983812471573375529004348L, -1.6741492280355400404480393008490519L);
  TEST_c_c (csqrt, -2, 3, 0.89597747612983812471573375529004348L, 1.6741492280355400404480393008490519L);

  END (csqrt, complex);
}

static void
ctan_test (void)
{
  errno = 0;
  FUNC(ctan) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctan);

  TEST_c_c (ctan, 0, 0, 0.0, 0.0);
  TEST_c_c (ctan, 0, minus_zero, 0.0, minus_zero);
  TEST_c_c (ctan, minus_zero, 0, minus_zero, 0.0);
  TEST_c_c (ctan, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (ctan, 0, plus_infty, 0.0, 1.0);
  TEST_c_c (ctan, 1, plus_infty, 0.0, 1.0);
  TEST_c_c (ctan, minus_zero, plus_infty, minus_zero, 1.0);
  TEST_c_c (ctan, -1, plus_infty, minus_zero, 1.0);

  TEST_c_c (ctan, 0, minus_infty, 0.0, -1.0);
  TEST_c_c (ctan, 1, minus_infty, 0.0, -1.0);
  TEST_c_c (ctan, minus_zero, minus_infty, minus_zero, -1.0);
  TEST_c_c (ctan, -1, minus_infty, minus_zero, -1.0);

  TEST_c_c (ctan, plus_infty, 0, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, plus_infty, 2, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, minus_infty, 0, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, minus_infty, 2, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, plus_infty, minus_zero, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, plus_infty, -2, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, minus_infty, minus_zero, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, minus_infty, -2, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ctan, nan_value, plus_infty, 0.0, 1.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ctan, nan_value, minus_infty, 0.0, -1.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ctan, 0, nan_value, 0.0, nan_value);
  TEST_c_c (ctan, minus_zero, nan_value, minus_zero, nan_value);

  TEST_c_c (ctan, 0.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctan, -4.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ctan, nan_value, 0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctan, nan_value, 5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctan, nan_value, minus_zero, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctan, nan_value, -0.25, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ctan, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (ctan, 0.7L, 1.2L, 0.1720734197630349001L, 0.9544807059989405538L);
  TEST_c_c (ctan, -2, -3, 0.0037640256415042482L, -1.0032386273536098014L);

  END (ctan, complex);
}


static void
ctanh_test (void)
{
  errno = 0;
  FUNC(ctanh) (BUILD_COMPLEX (0, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctanh);

  TEST_c_c (ctanh, 0, 0, 0.0, 0.0);
  TEST_c_c (ctanh, 0, minus_zero, 0.0, minus_zero);
  TEST_c_c (ctanh, minus_zero, 0, minus_zero, 0.0);
  TEST_c_c (ctanh, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (ctanh, plus_infty, 0, 1.0, 0.0);
  TEST_c_c (ctanh, plus_infty, 1, 1.0, 0.0);
  TEST_c_c (ctanh, plus_infty, minus_zero, 1.0, minus_zero);
  TEST_c_c (ctanh, plus_infty, -1, 1.0, minus_zero);
  TEST_c_c (ctanh, minus_infty, 0, -1.0, 0.0);
  TEST_c_c (ctanh, minus_infty, 1, -1.0, 0.0);
  TEST_c_c (ctanh, minus_infty, minus_zero, -1.0, minus_zero);
  TEST_c_c (ctanh, minus_infty, -1, -1.0, minus_zero);

  TEST_c_c (ctanh, 0, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, 2, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, 0, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, 2, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, minus_zero, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, -2, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, minus_zero, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, -2, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ctanh, plus_infty, nan_value, 1.0, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ctanh, minus_infty, nan_value, -1.0, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ctanh, nan_value, 0, nan_value, 0.0);
  TEST_c_c (ctanh, nan_value, minus_zero, nan_value, minus_zero);

  TEST_c_c (ctanh, nan_value, 0.5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctanh, nan_value, -4.5, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ctanh, 0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctanh, 5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctanh, minus_zero, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctanh, -0.25, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ctanh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (ctanh, 0, M_PI_4l, 0.0, 1.0);

  TEST_c_c (ctanh, 0.7L, 1.2L, 1.3472197399061191630L, 0.4778641038326365540L);
  TEST_c_c (ctanh, -2, -3, -0.9653858790221331242L, 0.0098843750383224937L);

  END (ctanh, complex);
}
#endif

static void
erf_test (void)
{
  errno = 0;
  FUNC(erf) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (erf);

  TEST_f_f (erf, 0, 0);
  TEST_f_f (erf, minus_zero, minus_zero);
  TEST_f_f (erf, plus_infty, 1);
  TEST_f_f (erf, minus_infty, -1);
  TEST_f_f (erf, nan_value, nan_value);

  TEST_f_f (erf, 0.7L, 0.67780119383741847297L);

  TEST_f_f (erf, 1.2L, 0.91031397822963538024L);
  TEST_f_f (erf, 2.0, 0.99532226501895273416L);
  TEST_f_f (erf, 4.1L, 0.99999999329997234592L);
  TEST_f_f (erf, 27, 1.0L);

  END (erf);
}


static void
erfc_test (void)
{
  errno = 0;
  FUNC(erfc) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (erfc);

  TEST_f_f (erfc, plus_infty, 0.0);
  TEST_f_f (erfc, minus_infty, 2.0);
  TEST_f_f (erfc, 0.0, 1.0);
  TEST_f_f (erfc, minus_zero, 1.0);
  TEST_f_f (erfc, nan_value, nan_value);

  TEST_f_f (erfc, 0.7L, 0.32219880616258152702L);

  TEST_f_f (erfc, 1.2L, 0.089686021770364619762L);
  TEST_f_f (erfc, 2.0, 0.0046777349810472658379L);
  TEST_f_f (erfc, 4.1L, 0.67000276540848983727e-8L);
  TEST_f_f (erfc, 9, 0.41370317465138102381e-36L);

  END (erfc);
}

static void
exp_test (void)
{
  errno = 0;
  FUNC(exp) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp);

  TEST_f_f (exp, 0, 1);
  TEST_f_f (exp, minus_zero, 1);

#ifndef TEST_INLINE
  TEST_f_f (exp, plus_infty, plus_infty);
  TEST_f_f (exp, minus_infty, 0);
#endif
  TEST_f_f (exp, nan_value, nan_value);
  TEST_f_f (exp, 1, M_El);

  TEST_f_f (exp, 2, M_E2l);
  TEST_f_f (exp, 3, M_E3l);
  TEST_f_f (exp, 0.7L, 2.0137527074704765216L);
  TEST_f_f (exp, 50.0L, 5184705528587072464087.45332293348538L);
#ifdef TEST_LDOUBLE
  /* The result can only be represented in long double.  */
  TEST_f_f (exp, 1000.0L, 0.197007111401704699388887935224332313e435L);
#endif
  END (exp);
}


#if 0
static void
exp10_test (void)
{
  errno = 0;
  FUNC(exp10) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp10);

  TEST_f_f (exp10, 0, 1);
  TEST_f_f (exp10, minus_zero, 1);

  TEST_f_f (exp10, plus_infty, plus_infty);
  TEST_f_f (exp10, minus_infty, 0);
  TEST_f_f (exp10, nan_value, nan_value);
  TEST_f_f (exp10, 3, 1000);
  TEST_f_f (exp10, -1, 0.1L);
  TEST_f_f (exp10, 1e6, plus_infty);
  TEST_f_f (exp10, -1e6, 0);
  TEST_f_f (exp10, 0.7L, 5.0118723362727228500155418688494574L);

  END (exp10);
}

static void
exp2_test (void)
{
  errno = 0;
  FUNC(exp2) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp2);

  TEST_f_f (exp2, 0, 1);
  TEST_f_f (exp2, minus_zero, 1);
  TEST_f_f (exp2, plus_infty, plus_infty);
  TEST_f_f (exp2, minus_infty, 0);
  TEST_f_f (exp2, nan_value, nan_value);

  TEST_f_f (exp2, 10, 1024);
  TEST_f_f (exp2, -1, 0.5);
  TEST_f_f (exp2, 1e6, plus_infty);
  TEST_f_f (exp2, -1e6, 0);
  TEST_f_f (exp2, 0.7L, 1.6245047927124710452L);

  END (exp2);
}
#endif

static void
expm1_test (void)
{
  errno = 0;
  FUNC(expm1) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (expm1);

  TEST_f_f (expm1, 0, 0);
  TEST_f_f (expm1, minus_zero, minus_zero);

#ifndef TEST_INLINE
  TEST_f_f (expm1, plus_infty, plus_infty);
  TEST_f_f (expm1, minus_infty, -1);
#endif
  TEST_f_f (expm1, nan_value, nan_value);

  TEST_f_f (expm1, 1, M_El - 1.0);
  TEST_f_f (expm1, 0.7L, 1.0137527074704765216L);

  END (expm1);
}

static void
fabs_test (void)
{
  START (fabs);

  TEST_f_f (fabs, 0, 0);
  TEST_f_f (fabs, minus_zero, 0);

  TEST_f_f (fabs, plus_infty, plus_infty);
  TEST_f_f (fabs, minus_infty, plus_infty);
  TEST_f_f (fabs, nan_value, nan_value);

  TEST_f_f (fabs, 38.0, 38.0);
  TEST_f_f (fabs, -M_El, M_El);

  END (fabs);
}

#if 0
static void
fdim_test (void)
{
  START (fdim);

  TEST_ff_f (fdim, 0, 0, 0);
  TEST_ff_f (fdim, 9, 0, 9);
  TEST_ff_f (fdim, 0, 9, 0);
  TEST_ff_f (fdim, -9, 0, 0);
  TEST_ff_f (fdim, 0, -9, 9);

  TEST_ff_f (fdim, plus_infty, 9, plus_infty);
  TEST_ff_f (fdim, plus_infty, -9, plus_infty);
  TEST_ff_f (fdim, minus_infty, 9, 0);
  TEST_ff_f (fdim, minus_infty, -9, 0);
  TEST_ff_f (fdim, 9, minus_infty, plus_infty);
  TEST_ff_f (fdim, -9, minus_infty, plus_infty);
  TEST_ff_f (fdim, 9, plus_infty, 0);
  TEST_ff_f (fdim, -9, plus_infty, 0);

  TEST_ff_f (fdim, 0, nan_value, nan_value);
  TEST_ff_f (fdim, 9, nan_value, nan_value);
  TEST_ff_f (fdim, -9, nan_value, nan_value);
  TEST_ff_f (fdim, nan_value, 9, nan_value);
  TEST_ff_f (fdim, nan_value, -9, nan_value);
  TEST_ff_f (fdim, plus_infty, nan_value, nan_value);
  TEST_ff_f (fdim, minus_infty, nan_value, nan_value);
  TEST_ff_f (fdim, nan_value, plus_infty, nan_value);
  TEST_ff_f (fdim, nan_value, minus_infty, nan_value);
  TEST_ff_f (fdim, nan_value, nan_value, nan_value);

  END (fdim);
}
#endif

static void
floor_test (void)
{
  START (floor);

  TEST_f_f (floor, 0.0, 0.0);
  TEST_f_f (floor, minus_zero, minus_zero);
  TEST_f_f (floor, plus_infty, plus_infty);
  TEST_f_f (floor, minus_infty, minus_infty);
  TEST_f_f (floor, nan_value, nan_value);

  TEST_f_f (floor, M_PIl, 3.0);
  TEST_f_f (floor, -M_PIl, -4.0);

  END (floor);
}

#if 0
static void
fma_test (void)
{
  START (fma);

  TEST_fff_f (fma, 1.0, 2.0, 3.0, 5.0);
  TEST_fff_f (fma, nan_value, 2.0, 3.0, nan_value);
  TEST_fff_f (fma, 1.0, nan_value, 3.0, nan_value);
  TEST_fff_f (fma, 1.0, 2.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_fff_f (fma, plus_infty, 0.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_fff_f (fma, minus_infty, 0.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_fff_f (fma, 0.0, plus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_fff_f (fma, 0.0, minus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_fff_f (fma, plus_infty, 0.0, 1.0, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, minus_infty, 0.0, 1.0, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, 0.0, plus_infty, 1.0, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, 0.0, minus_infty, 1.0, nan_value, INVALID_EXCEPTION);

  TEST_fff_f (fma, plus_infty, plus_infty, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, minus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, plus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, minus_infty, minus_infty, minus_infty, nan_value, INVALID_EXCEPTION);

  END (fma);
}


static void
fmax_test (void)
{
  START (fmax);

  TEST_ff_f (fmax, 0, 0, 0);
  TEST_ff_f (fmax, minus_zero, minus_zero, minus_zero);
  TEST_ff_f (fmax, 9, 0, 9);
  TEST_ff_f (fmax, 0, 9, 9);
  TEST_ff_f (fmax, -9, 0, 0);
  TEST_ff_f (fmax, 0, -9, 0);

  TEST_ff_f (fmax, plus_infty, 9, plus_infty);
  TEST_ff_f (fmax, 0, plus_infty, plus_infty);
  TEST_ff_f (fmax, -9, plus_infty, plus_infty);
  TEST_ff_f (fmax, plus_infty, -9, plus_infty);

  TEST_ff_f (fmax, minus_infty, 9, 9);
  TEST_ff_f (fmax, minus_infty, -9, -9);
  TEST_ff_f (fmax, 9, minus_infty, 9);
  TEST_ff_f (fmax, -9, minus_infty, -9);

  TEST_ff_f (fmax, 0, nan_value, 0);
  TEST_ff_f (fmax, 9, nan_value, 9);
  TEST_ff_f (fmax, -9, nan_value, -9);
  TEST_ff_f (fmax, nan_value, 0, 0);
  TEST_ff_f (fmax, nan_value, 9, 9);
  TEST_ff_f (fmax, nan_value, -9, -9);
  TEST_ff_f (fmax, plus_infty, nan_value, plus_infty);
  TEST_ff_f (fmax, minus_infty, nan_value, minus_infty);
  TEST_ff_f (fmax, nan_value, plus_infty, plus_infty);
  TEST_ff_f (fmax, nan_value, minus_infty, minus_infty);
  TEST_ff_f (fmax, nan_value, nan_value, nan_value);

  END (fmax);
}


static void
fmin_test (void)
{
  START (fmin);

  TEST_ff_f (fmin, 0, 0, 0);
  TEST_ff_f (fmin, minus_zero, minus_zero, minus_zero);
  TEST_ff_f (fmin, 9, 0, 0);
  TEST_ff_f (fmin, 0, 9, 0);
  TEST_ff_f (fmin, -9, 0, -9);
  TEST_ff_f (fmin, 0, -9, -9);

  TEST_ff_f (fmin, plus_infty, 9, 9);
  TEST_ff_f (fmin, 9, plus_infty, 9);
  TEST_ff_f (fmin, plus_infty, -9, -9);
  TEST_ff_f (fmin, -9, plus_infty, -9);
  TEST_ff_f (fmin, minus_infty, 9, minus_infty);
  TEST_ff_f (fmin, minus_infty, -9, minus_infty);
  TEST_ff_f (fmin, 9, minus_infty, minus_infty);
  TEST_ff_f (fmin, -9, minus_infty, minus_infty);

  TEST_ff_f (fmin, 0, nan_value, 0);
  TEST_ff_f (fmin, 9, nan_value, 9);
  TEST_ff_f (fmin, -9, nan_value, -9);
  TEST_ff_f (fmin, nan_value, 0, 0);
  TEST_ff_f (fmin, nan_value, 9, 9);
  TEST_ff_f (fmin, nan_value, -9, -9);
  TEST_ff_f (fmin, plus_infty, nan_value, plus_infty);
  TEST_ff_f (fmin, minus_infty, nan_value, minus_infty);
  TEST_ff_f (fmin, nan_value, plus_infty, plus_infty);
  TEST_ff_f (fmin, nan_value, minus_infty, minus_infty);
  TEST_ff_f (fmin, nan_value, nan_value, nan_value);

  END (fmin);
}
#endif

static void
fmod_test (void)
{
  errno = 0;
  FUNC(fmod) (6.5, 2.3L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (fmod);

  /* fmod (+0, y) == +0 for y != 0.  */
  TEST_ff_f (fmod, 0, 3, 0);

  /* fmod (-0, y) == -0 for y != 0.  */
  TEST_ff_f (fmod, minus_zero, 3, minus_zero);

  /* fmod (+inf, y) == NaN plus invalid exception.  */
  TEST_ff_f (fmod, plus_infty, 3, nan_value, INVALID_EXCEPTION);
  /* fmod (-inf, y) == NaN plus invalid exception.  */
  TEST_ff_f (fmod, minus_infty, 3, nan_value, INVALID_EXCEPTION);
  /* fmod (x, +0) == NaN plus invalid exception.  */
  TEST_ff_f (fmod, 3, 0, nan_value, INVALID_EXCEPTION);
  /* fmod (x, -0) == NaN plus invalid exception.  */
  TEST_ff_f (fmod, 3, minus_zero, nan_value, INVALID_EXCEPTION);

  /* fmod (x, +inf) == x for x not infinite.  */
  TEST_ff_f (fmod, 3.0, plus_infty, 3.0);
  /* fmod (x, -inf) == x for x not infinite.  */
  TEST_ff_f (fmod, 3.0, minus_infty, 3.0);

  TEST_ff_f (fmod, nan_value, nan_value, nan_value);

  TEST_ff_f (fmod, 6.5, 2.3L, 1.9L);
  TEST_ff_f (fmod, -6.5, 2.3L, -1.9L);
  TEST_ff_f (fmod, 6.5, -2.3L, 1.9L);
  TEST_ff_f (fmod, -6.5, -2.3L, -1.9L);

  END (fmod);
}

static void
fpclassify_test (void)
{
  START (fpclassify);

  TEST_f_i (fpclassify, nan_value, FP_NAN);
  TEST_f_i (fpclassify, plus_infty, FP_INFINITE);
  TEST_f_i (fpclassify, minus_infty, FP_INFINITE);
  TEST_f_i (fpclassify, plus_zero, FP_ZERO);
  TEST_f_i (fpclassify, minus_zero, FP_ZERO);
  TEST_f_i (fpclassify, 1000, FP_NORMAL);

  END (fpclassify);
}


static void
frexp_test (void)
{
  int x;

  START (frexp);

  TEST_fI_f1 (frexp, plus_infty, plus_infty, IGNORE);
  TEST_fI_f1 (frexp, minus_infty, minus_infty, IGNORE);
  TEST_fI_f1 (frexp, nan_value, nan_value, IGNORE);

  TEST_fI_f1 (frexp, 0.0, 0.0, 0.0);
  TEST_fI_f1 (frexp, minus_zero, minus_zero, 0.0);

  TEST_fI_f1 (frexp, 12.8L, 0.8L, 4);
  TEST_fI_f1 (frexp, -27.34L, -0.854375L, 5);

  END (frexp);
}


static void
gamma_test (void)
{
  errno = 0;
  FUNC(gamma) (1);

  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;
  feclearexcept (FE_ALL_EXCEPT);

  START (gamma);

  TEST_f_f (gamma, plus_infty, plus_infty);
  TEST_f_f (gamma, 0, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (gamma, -3, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (gamma, minus_infty, plus_infty);
  TEST_f_f (gamma, nan_value, nan_value);

  TEST_f_f1 (gamma, 1, 0, 1);
  TEST_f_f1 (gamma, 3, M_LN2l, 1);

  TEST_f_f1 (gamma, 0.5, M_LOG_SQRT_PIl, 1);
  TEST_f_f1 (gamma, -0.5, M_LOG_2_SQRT_PIl, -1);

  END (gamma);
}

static void
hypot_test (void)
{
  errno = 0;
  FUNC(hypot) (0.7L, 12.4L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (hypot);

  TEST_ff_f (hypot, plus_infty, 1, plus_infty, IGNORE_ZERO_INF_SIGN);
  TEST_ff_f (hypot, minus_infty, 1, plus_infty, IGNORE_ZERO_INF_SIGN);

#ifndef TEST_INLINE
  TEST_ff_f (hypot, plus_infty, nan_value, plus_infty);
  TEST_ff_f (hypot, minus_infty, nan_value, plus_infty);
  TEST_ff_f (hypot, nan_value, plus_infty, plus_infty);
  TEST_ff_f (hypot, nan_value, minus_infty, plus_infty);
#endif

  TEST_ff_f (hypot, nan_value, nan_value, nan_value);

  /* hypot (x,y) == hypot (+-x, +-y)  */
  TEST_ff_f (hypot, 0.7L, 12.4L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, -0.7L, 12.4L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, 0.7L, -12.4L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, -0.7L, -12.4L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, 12.4L, 0.7L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, -12.4L, 0.7L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, 12.4L, -0.7L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, -12.4L, -0.7L, 12.419742348374220601176836866763271L);

  /*  hypot (x,0) == fabs (x)  */
  TEST_ff_f (hypot, 0.7L, 0, 0.7L);
  TEST_ff_f (hypot, -0.7L, 0, 0.7L);
  TEST_ff_f (hypot, -5.7e7, 0, 5.7e7L);

  TEST_ff_f (hypot, 0.7L, 1.2L, 1.3892443989449804508432547041028554L);

  END (hypot);
}


static void
ilogb_test (void)
{
  START (ilogb);

  TEST_f_i (ilogb, 1, 0);
  TEST_f_i (ilogb, M_El, 1);
  TEST_f_i (ilogb, 1024, 10);
  TEST_f_i (ilogb, -2000, 10);

  /* XXX We have a problem here: the standard does not tell us whether
     exceptions are allowed/required.  ignore them for now.  */

  TEST_f_i (ilogb, 0.0, FP_ILOGB0, EXCEPTIONS_OK);
  TEST_f_i (ilogb, nan_value, FP_ILOGBNAN, EXCEPTIONS_OK);
  TEST_f_i (ilogb, plus_infty, INT_MAX, EXCEPTIONS_OK);
  TEST_f_i (ilogb, minus_infty, INT_MAX, EXCEPTIONS_OK);

  END (ilogb);
}

static void
isfinite_test (void)
{
  START (isfinite);

  TEST_f_b (isfinite, 0, 1);
  TEST_f_b (isfinite, minus_zero, 1);
  TEST_f_b (isfinite, 10, 1);
  TEST_f_b (isfinite, plus_infty, 0);
  TEST_f_b (isfinite, minus_infty, 0);
  TEST_f_b (isfinite, nan_value, 0);

  END (isfinite);
}

static void
isnormal_test (void)
{
  START (isnormal);

  TEST_f_b (isnormal, 0, 0);
  TEST_f_b (isnormal, minus_zero, 0);
  TEST_f_b (isnormal, 10, 1);
  TEST_f_b (isnormal, plus_infty, 0);
  TEST_f_b (isnormal, minus_infty, 0);
  TEST_f_b (isnormal, nan_value, 0);

  END (isnormal);
}

static void
j0_test (void)
{
  errno = 0;
#if 0
  FLOAT s, c;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
#endif
  FUNC(j0) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (j0);

  /* j0 is the Bessel function of the first kind of order 0 */
  TEST_f_f (j0, nan_value, nan_value);
  TEST_f_f (j0, plus_infty, 0);
  TEST_f_f (j0, -1.0, 0.76519768655796655145L);
  TEST_f_f (j0, 0.0, 1.0);
  TEST_f_f (j0, 0.1L, 0.99750156206604003228L);
  TEST_f_f (j0, 0.7L, 0.88120088860740528084L);
  TEST_f_f (j0, 1.0, 0.76519768655796655145L);
  TEST_f_f (j0, 1.5, 0.51182767173591812875L);
  TEST_f_f (j0, 2.0, 0.22389077914123566805L);
  TEST_f_f (j0, 8.0, 0.17165080713755390609L);
  TEST_f_f (j0, 10.0, -0.24593576445134833520L);
  TEST_f_f (j0, 4.0, -3.9714980986384737228659076845169804197562E-1L);
  TEST_f_f (j0, -4.0, -3.9714980986384737228659076845169804197562E-1L);

  END (j0);
}


static void
j1_test (void)
{
  errno = 0;
#if 0
  FLOAT s, c;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
#endif
  FUNC(j1) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* j1 is the Bessel function of the first kind of order 1 */

  START (j1);

  TEST_f_f (j1, nan_value, nan_value);
  TEST_f_f (j1, plus_infty, 0);

  TEST_f_f (j1, -1.0, -0.44005058574493351596L);
  TEST_f_f (j1, 0.0, 0.0);
  TEST_f_f (j1, 0.1L, 0.049937526036241997556L);
  TEST_f_f (j1, 0.7L, 0.32899574154005894785L);
  TEST_f_f (j1, 1.0, 0.44005058574493351596L);
  TEST_f_f (j1, 1.5, 0.55793650791009964199L);
  TEST_f_f (j1, 2.0, 0.57672480775687338720L);
  TEST_f_f (j1, 8.0, 0.23463634685391462438L);
  TEST_f_f (j1, 10.0, 0.043472746168861436670L);

  END (j1);
}

static void
jn_test (void)
{
  errno = 0;
#if 0
  FLOAT s, c;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
#endif
  FUNC(jn) (1, 1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* jn is the Bessel function of the first kind of order n.  */
  START (jn);

  /* jn (0, x) == j0 (x)  */
  TEST_ff_f (jn, 0, nan_value, nan_value);
  TEST_ff_f (jn, 0, plus_infty, 0);
  TEST_ff_f (jn, 0, -1.0, 0.76519768655796655145L);
  TEST_ff_f (jn, 0, 0.0, 1.0);
  TEST_ff_f (jn, 0, 0.1L, 0.99750156206604003228L);
  TEST_ff_f (jn, 0, 0.7L, 0.88120088860740528084L);
  TEST_ff_f (jn, 0, 1.0, 0.76519768655796655145L);
  TEST_ff_f (jn, 0, 1.5, 0.51182767173591812875L);
  TEST_ff_f (jn, 0, 2.0, 0.22389077914123566805L);
  TEST_ff_f (jn, 0, 8.0, 0.17165080713755390609L);
  TEST_ff_f (jn, 0, 10.0, -0.24593576445134833520L);

  /* jn (1, x) == j1 (x)  */
  TEST_ff_f (jn, 1, nan_value, nan_value);
  TEST_ff_f (jn, 1, plus_infty, 0);

  TEST_ff_f (jn, 1, -1.0, -0.44005058574493351596L);
  TEST_ff_f (jn, 1, 0.0, 0.0);
  TEST_ff_f (jn, 1, 0.1L, 0.049937526036241997556L);
  TEST_ff_f (jn, 1, 0.7L, 0.32899574154005894785L);
  TEST_ff_f (jn, 1, 1.0, 0.44005058574493351596L);
  TEST_ff_f (jn, 1, 1.5, 0.55793650791009964199L);
  TEST_ff_f (jn, 1, 2.0, 0.57672480775687338720L);
  TEST_ff_f (jn, 1, 8.0, 0.23463634685391462438L);
  TEST_ff_f (jn, 1, 10.0, 0.043472746168861436670L);

  /* jn (3, x)  */
  TEST_ff_f (jn, 3, nan_value, nan_value);
  TEST_ff_f (jn, 3, plus_infty, 0);

  TEST_ff_f (jn, 3, -1.0, -0.019563353982668405919L);
  TEST_ff_f (jn, 3, 0.0, 0.0);
  TEST_ff_f (jn, 3, 0.1L, 0.000020820315754756261429L);
  TEST_ff_f (jn, 3, 0.7L, 0.0069296548267508408077L);
  TEST_ff_f (jn, 3, 1.0, 0.019563353982668405919L);
  TEST_ff_f (jn, 3, 2.0, 0.12894324947440205110L);
  TEST_ff_f (jn, 3, 10.0, 0.058379379305186812343L);

  /*  jn (10, x)  */
  TEST_ff_f (jn, 10, nan_value, nan_value);
  TEST_ff_f (jn, 10, plus_infty, 0);

  TEST_ff_f (jn, 10, -1.0, 0.26306151236874532070e-9L);
  TEST_ff_f (jn, 10, 0.0, 0.0);
  TEST_ff_f (jn, 10, 0.1L, 0.26905328954342155795e-19L);
  TEST_ff_f (jn, 10, 0.7L, 0.75175911502153953928e-11L);
  TEST_ff_f (jn, 10, 1.0, 0.26306151236874532070e-9L);
  TEST_ff_f (jn, 10, 2.0, 0.25153862827167367096e-6L);
  TEST_ff_f (jn, 10, 10.0, 0.20748610663335885770L);

  END (jn);
}


static void
ldexp_test (void)
{
  TEST_ff_f (ldexp, 0, 0, 0);
  TEST_ff_f (ldexp, minus_zero, 0, minus_zero);

  TEST_ff_f (ldexp, plus_infty, 1, plus_infty);
  TEST_ff_f (ldexp, minus_infty, 1, minus_infty);
  TEST_ff_f (ldexp, nan_value, 1, nan_value);

  TEST_ff_f (ldexp, 0.8L, 4, 12.8L);
  TEST_ff_f (ldexp, -0.854375L, 5, -27.34L);

  /* ldexp (x, 0) == x.  */
  TEST_ff_f (ldexp, 1.0L, 0L, 1.0L);
}

static void
lgamma_test (void)
{
  errno = 0;
  FUNC(lgamma) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;
  feclearexcept (FE_ALL_EXCEPT);

  START (lgamma);

  TEST_f_f (lgamma, plus_infty, plus_infty);
  TEST_f_f (lgamma, 0, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (lgamma, nan_value, nan_value);

  /* lgamma (x) == +inf plus divide by zero exception for integer x <= 0.  */
  TEST_f_f (lgamma, -3, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (lgamma, minus_infty, plus_infty);

  TEST_f_f1 (lgamma, 1, 0, 1);

  TEST_f_f1 (lgamma, 3, M_LN2l, 1);

  TEST_f_f1 (lgamma, 0.5, M_LOG_SQRT_PIl, 1);
  TEST_f_f1 (lgamma, -0.5, M_LOG_2_SQRT_PIl, -1);
  TEST_f_f1 (lgamma, 0.7L, 0.26086724653166651439L, 1);
  TEST_f_f1 (lgamma, 1.2L, -0.853740900033158497197e-1L, 1);

  END (lgamma);
}

#if 0
static void
lrint_test (void)
{
  /* XXX this test is incomplete.  We need to have a way to specifiy
     the rounding method and test the critical cases.  So far, only
     unproblematic numbers are tested.  */

  START (lrint);

  TEST_f_l (lrint, 0.0, 0);
  TEST_f_l (lrint, minus_zero, 0);
  TEST_f_l (lrint, 0.2L, 0);
  TEST_f_l (lrint, -0.2L, 0);

  TEST_f_l (lrint, 1.4L, 1);
  TEST_f_l (lrint, -1.4L, -1);

  TEST_f_l (lrint, 8388600.3L, 8388600);
  TEST_f_l (lrint, -8388600.3L, -8388600);

  END (lrint);
}

static void
llrint_test (void)
{
  /* XXX this test is incomplete.  We need to have a way to specifiy
     the rounding method and test the critical cases.  So far, only
     unproblematic numbers are tested.  */

  START (llrint);

  TEST_f_L (llrint, 0.0, 0);
  TEST_f_L (llrint, minus_zero, 0);
  TEST_f_L (llrint, 0.2L, 0);
  TEST_f_L (llrint, -0.2L, 0);

  TEST_f_L (llrint, 1.4L, 1);
  TEST_f_L (llrint, -1.4L, -1);

  TEST_f_L (llrint, 8388600.3L, 8388600);
  TEST_f_L (llrint, -8388600.3L, -8388600);

  /* Test boundary conditions.  */
  /* 0x1FFFFF */
  TEST_f_L (llrint, 2097151.0,2097151LL);
  /* 0x800000 */
  TEST_f_L (llrint, 8388608.0, 8388608LL);
  /* 0x1000000 */
  TEST_f_L (llrint, 16777216.0, 16777216LL);
  /* 0x20000000000 */
  TEST_f_L (llrint, 2199023255552.0, 2199023255552LL);
  /* 0x40000000000 */
  TEST_f_L (llrint, 4398046511104.0, 4398046511104LL);
  /* 0x10000000000000 */
  TEST_f_L (llrint, 4503599627370496.0, 4503599627370496LL);
  /* 0x10000080000000 */
  TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL);
  /* 0x20000000000000 */
  TEST_f_L (llrint, 9007199254740992.0, 9007199254740992LL);
  /* 0x80000000000000 */
  TEST_f_L (llrint, 36028797018963968.0, 36028797018963968LL);
  /* 0x100000000000000 */
  TEST_f_L (llrint, 72057594037927936.0, 72057594037927936LL);

  END (llrint);
}
#endif

static void
log_test (void)
{
  errno = 0;
  FUNC(log) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;
  START (log);

  TEST_f_f (log, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (log, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_f_f (log, 1, 0);

  TEST_f_f (log, -1, nan_value, INVALID_EXCEPTION);
  TEST_f_f (log, plus_infty, plus_infty);

  TEST_f_f (log, M_El, 1);
  TEST_f_f (log, 1.0 / M_El, -1);
  TEST_f_f (log, 2, M_LN2l);
  TEST_f_f (log, 10, M_LN10l);
  TEST_f_f (log, 0.7L, -0.35667494393873237891263871124118447L);

  END (log);
}


static void
log10_test (void)
{
  errno = 0;
  FUNC(log10) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (log10);

  TEST_f_f (log10, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (log10, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_f_f (log10, 1, 0);

  /* log10 (x) == NaN plus invalid exception if x < 0.  */
  TEST_f_f (log10, -1, nan_value, INVALID_EXCEPTION);

  TEST_f_f (log10, plus_infty, plus_infty);
  TEST_f_f (log10, nan_value, nan_value);

  TEST_f_f (log10, 0.1L, -1);
  TEST_f_f (log10, 10.0, 1);
  TEST_f_f (log10, 100.0, 2);
  TEST_f_f (log10, 10000.0, 4);
  TEST_f_f (log10, M_El, M_LOG10El);
  TEST_f_f (log10, 0.7L, -0.15490195998574316929L);

  END (log10);
}


static void
log1p_test (void)
{
  errno = 0;
  FUNC(log1p) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (log1p);

  TEST_f_f (log1p, 0, 0);
  TEST_f_f (log1p, minus_zero, minus_zero);

  TEST_f_f (log1p, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (log1p, -2, nan_value, INVALID_EXCEPTION);

  TEST_f_f (log1p, plus_infty, plus_infty);
  TEST_f_f (log1p, nan_value, nan_value);

  TEST_f_f (log1p, M_El - 1.0, 1);

  TEST_f_f (log1p, -0.3L, -0.35667494393873237891263871124118447L);

  END (log1p);
}

#if 0
static void
log2_test (void)
{
  errno = 0;
  FUNC(log2) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (log2);

  TEST_f_f (log2, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (log2, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_f_f (log2, 1, 0);

  TEST_f_f (log2, -1, nan_value, INVALID_EXCEPTION);

  TEST_f_f (log2, plus_infty, plus_infty);
  TEST_f_f (log2, nan_value, nan_value);

  TEST_f_f (log2, M_El, M_LOG2El);
  TEST_f_f (log2, 2.0, 1);
  TEST_f_f (log2, 16.0, 4);
  TEST_f_f (log2, 256.0, 8);
  TEST_f_f (log2, 0.7L, -0.51457317282975824043L);

  END (log2);
}
#endif


static void
logb_test (void)
{
  START (logb);

  TEST_f_f (logb, plus_infty, plus_infty);
  TEST_f_f (logb, minus_infty, plus_infty);

  TEST_f_f (logb, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_f_f (logb, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (logb, nan_value, nan_value);

  TEST_f_f (logb, 1, 0);
  TEST_f_f (logb, M_El, 1);
  TEST_f_f (logb, 1024, 10);
  TEST_f_f (logb, -2000, 10);

  END (logb);
}

#if 0
static void
lround_test (void)
{
  START (lround);

  TEST_f_l (lround, 0, 0);
  TEST_f_l (lround, minus_zero, 0);
  TEST_f_l (lround, 0.2L, 0.0);
  TEST_f_l (lround, -0.2L, 0);
  TEST_f_l (lround, 0.5, 1);
  TEST_f_l (lround, -0.5, -1);
  TEST_f_l (lround, 0.8L, 1);
  TEST_f_l (lround, -0.8L, -1);
  TEST_f_l (lround, 1.5, 2);
  TEST_f_l (lround, -1.5, -2);
  TEST_f_l (lround, 22514.5, 22515);
  TEST_f_l (lround, -22514.5, -22515);
#ifndef TEST_FLOAT
  TEST_f_l (lround, 2097152.5, 2097153);
  TEST_f_l (lround, -2097152.5, -2097153);
#endif
  END (lround);
}


static void
llround_test (void)
{
  START (llround);

  TEST_f_L (llround, 0, 0);
  TEST_f_L (llround, minus_zero, 0);
  TEST_f_L (llround, 0.2L, 0.0);
  TEST_f_L (llround, -0.2L, 0);
  TEST_f_L (llround, 0.5, 1);
  TEST_f_L (llround, -0.5, -1);
  TEST_f_L (llround, 0.8L, 1);
  TEST_f_L (llround, -0.8L, -1);
  TEST_f_L (llround, 1.5, 2);
  TEST_f_L (llround, -1.5, -2);
  TEST_f_L (llround, 22514.5, 22515);
  TEST_f_L (llround, -22514.5, -22515);
#ifndef TEST_FLOAT
  TEST_f_L (llround, 2097152.5, 2097153);
  TEST_f_L (llround, -2097152.5, -2097153);
  TEST_f_L (llround, 34359738368.5, 34359738369ll);
  TEST_f_L (llround, -34359738368.5, -34359738369ll);
#endif

  /* Test boundary conditions.  */
  /* 0x1FFFFF */
  TEST_f_L (llround, 2097151.0, 2097151LL);
  /* 0x800000 */
  TEST_f_L (llround, 8388608.0, 8388608LL);
  /* 0x1000000 */
  TEST_f_L (llround, 16777216.0, 16777216LL);
  /* 0x20000000000 */
  TEST_f_L (llround, 2199023255552.0, 2199023255552LL);
  /* 0x40000000000 */
  TEST_f_L (llround, 4398046511104.0, 4398046511104LL);
  /* 0x10000000000000 */
  TEST_f_L (llround, 4503599627370496.0, 4503599627370496LL);
  /* 0x10000080000000 */
  TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL);
  /* 0x20000000000000 */
  TEST_f_L (llround, 9007199254740992.0, 9007199254740992LL);
  /* 0x80000000000000 */
  TEST_f_L (llround, 36028797018963968.0, 36028797018963968LL);
  /* 0x100000000000000 */
  TEST_f_L (llround, 72057594037927936.0, 72057594037927936LL);

#ifndef TEST_FLOAT
  /* 0x100000000 */
  TEST_f_L (llround, 4294967295.5, 4294967296LL);
  /* 0x200000000 */
  TEST_f_L (llround, 8589934591.5, 8589934592LL);
#endif

  END (llround);
}
#endif

static void
modf_test (void)
{
  FLOAT x;

  START (modf);

  TEST_fF_f1 (modf, plus_infty, 0, plus_infty);
  TEST_fF_f1 (modf, minus_infty, minus_zero, minus_infty);
  TEST_fF_f1 (modf, nan_value, nan_value, nan_value);
  TEST_fF_f1 (modf, 0, 0, 0);
  TEST_fF_f1 (modf, 1.5, 0.5, 1);
  TEST_fF_f1 (modf, 2.5, 0.5, 2);
  TEST_fF_f1 (modf, -2.5, -0.5, -2);
  TEST_fF_f1 (modf, 20, 0, 20);
  TEST_fF_f1 (modf, 21, 0, 21);
  TEST_fF_f1 (modf, 89.5, 0.5, 89);

  END (modf);
}


#if 0
static void
nearbyint_test (void)
{
  START (nearbyint);

  TEST_f_f (nearbyint, 0.0, 0.0);
  TEST_f_f (nearbyint, minus_zero, minus_zero);
  TEST_f_f (nearbyint, plus_infty, plus_infty);
  TEST_f_f (nearbyint, minus_infty, minus_infty);
  TEST_f_f (nearbyint, nan_value, nan_value);

  /* Default rounding mode is round to nearest.  */
  TEST_f_f (nearbyint, 0.5, 0.0);
  TEST_f_f (nearbyint, 1.5, 2.0);
  TEST_f_f (nearbyint, -0.5, minus_zero);
  TEST_f_f (nearbyint, -1.5, -2.0);

  END (nearbyint);
}

static void
nextafter_test (void)
{

  START (nextafter);

  TEST_ff_f (nextafter, 0, 0, 0);
  TEST_ff_f (nextafter, minus_zero, 0, 0);
  TEST_ff_f (nextafter, 0, minus_zero, minus_zero);
  TEST_ff_f (nextafter, minus_zero, minus_zero, minus_zero);

  TEST_ff_f (nextafter, 9, 9, 9);
  TEST_ff_f (nextafter, -9, -9, -9);
  TEST_ff_f (nextafter, plus_infty, plus_infty, plus_infty);
  TEST_ff_f (nextafter, minus_infty, minus_infty, minus_infty);

  TEST_ff_f (nextafter, nan_value, 1.1L, nan_value);
  TEST_ff_f (nextafter, 1.1L, nan_value, nan_value);
  TEST_ff_f (nextafter, nan_value, nan_value, nan_value);

  /* XXX We need the hexadecimal FP number representation here for further
     tests.  */

  END (nextafter);
}


static void
nexttoward_test (void)
{
  START (nexttoward);
  TEST_ff_f (nexttoward, 0, 0, 0);
  TEST_ff_f (nexttoward, minus_zero, 0, 0);
  TEST_ff_f (nexttoward, 0, minus_zero, minus_zero);
  TEST_ff_f (nexttoward, minus_zero, minus_zero, minus_zero);

  TEST_ff_f (nexttoward, 9, 9, 9);
  TEST_ff_f (nexttoward, -9, -9, -9);
  TEST_ff_f (nexttoward, plus_infty, plus_infty, plus_infty);
  TEST_ff_f (nexttoward, minus_infty, minus_infty, minus_infty);

  TEST_ff_f (nexttoward, nan_value, 1.1L, nan_value);
  TEST_ff_f (nexttoward, 1.1L, nan_value, nan_value);
  TEST_ff_f (nexttoward, nan_value, nan_value, nan_value);

  /* XXX We need the hexadecimal FP number representation here for further
     tests.  */

  END (nexttoward);
}
#endif

static void
pow_test (void)
{

  errno = 0;
  FUNC(pow) (0, 0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (pow);

  TEST_ff_f (pow, 0, 0, 1);
  TEST_ff_f (pow, 0, minus_zero, 1);
  TEST_ff_f (pow, minus_zero, 0, 1);
  TEST_ff_f (pow, minus_zero, minus_zero, 1);

  TEST_ff_f (pow, 10, 0, 1);
  TEST_ff_f (pow, 10, minus_zero, 1);
  TEST_ff_f (pow, -10, 0, 1);
  TEST_ff_f (pow, -10, minus_zero, 1);

  TEST_ff_f (pow, nan_value, 0, 1);
  TEST_ff_f (pow, nan_value, minus_zero, 1);


#ifndef TEST_INLINE
  TEST_ff_f (pow, 1.1L, plus_infty, plus_infty);
  TEST_ff_f (pow, plus_infty, plus_infty, plus_infty);
  TEST_ff_f (pow, -1.1L, plus_infty, plus_infty);
  TEST_ff_f (pow, minus_infty, plus_infty, plus_infty);

  TEST_ff_f (pow, 0.9L, plus_infty, 0);
  TEST_ff_f (pow, 1e-7L, plus_infty, 0);
  TEST_ff_f (pow, -0.9L, plus_infty, 0);
  TEST_ff_f (pow, -1e-7L, plus_infty, 0);

  TEST_ff_f (pow, 1.1L, minus_infty, 0);
  TEST_ff_f (pow, plus_infty, minus_infty, 0);
  TEST_ff_f (pow, -1.1L, minus_infty, 0);
  TEST_ff_f (pow, minus_infty, minus_infty, 0);

  TEST_ff_f (pow, 0.9L, minus_infty, plus_infty);
  TEST_ff_f (pow, 1e-7L, minus_infty, plus_infty);
  TEST_ff_f (pow, -0.9L, minus_infty, plus_infty);
  TEST_ff_f (pow, -1e-7L, minus_infty, plus_infty);

  TEST_ff_f (pow, plus_infty, 1e-7L, plus_infty);
  TEST_ff_f (pow, plus_infty, 1, plus_infty);
  TEST_ff_f (pow, plus_infty, 1e7L, plus_infty);

  TEST_ff_f (pow, plus_infty, -1e-7L, 0);
  TEST_ff_f (pow, plus_infty, -1, 0);
  TEST_ff_f (pow, plus_infty, -1e7L, 0);

  TEST_ff_f (pow, minus_infty, 1, minus_infty);
  TEST_ff_f (pow, minus_infty, 11, minus_infty);
  TEST_ff_f (pow, minus_infty, 1001, minus_infty);

  TEST_ff_f (pow, minus_infty, 2, plus_infty);
  TEST_ff_f (pow, minus_infty, 12, plus_infty);
  TEST_ff_f (pow, minus_infty, 1002, plus_infty);
  TEST_ff_f (pow, minus_infty, 0.1L, plus_infty);
  TEST_ff_f (pow, minus_infty, 1.1L, plus_infty);
  TEST_ff_f (pow, minus_infty, 11.1L, plus_infty);
  TEST_ff_f (pow, minus_infty, 1001.1L, plus_infty);

  TEST_ff_f (pow, minus_infty, -1, minus_zero);
  TEST_ff_f (pow, minus_infty, -11, minus_zero);
  TEST_ff_f (pow, minus_infty, -1001, minus_zero);

  TEST_ff_f (pow, minus_infty, -2, 0);
  TEST_ff_f (pow, minus_infty, -12, 0);
  TEST_ff_f (pow, minus_infty, -1002, 0);
  TEST_ff_f (pow, minus_infty, -0.1L, 0);
  TEST_ff_f (pow, minus_infty, -1.1L, 0);
  TEST_ff_f (pow, minus_infty, -11.1L, 0);
  TEST_ff_f (pow, minus_infty, -1001.1L, 0);
#endif

  TEST_ff_f (pow, nan_value, nan_value, nan_value);
  TEST_ff_f (pow, 0, nan_value, nan_value);
  TEST_ff_f (pow, 1, nan_value, 1);
  TEST_ff_f (pow, -1, nan_value, nan_value);
  TEST_ff_f (pow, nan_value, 1, nan_value);
  TEST_ff_f (pow, nan_value, -1, nan_value);

  /* pow (x, NaN) == NaN.  */
  TEST_ff_f (pow, 3.0, nan_value, nan_value);

  TEST_ff_f (pow, 1, plus_infty, 1);
  TEST_ff_f (pow, -1, plus_infty, 1);
  TEST_ff_f (pow, 1, minus_infty, 1);
  TEST_ff_f (pow, -1, minus_infty, 1);

  TEST_ff_f (pow, -0.1L, 1.1L, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -0.1L, -1.1L, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -10.1L, 1.1L, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -10.1L, -1.1L, nan_value, INVALID_EXCEPTION);

  TEST_ff_f (pow, 0, -1, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_ff_f (pow, 0, -11, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_ff_f (pow, minus_zero, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_ff_f (pow, minus_zero, -11, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_ff_f (pow, 0, -2, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_ff_f (pow, 0, -11.1L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_ff_f (pow, minus_zero, -2, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_ff_f (pow, minus_zero, -11.1L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);


  TEST_ff_f (pow, 0, 1, 0);
  TEST_ff_f (pow, 0, 11, 0);

  TEST_ff_f (pow, minus_zero, 1, minus_zero);
  TEST_ff_f (pow, minus_zero, 11, minus_zero);


  TEST_ff_f (pow, 0, 2, 0);
  TEST_ff_f (pow, 0, 11.1L, 0);


  TEST_ff_f (pow, minus_zero, 2, 0);
  TEST_ff_f (pow, minus_zero, 11.1L, 0);

#ifndef TEST_INLINE
  /* pow (x, +inf) == +inf for |x| > 1.  */
  TEST_ff_f (pow, 1.5, plus_infty, plus_infty);

  /* pow (x, +inf) == +0 for |x| < 1.  */
  TEST_ff_f (pow, 0.5, plus_infty, 0.0);

  /* pow (x, -inf) == +0 for |x| > 1.  */
  TEST_ff_f (pow, 1.5, minus_infty, 0.0);

  /* pow (x, -inf) == +inf for |x| < 1.  */
  TEST_ff_f (pow, 0.5, minus_infty, plus_infty);
#endif

  /* pow (+inf, y) == +inf for y > 0.  */
  TEST_ff_f (pow, plus_infty, 2, plus_infty);

  /* pow (+inf, y) == +0 for y < 0.  */
  TEST_ff_f (pow, plus_infty, -1, 0.0);

  /* pow (-inf, y) == -inf for y an odd integer > 0.  */
  TEST_ff_f (pow, minus_infty, 27, minus_infty);

  /* pow (-inf, y) == +inf for y > 0 and not an odd integer.  */
  TEST_ff_f (pow, minus_infty, 28, plus_infty);

  /* pow (-inf, y) == -0 for y an odd integer < 0. */
  TEST_ff_f (pow, minus_infty, -3, minus_zero);
  /* pow (-inf, y) == +0 for y < 0 and not an odd integer.  */
  TEST_ff_f (pow, minus_infty, -2.0, 0.0);

  /* pow (+0, y) == +0 for y an odd integer > 0.  */
  TEST_ff_f (pow, 0.0, 27, 0.0);

  /* pow (-0, y) == -0 for y an odd integer > 0.  */
  TEST_ff_f (pow, minus_zero, 27, minus_zero);

  /* pow (+0, y) == +0 for y > 0 and not an odd integer.  */
  TEST_ff_f (pow, 0.0, 4, 0.0);

  /* pow (-0, y) == +0 for y > 0 and not an odd integer.  */
  TEST_ff_f (pow, minus_zero, 4, 0.0);

  TEST_ff_f (pow, 0.7L, 1.2L, 0.65180494056638638188L);

#if defined TEST_DOUBLE || defined TEST_LDOUBLE
  TEST_ff_f (pow, -7.49321e+133, -9.80818e+16, 0);
#endif

  END (pow);
}

static void
remainder_test (void)
{
  errno = 0;
  FUNC(remainder) (1.625, 1.0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (remainder);

  TEST_ff_f (remainder, 1, 0, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (remainder, 1, minus_zero, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (remainder, plus_infty, 1, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (remainder, minus_infty, 1, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (remainder, nan_value, nan_value, nan_value);

  TEST_ff_f (remainder, 1.625, 1.0, -0.375);
  TEST_ff_f (remainder, -1.625, 1.0, 0.375);
  TEST_ff_f (remainder, 1.625, -1.0, -0.375);
  TEST_ff_f (remainder, -1.625, -1.0, 0.375);
  TEST_ff_f (remainder, 5.0, 2.0, 1.0);
  TEST_ff_f (remainder, 3.0, 2.0, -1.0);

  END (remainder);
}

#if 0
static void
remquo_test (void)
{
  /* x is needed.  */
  int x;

  errno = 0;
  FUNC(remquo) (1.625, 1.0, &x);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (remquo);

  TEST_ffI_f1 (remquo, 1, 0, nan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, 1, minus_zero, nan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, plus_infty, 1, nan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, minus_infty, 1, nan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, nan_value, nan_value, nan_value, IGNORE);

  TEST_ffI_f1 (remquo, 1.625, 1.0, -0.375, 2);
  TEST_ffI_f1 (remquo, -1.625, 1.0, 0.375, -2);
  TEST_ffI_f1 (remquo, 1.625, -1.0, -0.375, -2);
  TEST_ffI_f1 (remquo, -1.625, -1.0, 0.375, 2);

  TEST_ffI_f1 (remquo, 5, 2, 1, 2);
  TEST_ffI_f1 (remquo, 3, 2, -1, 2);

  END (remquo);
}
#endif

static void
rint_test (void)
{
  START (rint);

  TEST_f_f (rint, 0.0, 0.0);
  TEST_f_f (rint, minus_zero, minus_zero);
  TEST_f_f (rint, plus_infty, plus_infty);
  TEST_f_f (rint, minus_infty, minus_infty);

  /* Default rounding mode is round to even.  */
  TEST_f_f (rint, 0.5, 0.0);
  TEST_f_f (rint, 1.5, 2.0);
  TEST_f_f (rint, 2.5, 2.0);
  TEST_f_f (rint, 3.5, 4.0);
  TEST_f_f (rint, 4.5, 4.0);
  TEST_f_f (rint, -0.5, -0.0);
  TEST_f_f (rint, -1.5, -2.0);
  TEST_f_f (rint, -2.5, -2.0);
  TEST_f_f (rint, -3.5, -4.0);
  TEST_f_f (rint, -4.5, -4.0);

  END (rint);
}

#if 0
static void
round_test (void)
{
  START (round);

  TEST_f_f (round, 0, 0);
  TEST_f_f (round, minus_zero, minus_zero);
  TEST_f_f (round, 0.2L, 0.0);
  TEST_f_f (round, -0.2L, minus_zero);
  TEST_f_f (round, 0.5, 1.0);
  TEST_f_f (round, -0.5, -1.0);
  TEST_f_f (round, 0.8L, 1.0);
  TEST_f_f (round, -0.8L, -1.0);
  TEST_f_f (round, 1.5, 2.0);
  TEST_f_f (round, -1.5, -2.0);
  TEST_f_f (round, 2097152.5, 2097153);
  TEST_f_f (round, -2097152.5, -2097153);

  END (round);
}
#endif


static void
scalb_test (void)
{

  START (scalb);

  TEST_ff_f (scalb, 2.0, 0.5, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (scalb, 3.0, -2.5, nan_value, INVALID_EXCEPTION);

  TEST_ff_f (scalb, 0, nan_value, nan_value);
  TEST_ff_f (scalb, 1, nan_value, nan_value);

  TEST_ff_f (scalb, 1, 0, 1);
  TEST_ff_f (scalb, -1, 0, -1);

  TEST_ff_f (scalb, 0, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (scalb, minus_zero, plus_infty, nan_value, INVALID_EXCEPTION);

  TEST_ff_f (scalb, 0, 2, 0);
  TEST_ff_f (scalb, minus_zero, -4, minus_zero);
  TEST_ff_f (scalb, 0, 0, 0);
  TEST_ff_f (scalb, minus_zero, 0, minus_zero);
  TEST_ff_f (scalb, 0, -1, 0);
  TEST_ff_f (scalb, minus_zero, -10, minus_zero);
  TEST_ff_f (scalb, 0, minus_infty, 0);
  TEST_ff_f (scalb, minus_zero, minus_infty, minus_zero);

  TEST_ff_f (scalb, plus_infty, -1, plus_infty);
  TEST_ff_f (scalb, minus_infty, -10, minus_infty);
  TEST_ff_f (scalb, plus_infty, 0, plus_infty);
  TEST_ff_f (scalb, minus_infty, 0, minus_infty);
  TEST_ff_f (scalb, plus_infty, 2, plus_infty);
  TEST_ff_f (scalb, minus_infty, 100, minus_infty);

  TEST_ff_f (scalb, 0.1L, minus_infty, 0.0);
  TEST_ff_f (scalb, -0.1L, minus_infty, minus_zero);

  TEST_ff_f (scalb, 1, plus_infty, plus_infty);
  TEST_ff_f (scalb, -1, plus_infty, minus_infty);
  TEST_ff_f (scalb, plus_infty, plus_infty, plus_infty);
  TEST_ff_f (scalb, minus_infty, plus_infty, minus_infty);

  TEST_ff_f (scalb, plus_infty, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (scalb, minus_infty, minus_infty, nan_value, INVALID_EXCEPTION);

  TEST_ff_f (scalb, nan_value, 1, nan_value);
  TEST_ff_f (scalb, 1, nan_value, nan_value);
  TEST_ff_f (scalb, nan_value, 0, nan_value);
  TEST_ff_f (scalb, 0, nan_value, nan_value);
  TEST_ff_f (scalb, nan_value, plus_infty, nan_value);
  TEST_ff_f (scalb, plus_infty, nan_value, nan_value);
  TEST_ff_f (scalb, nan_value, nan_value, nan_value);

  TEST_ff_f (scalb, 0.8L, 4, 12.8L);
  TEST_ff_f (scalb, -0.854375L, 5, -27.34L);

  END (scalb);
}


static void
scalbn_test (void)
{

  START (scalbn);

  TEST_fi_f (scalbn, 0, 0, 0);
  TEST_fi_f (scalbn, minus_zero, 0, minus_zero);

  TEST_fi_f (scalbn, plus_infty, 1, plus_infty);
  TEST_fi_f (scalbn, minus_infty, 1, minus_infty);
  TEST_fi_f (scalbn, nan_value, 1, nan_value);

  TEST_fi_f (scalbn, 0.8L, 4, 12.8L);
  TEST_fi_f (scalbn, -0.854375L, 5, -27.34L);

  TEST_fi_f (scalbn, 1, 0L, 1);

  END (scalbn);
}

#if 0
static void
scalbln_test (void)
{

  START (scalbln);

  TEST_fl_f (scalbln, 0, 0, 0);
  TEST_fl_f (scalbln, minus_zero, 0, minus_zero);

  TEST_fl_f (scalbln, plus_infty, 1, plus_infty);
  TEST_fl_f (scalbln, minus_infty, 1, minus_infty);
  TEST_fl_f (scalbln, nan_value, 1, nan_value);

  TEST_fl_f (scalbln, 0.8L, 4, 12.8L);
  TEST_fl_f (scalbln, -0.854375L, 5, -27.34L);

  TEST_fl_f (scalbln, 1, 0L, 1);

  END (scalbn);
}
#endif

static void
signbit_test (void)
{

  START (signbit);

  TEST_f_b (signbit, 0, 0);
  TEST_f_b (signbit, minus_zero, 1);
  TEST_f_b (signbit, plus_infty, 0);
  TEST_f_b (signbit, minus_infty, 1);

  /* signbit (x) != 0 for x < 0.  */
  TEST_f_b (signbit, -1, 1);
  /* signbit (x) == 0 for x >= 0.  */
  TEST_f_b (signbit, 1, 0);

  END (signbit);
}

static void
sin_test (void)
{
  errno = 0;
  FUNC(sin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sin);

  TEST_f_f (sin, 0, 0);
  TEST_f_f (sin, minus_zero, minus_zero);
  TEST_f_f (sin, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (sin, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (sin, nan_value, nan_value);

  TEST_f_f (sin, M_PI_6l, 0.5);
  TEST_f_f (sin, -M_PI_6l, -0.5);
  TEST_f_f (sin, M_PI_2l, 1);
  TEST_f_f (sin, -M_PI_2l, -1);
  TEST_f_f (sin, 0.7L, 0.64421768723769105367261435139872014L);

  END (sin);

}

#if 0
static void
sincos_test (void)
{
  FLOAT sin_res, cos_res;

  errno = 0;
  FUNC(sincos) (0, &sin_res, &cos_res);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sincos);

  /* sincos is treated differently because it returns void.  */
  TEST_extra (sincos, 0, 0, 1);

  TEST_extra (sincos, minus_zero, minus_zero, 1);
  TEST_extra (sincos, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_extra (sincos, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_extra (sincos, nan_value, nan_value, nan_value);

  TEST_extra (sincos, M_PI_2l, 1, 0);
  TEST_extra (sincos, M_PI_6l, 0.5, 0.86602540378443864676372317075293616L);
  TEST_extra (sincos, M_PI_6l*2.0, 0.86602540378443864676372317075293616L, 0.5);
  TEST_extra (sincos, 0.7L, 0.64421768723769105367261435139872014L, 0.76484218728448842625585999019186495L);

  END (sincos);
}
#endif

static void
sinh_test (void)
{
  errno = 0;
  FUNC(sinh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sinh);
  TEST_f_f (sinh, 0, 0);
  TEST_f_f (sinh, minus_zero, minus_zero);

#ifndef TEST_INLINE
  TEST_f_f (sinh, plus_infty, plus_infty);
  TEST_f_f (sinh, minus_infty, minus_infty);
#endif
  TEST_f_f (sinh, nan_value, nan_value);

  TEST_f_f (sinh, 0.7L, 0.75858370183953350346L);
  TEST_f_f (sinh, 0x8p-32L, 1.86264514923095703232705808926175479e-9L);

  END (sinh);
}

static void
sqrt_test (void)
{
  errno = 0;
  FUNC(sqrt) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sqrt);

  TEST_f_f (sqrt, 0, 0);
  TEST_f_f (sqrt, nan_value, nan_value);
  TEST_f_f (sqrt, plus_infty, plus_infty);

  TEST_f_f (sqrt, minus_zero, minus_zero);

  /* sqrt (x) == NaN plus invalid exception for x < 0.  */
  TEST_f_f (sqrt, -1, nan_value, INVALID_EXCEPTION);
  TEST_f_f (sqrt, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (sqrt, nan_value, nan_value);

  TEST_f_f (sqrt, 2209, 47);
  TEST_f_f (sqrt, 4, 2);
  TEST_f_f (sqrt, 2, M_SQRT2l);
  TEST_f_f (sqrt, 0.25, 0.5);
  TEST_f_f (sqrt, 6642.25, 81.5);
  TEST_f_f (sqrt, 15239.9025L, 123.45L);
  TEST_f_f (sqrt, 0.7L, 0.83666002653407554797817202578518747L);

  END (sqrt);
}

static void
tan_test (void)
{
  errno = 0;
  FUNC(tan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tan);

  TEST_f_f (tan, 0, 0);
  TEST_f_f (tan, minus_zero, minus_zero);
  TEST_f_f (tan, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (tan, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (tan, nan_value, nan_value);

  TEST_f_f (tan, M_PI_4l, 1);
  TEST_f_f (tan, 0.7L, 0.84228838046307944812813500221293775L);

  END (tan);
}

static void
tanh_test (void)
{
  errno = 0;
  FUNC(tanh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tanh);

  TEST_f_f (tanh, 0, 0);
  TEST_f_f (tanh, minus_zero, minus_zero);

#ifndef TEST_INLINE
  TEST_f_f (tanh, plus_infty, 1);
  TEST_f_f (tanh, minus_infty, -1);
#endif
  TEST_f_f (tanh, nan_value, nan_value);

  TEST_f_f (tanh, 0.7L, 0.60436777711716349631L);
  TEST_f_f (tanh, -0.7L, -0.60436777711716349631L);

  TEST_f_f (tanh, 1.0L, 0.7615941559557648881194582826047935904L);
  TEST_f_f (tanh, -1.0L, -0.7615941559557648881194582826047935904L);

  /* 2^-57  */
  TEST_f_f (tanh, 6.938893903907228377647697925567626953125e-18L,6.938893903907228377647697925567626953125e-18L);

  END (tanh);
}

#if 0
static void
tgamma_test (void)
{
  errno = 0;
  FUNC(tgamma) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;
  feclearexcept (FE_ALL_EXCEPT);

  START (tgamma);

  TEST_f_f (tgamma, plus_infty, plus_infty);
  TEST_f_f (tgamma, 0, nan_value, INVALID_EXCEPTION);
  TEST_f_f (tgamma, minus_zero, nan_value, INVALID_EXCEPTION);
  /* tgamma (x) == NaN plus invalid exception for integer x <= 0.  */
  TEST_f_f (tgamma, -2, nan_value, INVALID_EXCEPTION);
  TEST_f_f (tgamma, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (tgamma, nan_value, nan_value);

  TEST_f_f (tgamma, 0.5, M_SQRT_PIl);
  TEST_f_f (tgamma, -0.5, -M_2_SQRT_PIl);

  TEST_f_f (tgamma, 1, 1);
  TEST_f_f (tgamma, 4, 6);

  TEST_f_f (tgamma, 0.7L, 1.29805533264755778568L);
  TEST_f_f (tgamma, 1.2L, 0.91816874239976061064L);

  END (tgamma);
}
#endif

#if 0
static void
trunc_test (void)
{
  START (trunc);

  TEST_f_f (trunc, plus_infty, plus_infty);
  TEST_f_f (trunc, minus_infty, minus_infty);
  TEST_f_f (trunc, nan_value, nan_value);

  TEST_f_f (trunc, 0, 0);
  TEST_f_f (trunc, minus_zero, minus_zero);
  TEST_f_f (trunc, 0.625, 0);
  TEST_f_f (trunc, -0.625, minus_zero);
  TEST_f_f (trunc, 1, 1);
  TEST_f_f (trunc, -1, -1);
  TEST_f_f (trunc, 1.625, 1);
  TEST_f_f (trunc, -1.625, -1);

  TEST_f_f (trunc, 1048580.625L, 1048580L);
  TEST_f_f (trunc, -1048580.625L, -1048580L);

  TEST_f_f (trunc, 8388610.125L, 8388610.0L);
  TEST_f_f (trunc, -8388610.125L, -8388610.0L);

  TEST_f_f (trunc, 4294967296.625L, 4294967296.0L);
  TEST_f_f (trunc, -4294967296.625L, -4294967296.0L);


  END (trunc);
}
#endif

static void
y0_test (void)
{
  errno = 0;
#if 0
  FLOAT s, c;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
#endif
  FUNC(y0) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* y0 is the Bessel function of the second kind of order 0 */
  START (y0);

  TEST_f_f (y0, -1.0, minus_infty);
  TEST_f_f (y0, 0.0, minus_infty);
  TEST_f_f (y0, nan_value, nan_value);
  TEST_f_f (y0, plus_infty, 0);

  TEST_f_f (y0, 0.1L, -1.5342386513503668441L);
  TEST_f_f (y0, 0.7L, -0.19066492933739506743L);
  TEST_f_f (y0, 1.0, 0.088256964215676957983L);
  TEST_f_f (y0, 1.5, 0.38244892379775884396L);
  TEST_f_f (y0, 2.0, 0.51037567264974511960L);
  TEST_f_f (y0, 8.0, 0.22352148938756622053L);
  TEST_f_f (y0, 10.0, 0.055671167283599391424L);

  END (y0);
}


static void
y1_test (void)
{
  errno = 0;
#if 0
  FLOAT s, c;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
#endif
  FUNC(y1) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* y1 is the Bessel function of the second kind of order 1 */
  START (y1);

  TEST_f_f (y1, -1.0, minus_infty);
  TEST_f_f (y1, 0.0, minus_infty);
  TEST_f_f (y1, plus_infty, 0);
  TEST_f_f (y1, nan_value, nan_value);

  TEST_f_f (y1, 0.1L, -6.4589510947020269877L);
  TEST_f_f (y1, 0.7L, -1.1032498719076333697L);
  TEST_f_f (y1, 1.0, -0.78121282130028871655L);
  TEST_f_f (y1, 1.5, -0.41230862697391129595L);
  TEST_f_f (y1, 2.0, -0.10703243154093754689L);
  TEST_f_f (y1, 8.0, -0.15806046173124749426L);
  TEST_f_f (y1, 10.0, 0.24901542420695388392L);

  END (y1);
}

static void
yn_test (void)
{
  errno = 0;
#if 0
  FLOAT s, c;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
#endif
  FUNC(yn) (1, 1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* yn is the Bessel function of the second kind of order n */
  START (yn);

  /* yn (0, x) == y0 (x)  */
  TEST_ff_f (yn, 0, -1.0, minus_infty);
  TEST_ff_f (yn, 0, 0.0, minus_infty);
  TEST_ff_f (yn, 0, nan_value, nan_value);
  TEST_ff_f (yn, 0, plus_infty, 0);

  TEST_ff_f (yn, 0, 0.1L, -1.5342386513503668441L);
  TEST_ff_f (yn, 0, 0.7L, -0.19066492933739506743L);
  TEST_ff_f (yn, 0, 1.0, 0.088256964215676957983L);
  TEST_ff_f (yn, 0, 1.5, 0.38244892379775884396L);
  TEST_ff_f (yn, 0, 2.0, 0.51037567264974511960L);
  TEST_ff_f (yn, 0, 8.0, 0.22352148938756622053L);
  TEST_ff_f (yn, 0, 10.0, 0.055671167283599391424L);

  /* yn (1, x) == y1 (x)  */
  TEST_ff_f (yn, 1, -1.0, minus_infty);
  TEST_ff_f (yn, 1, 0.0, minus_infty);
  TEST_ff_f (yn, 1, plus_infty, 0);
  TEST_ff_f (yn, 1, nan_value, nan_value);

  TEST_ff_f (yn, 1, 0.1L, -6.4589510947020269877L);
  TEST_ff_f (yn, 1, 0.7L, -1.1032498719076333697L);
  TEST_ff_f (yn, 1, 1.0, -0.78121282130028871655L);
  TEST_ff_f (yn, 1, 1.5, -0.41230862697391129595L);
  TEST_ff_f (yn, 1, 2.0, -0.10703243154093754689L);
  TEST_ff_f (yn, 1, 8.0, -0.15806046173124749426L);
  TEST_ff_f (yn, 1, 10.0, 0.24901542420695388392L);

  /* yn (3, x)  */
  TEST_ff_f (yn, 3, plus_infty, 0);
  TEST_ff_f (yn, 3, nan_value, nan_value);

  TEST_ff_f (yn, 3, 0.1L, -5099.3323786129048894L);
  TEST_ff_f (yn, 3, 0.7L, -15.819479052819633505L);
  TEST_ff_f (yn, 3, 1.0, -5.8215176059647288478L);
  TEST_ff_f (yn, 3, 2.0, -1.1277837768404277861L);
  TEST_ff_f (yn, 3, 10.0, -0.25136265718383732978L);

  /* yn (10, x)  */
  TEST_ff_f (yn, 10, plus_infty, 0);
  TEST_ff_f (yn, 10, nan_value, nan_value);

  TEST_ff_f (yn, 10, 0.1L, -0.11831335132045197885e19L);
  TEST_ff_f (yn, 10, 0.7L, -0.42447194260703866924e10L);
  TEST_ff_f (yn, 10, 1.0, -0.12161801427868918929e9L);
  TEST_ff_f (yn, 10, 2.0, -129184.54220803928264L);
  TEST_ff_f (yn, 10, 10.0, -0.35981415218340272205L);

  END (yn);

}



static void
initialize (void)
{
  fpstack_test ("start *init*");
  plus_zero = 0.0;
  nan_value = plus_zero / plus_zero;	/* Suppress GCC warning */

  minus_zero = FUNC(copysign) (0.0, -1.0);
  plus_infty = CHOOSE (HUGE_VALL, HUGE_VAL, HUGE_VALF,
		       HUGE_VALL, HUGE_VAL, HUGE_VALF);
  minus_infty = CHOOSE (-HUGE_VALL, -HUGE_VAL, -HUGE_VALF,
			-HUGE_VALL, -HUGE_VAL, -HUGE_VALF);

  (void) &plus_zero;
  (void) &nan_value;
  (void) &minus_zero;
  (void) &plus_infty;
  (void) &minus_infty;

  /* Clear all exceptions.  From now on we must not get random exceptions.  */
  feclearexcept (FE_ALL_EXCEPT);

  /* Test to make sure we start correctly.  */
  fpstack_test ("end *init*");
}

#if 0
/* function to check our ulp calculation.  */
void
check_ulp (void)
{
  int i;

  FLOAT u, diff, ulp;
  /* This gives one ulp.  */
  u = FUNC(nextafter) (10, 20);
  check_equal (10.0, u, 1, &diff, &ulp);
  printf ("One ulp: % .4" PRINTF_NEXPR "\n", ulp);

  /* This gives one more ulp.  */
  u = FUNC(nextafter) (u, 20);
  check_equal (10.0, u, 2, &diff, &ulp);
  printf ("two ulp: % .4" PRINTF_NEXPR "\n", ulp);

  /* And now calculate 100 ulp.  */
  for (i = 2; i < 100; i++)
    u = FUNC(nextafter) (u, 20);
  check_equal (10.0, u, 100, &diff, &ulp);
  printf ("100 ulp: % .4" PRINTF_NEXPR "\n", ulp);
}
#endif

int
main (int argc, char **argv)
{

  int key;

  verbose = 1;
  output_ulps = 0;
  output_max_error = 1;
  output_points = 1;
  /* XXX set to 0 for releases.  */
  ignore_max_ulp = 0;

  /* Parse and process arguments.  */
  while ((key = getopt(argc, argv, "fi:puv")) > 0) {
      switch (key)
      {
	  case 'f':
	      output_max_error = 0;
	      break;
	  case 'i':
	      if (strcmp (optarg, "yes") == 0)
		  ignore_max_ulp = 1;
	      else if (strcmp (optarg, "no") == 0)
		  ignore_max_ulp = 0;
	      break;
	  case 'p':
	      output_points = 0;
	      break;
	  case 'u':
	      output_ulps = 1;
	      break;
	  case 'v':
	      verbose = 3;
	      break;
	  default:
	      fprintf (stderr, "Unknown argument: %c", key);
	      exit (EXIT_FAILURE);
      }
  }

  if (optind != argc)
    {
      fprintf (stderr, "wrong number of arguments");
      exit (EXIT_FAILURE);
    }

  if (output_ulps)
    {
      ulps_file = fopen ("ULPs", "a");
      if (ulps_file == NULL)
	{
	  perror ("can't open file `ULPs' for writing: ");
	  exit (1);
	}
    }


  initialize ();
  printf (TEST_MSG);

#if 0
  check_ulp ();
#endif

  /* Keep the tests a wee bit ordered (according to ISO C99).  */
  /* Classification macros:  */
  fpclassify_test ();
  isfinite_test ();
  isnormal_test ();
  signbit_test ();

  /* Trigonometric functions:  */
  acos_test ();
  asin_test ();
  atan_test ();
  atan2_test ();
  cos_test ();
  sin_test ();
#if 0
  sincos_test ();
#endif
  tan_test ();

  /* Hyperbolic functions:  */
  acosh_test ();
  asinh_test ();
  atanh_test ();
  cosh_test ();
  sinh_test ();
  tanh_test ();

  /* Exponential and logarithmic functions:  */
  exp_test ();
#if 0
  exp10_test ();
  exp2_test ();
#endif
  expm1_test ();
  frexp_test ();
  ldexp_test ();
  log_test ();
  log10_test ();
  log1p_test ();
#if 0
  log2_test ();
#endif
  logb_test ();
  modf_test ();
  ilogb_test ();
  scalb_test ();
  scalbn_test ();
#if 0
  scalbln_test ();
#endif

  /* Power and absolute value functions:  */
  cbrt_test ();
  fabs_test ();
  hypot_test ();
  pow_test ();
  sqrt_test ();

  /* Error and gamma functions:  */
  erf_test ();
  erfc_test ();
  gamma_test ();
  lgamma_test ();
#if 0
  tgamma_test ();
#endif

  /* Nearest integer functions:  */
  ceil_test ();
  floor_test ();
#if 0
  nearbyint_test ();
#endif
  rint_test ();
#if 0
  lrint_test ();
  llrint_test ();
  round_test ();
  lround_test ();
  llround_test ();
  trunc_test ();
#endif

  /* Remainder functions:  */
  fmod_test ();
  remainder_test ();
#if 0
  remquo_test ();
#endif

  /* Manipulation functions:  */
  copysign_test ();
#if 0
  nextafter_test ();
  nexttoward_test ();

  /* maximum, minimum and positive difference functions */
  fdim_test ();
  fmax_test ();
  fmin_test ();

  /* Multiply and add:  */
  fma_test ();

  /* Complex functions:  */
  cabs_test ();
  cacos_test ();
  cacosh_test ();
  carg_test ();
  casin_test ();
  casinh_test ();
  catan_test ();
  catanh_test ();
  ccos_test ();
  ccosh_test ();
  cexp_test ();
  cimag_test ();
  clog10_test ();
  clog_test ();
  conj_test ();
  cpow_test ();
  cproj_test ();
  creal_test ();
  csin_test ();
  csinh_test ();
  csqrt_test ();
  ctan_test ();
  ctanh_test ();
#endif

  /* Bessel functions:  */
#if 0
  j0_test ();
  j1_test ();
  jn_test ();
  y0_test ();
  y1_test ();
  yn_test ();
#endif

  if (output_ulps)
    fclose (ulps_file);

  printf ("\nTest suite completed:\n");
  printf ("  %d test cases plus %d tests for exception flags executed.\n",
	  noTests, noExcTests);
  if (noXFails)
    printf ("  %d expected failures occurred.\n", noXFails);
  if (noXPasses)
    printf ("  %d unexpected passes occurred.\n", noXPasses);
  if (noErrors)
    {
      printf ("  %d errors occurred.\n", noErrors);
      return 1;
    }
  printf ("  All tests passed successfully.\n");

  return 0;
}

/*
 * Local Variables:
 * mode:c
 * End:
 */
