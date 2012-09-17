/* vi: set sw=4 ts=4: */
/*
 * vfork test for uClibc
 *
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
