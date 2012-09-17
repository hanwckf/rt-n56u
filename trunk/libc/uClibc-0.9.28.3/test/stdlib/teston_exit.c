/*
 * This test program will register the maximum number of exit functions
 * with on_exit().  When this program exits, each exit function should get
 * called in the reverse order in which it was registered.  (If the system
 * supports more than 25 exit functions, the function names will loop, but
 * the effect will be the same.  Feel free to add more functions if desired)
 */
#include <stdio.h>
#include <stdlib.h>

typedef void (*efuncp) (int, void *);

/* All functions call exit(), in order to test that exit functions can call
 * exit() without screwing everything up. The value passed in through arg gets
 * used as the next exit status.
 */
static void exitfunc0(int status, void *arg) { printf("Executing exitfunc0 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc1(int status, void *arg) { printf("Executing exitfunc1 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc2(int status, void *arg) { printf("Executing exitfunc2 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc3(int status, void *arg) { printf("Executing exitfunc3 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc4(int status, void *arg) { printf("Executing exitfunc4 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc5(int status, void *arg) { printf("Executing exitfunc5 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc6(int status, void *arg) { printf("Executing exitfunc6 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc7(int status, void *arg) { printf("Executing exitfunc7 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc8(int status, void *arg) { printf("Executing exitfunc8 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc9(int status, void *arg) { printf("Executing exitfunc9 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc10(int status, void *arg) { printf("Executing exitfunc10 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc11(int status, void *arg) { printf("Executing exitfunc11 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc12(int status, void *arg) { printf("Executing exitfunc12 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc13(int status, void *arg) { printf("Executing exitfunc13 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc14(int status, void *arg) { printf("Executing exitfunc14 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc15(int status, void *arg) { printf("Executing exitfunc15 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc16(int status, void *arg) { printf("Executing exitfunc16 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc17(int status, void *arg) { printf("Executing exitfunc17 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc18(int status, void *arg) { printf("Executing exitfunc18 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc19(int status, void *arg) { printf("Executing exitfunc19 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc20(int status, void *arg) { printf("Executing exitfunc20 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc21(int status, void *arg) { printf("Executing exitfunc21 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc22(int status, void *arg) { printf("Executing exitfunc22 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc23(int status, void *arg) { printf("Executing exitfunc23 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}
static void exitfunc24(int status, void *arg) { printf("Executing exitfunc24 (status=%d, arg=%lu)\n", status, (unsigned long)arg); exit((unsigned long)arg);}

static efuncp func_table[] =
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
#define ON_EXIT_LIMIT 20

int
main ( void )
{
	int i = 0;
	unsigned long count = 0;
	int numfuncs = sizeof(func_table)/sizeof(efuncp);

	/* loop until no more can be added */
	while(count < ON_EXIT_LIMIT && on_exit(func_table[i], (void *)count) >= 0) {
		count++;
		printf("Registered exitfunc%d with on_exit()\n", i);
		i = (i+1) % numfuncs;
	}
	printf("%lu functions registered with on_exit.\n", count);
	exit(count);
}

