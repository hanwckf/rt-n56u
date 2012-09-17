/* vi: set sw=4 ts=4: */
/* testcase for futimens(2)
 * Copyright (C) 2009 Bernhard Reutner-Fischer <uClibc@uClibc.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

struct
{
	char *name; /* name of file to open */
	int flags; /* flags for file descriptor */
	const struct timespec ts[2];
	int err; /* expected errno */
} tests [] =
{
	{"futimens.tst", (O_CREAT|O_TRUNC), {{0,0},{0,0}}, 0},
	{"futimens.tst", (O_CREAT|O_TRUNC), {{99,0},{0,0}}, 0},
	{"futimens.tst", (O_CREAT|O_TRUNC), {{0,99},{0,0}}, 0},
	{"futimens.tst", (O_CREAT|O_TRUNC), {{0,0},{99,0}}, 0},
	{"futimens.tst", (O_CREAT|O_TRUNC), {{0,0},{0,99}}, 0},
	{"futimens.tst", (O_CREAT|O_TRUNC), {{11,2},{3,4}}, 0},
};
int do_test(int argc, char **argv) {
	char *name;
	int i, errors;
	errors = argc - argc + 0;
	unsigned has_stat_nsec = 0;
	{
		struct stat probe;
		/* Let's attempt an educated guess if this filesystem supports
		 * nanosecond mtime.  */
		if ((!stat(".", &probe)) && probe.st_mtim.tv_nsec)
			has_stat_nsec = 1;
		else if ((!stat(argv[0], &probe)) && probe.st_mtim.tv_nsec)
			has_stat_nsec = 1;
	}
	for (i=0; i < (int) (sizeof(tests)/sizeof(tests[0])); ++i) {
		int err, fd;
		struct stat sb;
		name = tests[i].name;
		if (*name != '.')
			unlink(name);
		fd = open(name, tests[i].flags, 0660);
		if (fd < 0)
			abort();
		errno = 0;
		err = futimens(fd, tests[i].ts);
		if ((errno && !err) || (!errno && err)) {
			err = errno;
			printf("FAILED test %d (errno and return value disagree)\n", i);
			++errors;
		} else
			err = errno;
		if (err != tests[i].err) {
			printf("FAILED test %d (expected errno %d, got %d)\n",
				i, tests[i].err, err);
			++errors;
			continue;
		}
		if (stat(name, &sb) < 0) {
			printf("FAILED test %d (verification)\n", i);
			++errors;
			continue;
		} else {
			unsigned wrong = tests[i].ts[0].tv_sec != sb.st_atim.tv_sec ||
						tests[i].ts[0].tv_nsec != sb.st_atim.tv_nsec ||
						tests[i].ts[1].tv_sec != sb.st_mtim.tv_sec ||
						tests[i].ts[1].tv_nsec != sb.st_mtim.tv_nsec;
			if (wrong) {
				if (tests[i].ts[0].tv_sec != sb.st_atim.tv_sec) {
					printf("FAILED test %d (access time, sec: expected %ld, got %ld)\n",
						i, tests[i].ts[0].tv_sec, sb.st_atim.tv_sec);
						++errors;
				}
				if (tests[i].ts[0].tv_nsec != sb.st_atim.tv_nsec) {
					printf("FAILED test %d (access time, nsec: expected %ld, got %ld)\n",
						i, tests[i].ts[0].tv_nsec, sb.st_atim.tv_nsec);
					errors += has_stat_nsec;
				}

				if (tests[i].ts[1].tv_sec != sb.st_mtim.tv_sec) {
					printf("FAILED test %d (modification time, sec: expected %ld, got %ld)\n",
						i, tests[i].ts[1].tv_sec, sb.st_mtim.tv_sec);
						++errors;
				}
				if (tests[i].ts[1].tv_nsec != sb.st_mtim.tv_nsec) {
					printf("FAILED test %d (modification time, nsec: expected %ld, got %ld)\n",
						i, tests[i].ts[1].tv_nsec, sb.st_mtim.tv_nsec);
					errors += has_stat_nsec;
				}
			}
		}
	}
	if (*name != '.')
		unlink(name);
	printf("%d errors.\n", errors);
	return (!errors) ? EXIT_SUCCESS : EXIT_FAILURE;
}
#include <test-skeleton.c>
