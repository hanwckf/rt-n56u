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

double nan_value = 0.0;
int errors = 0;

int main(void)
{
	nan_value /= nan_value;

	check_i1(ilogb, 0.0, FP_ILOGB0);
	check_i1(ilogb, HUGE_VAL, INT_MAX);
	check_i1(ilogb, nan_value, FP_ILOGBNAN);
	check_i1(ilogbf, 0.0, FP_ILOGB0);
	check_i1(ilogbf, HUGE_VALF, INT_MAX);
	check_i1(ilogbf, nan_value, FP_ILOGBNAN);

	printf("Errors: %d\n", errors);
	return errors;
}
