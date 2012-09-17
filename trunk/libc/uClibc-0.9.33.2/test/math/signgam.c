#define _XOPEN_SOURCE 600
#include <math.h>
#include <float.h>
#include <stdio.h>

double zero = 0.0;
double mzero;

int main(void)
{
	double d;
	int errors = 0;

	mzero = copysign(zero, -1.0);

	d = lgamma(zero);
	printf("%g %d\n", d, signgam);
	errors += !(d == HUGE_VAL);
	errors += !(signgam == 1);

	d = lgamma(mzero);
	printf("%g %d\n", d, signgam);
	errors += !(d == HUGE_VAL);
	errors += !(signgam == -1);

	printf("Errors: %d\n", errors);
	return errors;
}
