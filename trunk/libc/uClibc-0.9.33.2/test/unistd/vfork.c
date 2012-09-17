/* vi: set sw=4 ts=4: */
/*
 * vfork test for uClibc
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


int main(void)
{
	pid_t pid;
	int status, wpid;
	char *argv[] = {
		"/bin/ls",
		"-laF",
		NULL,
	};

	clearenv();
	if ((pid = vfork()) == 0) {
		printf("Hi.  I'm the child process...\n");
		execvp(argv[0], argv);
		_exit(0);
	}

	printf("Hello.  I'm the parent process.\n");
	while (1) {
		wpid = wait(&status);
		if (wpid > 0 && wpid != pid) {
			continue;
		}
		if (wpid == pid)
			break;
	}

	printf("Child process exited.\nGoodbye.\n");
	return EXIT_SUCCESS;
}

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
