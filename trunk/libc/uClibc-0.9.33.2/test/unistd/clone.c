/* vi: set sw=4 ts=4: */
/*
 * clone test for uClibc
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <sys/wait.h>
#include "clone_cruft.h"

#define GOT1     (1 << 1)
#define GOT2     (1 << 2)
#define GOT3     (1 << 3)
#define ALLGOT   (GOT1|GOT2|GOT3)

static void child_handler(int sig)
{
	printf("I got a SIGCHLD\n");
}

static int clone_main(void *arg)
{
	unsigned long input = (unsigned long)arg;
	int secs = (input / 10) * 4;
	printf("Clone got %lu, sleeping for %i secs\n", input, secs);
	sleep(secs);
	return input + 20;
}

int main(void)
{
	int clone1, clone2, clone3;
	char clone1_stack[8192], clone2_stack[8192], clone3_stack[8192];
	int status, nostatus, result, wpid;

	signal(SIGCHLD, child_handler);

	if ((clone1 = do_clone(clone_main, clone1_stack, 0, (void*)11)) == -1) {
		perror("Clone 1 failed");
		exit(-1);
	}
	if ((clone2 = do_clone(clone_main, clone2_stack, 0, (void*)22)) == -1) {
		perror("Clone 2 failed");
		exit(-2);
	}
	if ((clone3 = do_clone(clone_main, clone3_stack, 0, (void*)33)) == -1) {
		perror("Clone 3 failed");
		exit(-3);
	}

	sleep(1);
	printf("Parent: waiting for the clones to die.\n");
	nostatus = status = 0;
	while (1) {
		if ((wpid = waitpid(clone1, &result, WNOHANG|__WCLONE)) == -1)
			nostatus |= GOT1;
		if (wpid == clone1) {
			status |= GOT1;
			printf("Clone1 gave back %i\n", WEXITSTATUS(result));
		}

		if ((wpid = waitpid(clone2, &result, WNOHANG|__WCLONE)) == -1)
			nostatus |= GOT2;
		if (wpid == clone2) {
			status |= GOT2;
			printf("Clone2 gave back %i\n", WEXITSTATUS(result));
		}

		if ((wpid = waitpid(clone3, &result, WNOHANG|__WCLONE)) == -1)
			nostatus |= GOT3;
		if (wpid == clone3) {
			status |= GOT3;
			printf("Clone3 gave back %i\n", WEXITSTATUS(result));
		}

		if (status == ALLGOT || nostatus == ALLGOT)
			break;
	}

	if (status == ALLGOT) {
		printf("Clones exited.\nGoodbye.\n");
		return EXIT_SUCCESS;
	} else {
		perror("Waiting for clones failed");
		return EXIT_FAILURE;
	}
}

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
