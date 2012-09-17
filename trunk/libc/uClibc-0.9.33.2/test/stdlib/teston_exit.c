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
#define make_exitfunc(num) \
__attribute__ ((__noreturn__)) static \
void exitfunc##num(int status, void *arg) \
{ \
	printf("Executing exitfunc"#num" (status=%d, arg=%lu)\n", status, (unsigned long)arg); \
	exit((unsigned long)arg); \
}
make_exitfunc(0)
make_exitfunc(1)
make_exitfunc(2)
make_exitfunc(3)
make_exitfunc(4)
make_exitfunc(5)
make_exitfunc(6)
make_exitfunc(7)
make_exitfunc(8)
make_exitfunc(9)
make_exitfunc(10)
make_exitfunc(11)
make_exitfunc(12)
make_exitfunc(13)
make_exitfunc(14)
make_exitfunc(15)
make_exitfunc(16)
make_exitfunc(17)
make_exitfunc(18)
make_exitfunc(19)
make_exitfunc(20)
make_exitfunc(21)
make_exitfunc(22)
make_exitfunc(23)
make_exitfunc(24)

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

