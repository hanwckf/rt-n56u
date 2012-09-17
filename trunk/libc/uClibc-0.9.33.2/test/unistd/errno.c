/* based originally on one the clone tests in the LTP */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include "clone_cruft.h"

__attribute__ ((__noreturn__))
static int child_fn(void *arg)
{
	fprintf(stderr, "in child_fn\n");
	exit(1);
}

int main(void)
{
	int r_clone, ret_errno;

	r_clone = do_clone(child_fn, NULL, 0, NULL);
	ret_errno = errno;
	if (ret_errno != EINVAL || r_clone != -1) {
		fprintf(stderr, "clone: res=%d (wanted -1) errno=%d (wanted %d)\n",
			r_clone, errno, EINVAL);
		return 1;
	}

	return 0;
}
