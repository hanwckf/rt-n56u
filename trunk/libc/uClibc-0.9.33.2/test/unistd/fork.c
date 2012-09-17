/* vi: set sw=4 ts=4: */
/*
 * fork test for uClibc
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define GOT1	(1 << 1)
#define GOT2	(1 << 2)
#define GOT3	(1 << 3)

#ifdef __ARCH_USE_MMU__

static void child_handler(int sig)
{
	fprintf(stderr, "I got a SIGCHLD\n");
}

int main(void)
{
	pid_t pid1, pid2, pid3;
	int status, result, wpid;

	signal(SIGCHLD, child_handler);

	if ((pid1 = fork()) == 0) {
		fprintf(stderr, "The child process sleeps 2 seconds...\n");
		sleep(4);
		fprintf(stderr, "Child exiting.\n");
		exit(-1);
	}
	if ((pid2 = fork()) == 0) {
		fprintf(stderr, "The child process sleeps 3 seconds...\n");
		sleep(3);
		fprintf(stderr, "Child exiting.\n");
		exit(-1);
	}
	if ((pid3 = fork()) == 0) {
		fprintf(stderr, "The child process sleeps 4 seconds...\n");
		sleep(2);
		fprintf(stderr, "Child exiting.\n");
		exit(-1);
	}

	fprintf(stderr, "Parent: waiting for the child to die.\n");
	status = 0;
	while (1) {
		wpid = waitpid(pid1, &result, WNOHANG);
		if (wpid == pid1)
			status |= GOT1;

		wpid = waitpid(pid2, &result, WNOHANG);
		if (wpid == pid2)
			status |= GOT2;

		wpid = waitpid(pid3, &result, WNOHANG);
		if (wpid == pid3)
			status |= GOT3;

		if (status == (GOT1 | GOT2 | GOT3))
			break;
	}

	fprintf(stderr, "Child process exited.\nGoodbye.\n");
	return EXIT_SUCCESS;
}

#else

int main(void)
{
	printf("Skipping test on non-mmu host!\n");
	return EXIT_SUCCESS;
}

#endif

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
