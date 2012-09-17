#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>

#define check_d1(func, param, expected) \
do { \
	int err; hex_union ur; hex_union up; \
	double result = func(param); up.f = param; ur.f = result; \
	errors += (err = (result != (expected))); \
	err \
	? printf("FAIL: %s(%g/"HEXFMT")=%g/"HEXFMT" (expected %g)\n", \
		#func, (double)(param), (long long)up.hex, result, (long long)ur.hex, (double)(expected)) \
	: printf("PASS: %s(%g)=%g\n", #func, (double)(param), result); \
} while (0)

#define check_i1(func, param, expected) \
do { \
	int err; hex_union up; \
	long long result = func(param); up.f = param; \
	errors += (err = (result != (expected))); \
	err \
	? printf("FAIL: %s(%g/"HEXFMT")=%lld/%llu (expected %llu)\n", \
		#func, (double)(param), (long long)up.hex, result, result, (long long)(expected)) \
	: printf("PASS: %s(%g)=%lld/%llu\n", #func, (double)(param), result, result); \
} while (0)

#define HEXFMT "%08llx"
typedef union {
	double f;
	uint64_t hex;
} hex_union;

double zero = 0.0;
double minus_zero = 0.0;
double nan_value = 0.0;
int errors = 0;

int main(void)
{
	nan_value /= nan_value;
	minus_zero = copysign(zero, -1.0);

	check_i1(isfinite, 1.0, 1);
	check_i1(isfinite, 2.0, 1);
	check_i1(isfinite, 3.0, 1);
	check_i1(isfinite, DBL_MAX, 1);
	check_i1(isfinite, FLT_MAX, 1);
	check_i1(isfinite, HUGE_VAL, 0);
	check_i1(isfinite, HUGE_VALF, 0);
	check_i1(isfinite, HUGE_VALL, 0);
	check_i1(isfinite, nan_value, 0);
	check_i1(isfinite, nan_value, 0);
	check_i1(isfinite, nan_value, 0);

	check_i1(isnan, 1.0, 0);
	check_i1(isnan, 2.0, 0);
	check_i1(isnan, 3.0, 0);
	check_i1(isnan, DBL_MAX, 0);
	check_i1(isnan, FLT_MAX, 0);
	check_i1(isnan, HUGE_VAL, 0);
	check_i1(isnan, HUGE_VALF, 0);
	check_i1(isnan, HUGE_VALL, 0);
	check_i1(isnan, (float)HUGE_VALL, 0);
	check_i1(isnan, nan_value, 1);
	check_i1(isnan, nan_value, 1);
	check_i1(isnan, nan_value, 1);

	check_i1(isinf, 1.0, 0);
	check_i1(isinf, 2.0, 0);
	check_i1(isinf, 3.0, 0);
	check_i1(isinf, DBL_MAX, 0);
	check_i1(isinf, FLT_MAX, 0);
	check_i1(isinf, (float)DBL_MAX, 1);
	check_i1(isinf, HUGE_VAL, 1);
	check_i1(isinf, HUGE_VALF, 1);
	check_i1(isinf, HUGE_VALL, 1);
	check_i1(isinf, (float)HUGE_VALL, 1);
	check_i1(isinf, nan_value, 0);
	check_i1(isinf, nan_value, 0);
	check_i1(isinf, nan_value, 0);

	check_i1(fpclassify, minus_zero, FP_ZERO);
	check_i1(fpclassify, 0.0, FP_ZERO);
	check_i1(fpclassify, 1.0, FP_NORMAL);
	check_i1(fpclassify, 2.0, FP_NORMAL);
	check_i1(fpclassify, 3.0, FP_NORMAL);
	check_i1(fpclassify, DBL_MIN/1.01, FP_SUBNORMAL);
	check_i1(fpclassify, DBL_MIN, FP_NORMAL);
	check_i1(fpclassify, DBL_MAX, FP_NORMAL);
	check_i1(fpclassify, FLT_MAX, FP_NORMAL);
	check_i1(fpclassify, DBL_MAX, FP_NORMAL);
	check_i1(fpclassify, DBL_MAX*1.01, FP_INFINITE);
	check_i1(fpclassify, HUGE_VAL, FP_INFINITE);
	check_i1(fpclassify, HUGE_VALF, FP_INFINITE);
	check_i1(fpclassify, HUGE_VALL, FP_INFINITE);
	check_i1(fpclassify, (float)HUGE_VALL, FP_INFINITE);
	check_i1(fpclassify, nan_value, FP_NAN);
	check_i1(fpclassify, nan_value, FP_NAN);
	check_i1(fpclassify, nan_value, FP_NAN);

	check_i1(!!signbit, -1.0, 1);
	check_i1(!!signbit, minus_zero, 1);
	check_i1(!!signbit, 0.0, 0);
	check_i1(!!signbit, HUGE_VAL, 0);
	check_i1(!!signbit, HUGE_VALF, 0);
	check_i1(!!signbit, HUGE_VALL, 0);
	check_i1(!!signbit, -HUGE_VAL, 1);
	check_i1(!!signbit, -HUGE_VALF, 1);
	check_i1(!!signbit, -HUGE_VALL, 1);

	printf("Errors: %d\n", errors);
	return errors;
}
