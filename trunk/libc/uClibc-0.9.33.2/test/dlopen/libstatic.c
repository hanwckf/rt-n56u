#include <stdio.h>

static int global_static = -1;

int static_test(void)
{
	static int local_static = -2;

	if (global_static != -1)
		printf("FAIL: global_static is not -1\n");
	if (local_static != -2)
		printf("FAIL: local_static is not -2\n");

	return (global_static == -1 && local_static == -2);
}
