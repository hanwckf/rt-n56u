/*
 * This test program will register the maximum number of exit functions
 * with atexit().  When this program exits, each exit function should get
 * called in the reverse order in which it was registered.  (If the system
 * supports more than 25 exit functions, the function names will loop, but
 * the effect will be the same.  Feel free to add more functions if desired)
 */
#include <stdio.h>
#include <stdlib.h>

typedef void (*vfuncp) (void);

/* All functions call exit(), in order to test that exit functions can call
 * exit() without screwing everything up. :)
 */
static void exitfunc0(void) { printf("Executing exitfunc0.\n"); exit(0);}
static void exitfunc1(void) { printf("Executing exitfunc1.\n"); exit(0);}
static void exitfunc2(void) { printf("Executing exitfunc2.\n"); exit(0);}
static void exitfunc3(void) { printf("Executing exitfunc3.\n"); exit(0);}
static void exitfunc4(void) { printf("Executing exitfunc4.\n"); exit(0);}
static void exitfunc5(void) { printf("Executing exitfunc5.\n"); exit(0);}
static void exitfunc6(void) { printf("Executing exitfunc6.\n"); exit(0);}
static void exitfunc7(void) { printf("Executing exitfunc7.\n"); exit(0);}
static void exitfunc8(void) { printf("Executing exitfunc8.\n"); exit(0);}
static void exitfunc9(void) { printf("Executing exitfunc9.\n"); exit(0);}
static void exitfunc10(void) { printf("Executing exitfunc10.\n"); exit(0);}
static void exitfunc11(void) { printf("Executing exitfunc11.\n"); exit(0);}
static void exitfunc12(void) { printf("Executing exitfunc12.\n"); exit(0);}
static void exitfunc13(void) { printf("Executing exitfunc13.\n"); exit(0);}
static void exitfunc14(void) { printf("Executing exitfunc14.\n"); exit(0);}
static void exitfunc15(void) { printf("Executing exitfunc15.\n"); exit(0);}
static void exitfunc16(void) { printf("Executing exitfunc16.\n"); exit(0);}
static void exitfunc17(void) { printf("Executing exitfunc17.\n"); exit(0);}
static void exitfunc18(void) { printf("Executing exitfunc18.\n"); exit(0);}
static void exitfunc19(void) { printf("Executing exitfunc19.\n"); exit(0);}
static void exitfunc20(void) { printf("Executing exitfunc20.\n"); exit(0);}
static void exitfunc21(void) { printf("Executing exitfunc21.\n"); exit(0);}
static void exitfunc22(void) { printf("Executing exitfunc22.\n"); exit(0);}
static void exitfunc23(void) { printf("Executing exitfunc23.\n"); exit(0);}
static void exitfunc24(void) { printf("Executing exitfunc24.\n"); exit(0);}

static vfuncp func_table[] =
	{
	exitfunc0, exitfunc1, exitfunc2, exitfunc3, exitfunc4,
	exitfunc5, exitfunc6, exitfunc7, exitfunc8, exitfunc9,
	exitfunc10, exitfunc11, exitfunc12, exitfunc13, exitfunc14,
	exitfunc15, exitfunc16, exitfunc17, exitfunc18, exitfunc19,
	exitfunc20, exitfunc21, exitfunc22, exitfunc23, exitfunc24
	};

/* glibc dynamically adds exit functions, so it will keep adding until
 * it runs out of memory!  So this will limit the number of exit functions
 * we add in the loop below.  uClibc has a set limit (currently 20), so the
 * loop will go until it can't add any more (so it should not hit this limit).
 */
#define ATEXIT_LIMIT 20

int
main ( void )
{
	int i = 0;
	int count = 0;
	int numfuncs = sizeof(func_table)/sizeof(vfuncp);

	/* loop until no more can be added */
	while(count < ATEXIT_LIMIT && atexit(func_table[i]) >= 0) {
		printf("Registered exitfunc%d with atexit()\n", i);
		count++;
		i = (i+1) % numfuncs;
	}
	printf("%d functions registered with atexit.\n", count);

	return 0;
}

