/* Copyright (C) 2009 Mikael Lund Jepsen <mlj@iccc.dk>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

char shared_name[] = "/sharetest";
int test_data[11] = {0,1,2,3,4,5,6,7,8,9,10};

int main(void) {
	int pfds[2];
	pid_t pid;
	int fd;
	int test_data_fails = 0;
	char *ptest_data;
	unsigned int i;
	char buf[30];
	int rv;

	pipe(pfds);

	switch(pid = fork()) {
	case -1:
		perror("fork");
		exit(1);	/* parent exits */

	case 0:
		/* Child */

		/* wait for parent */
		read(pfds[0], buf, 5);

		fd =  shm_open(shared_name, O_RDWR, DEFFILEMODE);
		if (fd == -1) {
			perror("CHILD - shm_open(existing):");
			exit(1);
		} else {
			ptest_data = mmap(0, sizeof(test_data), PROT_READ + PROT_WRITE, MAP_SHARED, fd, 0);
			if (ptest_data != MAP_FAILED) {
				for (i=0; i < ARRAY_SIZE(test_data); i++) {
					if (ptest_data[i] != test_data[i]) {
						printf("%-40s: Offset %d, local %d, shm %d\n", "Compare memory error", i, test_data[i], ptest_data[i]);
						test_data_fails++;
					}
				}
				if (test_data_fails == 0)
					printf("%-40s: %s\n", "Compare memory", "Success");

				munmap(ptest_data, sizeof(test_data));
			}
		}
		exit(0);

	default:
		/* Parent */
		fd = shm_open(shared_name, O_RDWR+O_CREAT+O_EXCL, DEFFILEMODE );
		if (fd == -1) {
			perror("PARENT - shm_open(create):");
		} else {
			if ((ftruncate(fd, sizeof(test_data))) == -1)
			{
				printf("%-40s: %s", "ftruncate", strerror(errno));
				shm_unlink(shared_name);
				return 0;
			}

			ptest_data = mmap(0, sizeof(test_data), PROT_READ + PROT_WRITE, MAP_SHARED, fd, 0);
			if (ptest_data == MAP_FAILED)
			{
				perror("PARENT - mmap:");
				if (shm_unlink(shared_name) == -1) {
					perror("PARENT - shm_unlink:");
				}
				return 0;
			}
			for (i=0; i < ARRAY_SIZE(test_data); i++)
				ptest_data[i] = test_data[i];

			/* signal child */
			write(pfds[1], "rdy", 5);
			/* wait for child */
			wait(&rv);

			/* Cleanup */
			munmap(ptest_data, sizeof(test_data));
			if (shm_unlink(shared_name) == -1) {
				perror("PARENT - shm_unlink:");
			}
		}
	}
	return 0;
}
