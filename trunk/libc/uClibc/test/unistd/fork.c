/* vi: set sw=4 ts=4: */
/*
 * fork test for uClibc
 *
 * Copyright (C) 2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 2000,2001 by Erik Andersen <andersen@uclibc.org>
 * Written by Erik Andersen <andersen@uclibc.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define GOT1	(1 << 1)
#define GOT2	(1 << 2)
#define GOT3	(1 << 3)

void child_handler(int sig)
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

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
