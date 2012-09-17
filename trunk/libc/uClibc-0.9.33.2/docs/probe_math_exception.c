/* Small test program for probing how various math functions
 * with specific operands set floating point exceptions
 */

#define _ISOC99_SOURCE 1
#define _GNU_SOURCE    1

#include <stdint.h>
#include <math.h>
#include <fenv.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	float largest, small, t, inf_float;

	largest = small = 1;
	while (1) {
		t = largest + small;
		/* optimizations may make plain "t == largest" unreliable */
		if (memcmp(&t, &largest, sizeof(float)) == 0)
			break;
		if (isfinite(t)) {
			largest = t;
			small *= 2;
			continue;
		}
		small /= 2;
	}
	inf_float = largest + largest;
	//printf("%.40g ", largest);
	//printf("[%llx]\n", (long long) (*(uint32_t *)&largest));

	feclearexcept(FE_ALL_EXCEPT);

	//t = 1.0 / 0.0; // simple test: FE_DIVBYZERO
	//t = nextafterf(largest, 1); // glibc 2.8: no math exceptions raised
	//t = nextafterf(largest, largest); // glibc 2.8: no math exceptions raised
	//t = nextafterf(largest, inf_float); // glibc 2.8: FE_INEXACT FE_OVERFLOW

#define PREX(ex) do { if (fetestexcept(ex)) printf(#ex " "); } while(0)
#ifdef FE_INEXACT
	PREX(FE_INEXACT);
#endif
#ifdef FE_DIVBYZERO
	PREX(FE_DIVBYZERO);
#endif
#ifdef FE_UNDERFLOW
	PREX(FE_UNDERFLOW);
#endif
#ifdef FE_OVERFLOW
	PREX(FE_OVERFLOW);
#endif
#ifdef FE_INVALID
	PREX(FE_INVALID);
#endif
	if (fetestexcept(FE_ALL_EXCEPT))
		printf("\n");
	else
		printf("no math exceptions raised\n");

	printf("%.40g\n", t);
	return 0;
}
