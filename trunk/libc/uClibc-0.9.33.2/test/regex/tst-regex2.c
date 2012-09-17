#define _GNU_SOURCE 1

#include <fcntl.h>
#include <locale.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

static int
do_test(void)
{
	static const char *pat[] = {
		".?.?.?.?.?.?.?Log\\.13",
		"(.?)(.?)(.?)(.?)(.?)(.?)(.?)Log\\.13",
		"((((((((((.?))))))))))((((((((((.?))))))))))((((((((((.?))))))))))"
		"((((((((((.?))))))))))((((((((((.?))))))))))((((((((((.?))))))))))"
		"((((((((((.?))))))))))Log\\.13"
	};
	char *buf, *string;
	const char *fname = "tst-regex2.dat";
	struct stat st;
	unsigned len;
	int testno;
	int exitcode = 0;

	int fd = open(fname, O_RDONLY);
	if (fd < 0) {
		printf("Couldn't open %s: %s\n", fname, strerror(errno));
		return 1;
	}
	if (fstat(fd, &st) < 0) {
		printf("Couldn't fstat %s: %s\n", fname, strerror(errno));
		return 1;
	}
	len = st.st_size;
	string = buf = malloc(len + 1);
	if (buf == NULL) {
		printf("Couldn't allocate %u bytes\n", len + 1);
		return 1;
	}
	if (read(fd, buf, st.st_size) != (ssize_t) st.st_size) {
		printf("Couldn't read %s\n", fname);
		return 1;
	}

	close(fd);
	buf[len] = '\0';

#if defined __UCLIBC_HAS_XLOCALE__ || !defined __UCLIBC__
	setlocale(LC_ALL, "de_DE.UTF-8");
#endif

	for (testno = 0; testno < 2; ++testno) {
		int i;
		for (i = 0; i < sizeof(pat) / sizeof(pat[0]); ++i) {
			struct timeval start, stop;
			regex_t rbuf;
			int err;

			printf("test %d pattern %d '%s'\n", testno, i, pat[i]);
			gettimeofday(&start, NULL);

			err = regcomp(&rbuf, pat[i],
				REG_EXTENDED | (testno ? REG_NOSUB : 0));
			if (err != 0) {
				char errstr[300];
				regerror(err, &rbuf, errstr, sizeof(errstr));
				puts(errstr);
				exitcode = 1;
				goto contin1;
			}

			regmatch_t pmatch[71];
			err = regexec(&rbuf, string, 71, pmatch, 0);
			if (err == REG_NOMATCH) {
				puts("regexec failed");
				exitcode = 1;
				goto contin1;
			}

			if (testno == 0) {
				if (pmatch[0].rm_eo != pmatch[0].rm_so + 13
				 || pmatch[0].rm_eo > len
				 || pmatch[0].rm_so < len - 100
				 || strncmp(string + pmatch[0].rm_so,
					" ChangeLog.13 for earlier changes",
					sizeof " ChangeLog.13 for earlier changes" - 1
				    ) != 0
				) {
					puts("regexec without REG_NOSUB did not find the correct match");
					exitcode = 1;
					goto contin1;
				}

				if (i > 0) {
					int j, k, l;
					for (j = 0, l = 1; j < 7; ++j) {
						for (k = 0; k < (i == 1 ? 1 : 10); ++k, ++l) {
							if (pmatch[l].rm_so != pmatch[0].rm_so + j
							|| pmatch[l].rm_eo != pmatch[l].rm_so + 1
							) {
								printf("pmatch[%d] incorrect\n", l);
								exitcode = 1;
								goto contin1;
							}
						}
					}
				}
			}

			gettimeofday(&stop, NULL);
			stop.tv_sec -= start.tv_sec;
			if (stop.tv_usec < start.tv_usec) {
				stop.tv_sec--;
				stop.tv_usec += 1000000;
			}
			stop.tv_usec -= start.tv_usec;
			printf(" %lu.%06lus\n", (unsigned long) stop.tv_sec,
						(unsigned long) stop.tv_usec);
 contin1:
			regfree(&rbuf);
		}
	}

	for (testno = 2; testno < 4; ++testno) {
		int i;
		for (i = 0; i < sizeof(pat) / sizeof(pat[0]); ++i) {
			struct timeval start, stop;
			struct re_pattern_buffer rpbuf;
			struct re_registers regs;
			const char *s;
			int match;

			printf("test %d pattern %d '%s'\n", testno, i, pat[i]);
			gettimeofday(&start, NULL);

			re_set_syntax(RE_SYNTAX_POSIX_EGREP
				| (testno == 3 ? RE_NO_SUB : 0));
			memset(&rpbuf, 0, sizeof(rpbuf));
			s = re_compile_pattern(pat[i], strlen(pat[i]), &rpbuf);
			if (s != NULL) {
				printf("%s\n", s);
				exitcode = 1;
				goto contin2;
			}

			memset(&regs, 0, sizeof(regs));
			match = re_search(&rpbuf, string, len, 0, len, &regs);
			if (match < 0) {
				printf("re_search failed (err:%d)\n", match);
				exitcode = 1;
				goto contin2;
			}
			if (match + 13 > len) {
				printf("re_search: match+13 > len (%d > %d)\n", match + 13, len);
				exitcode = 1;
				goto contin2;
			}
			if (match < len - 100) {
				printf("re_search: match < len-100 (%d < %d)\n", match, len - 100);
				exitcode = 1;
				goto contin2;
			}
			if (strncmp(string + match, " ChangeLog.13 for earlier changes",
				sizeof(" ChangeLog.13 for earlier changes") - 1
			    ) != 0
			) {
				printf("re_search did not find the correct match"
					"(found '%s' instead)\n", string + match);
				exitcode = 1;
				goto contin2;
			}

			if (testno == 2) {
				int expected = 72;
				if (i == 0)
					expected = 2;
				if (i == 1)
					expected = 9;
				if (regs.num_regs != expected) {
					printf("incorrect num_regs %d, expected %d\n", regs.num_regs, expected);
					exitcode = 1;
					goto contin2;
				}
				if (regs.start[0] != match || regs.end[0] != match + 13) {
					printf("incorrect regs.{start,end}[0] = { %d, %d },"
						" expected { %d, %d }\n",
						regs.start[0], regs.end[0],
						match, match + 13
					);
					exitcode = 1;
					goto contin2;
				}
				if (regs.start[regs.num_regs - 1] != -1
				 || regs.end[regs.num_regs - 1] != -1
				) {
					printf("incorrect regs.{start,end}[num_regs - 1] = { %d, %d },"
						" expected { -1, -1 }\n",
						regs.start[regs.num_regs - 1], regs.end[regs.num_regs - 1]
					);
					exitcode = 1;
					goto contin2;
				}

				if (i > 0) {
					int j, k, l;
					for (j = 0, l = 1; j < 7; ++j) {
						for (k = 0; k < (i == 1 ? 1 : 10); ++k, ++l) {
							if (regs.start[l] != match + j
							 || regs.end[l] != match + j + 1
							) {
								printf("incorrect regs.{start,end}[%d] = { %d, %d },"
									" expected { %d, %d }\n",
									l,
									regs.start[l], regs.end[l],
									match + j, match + j + 1
								);
								exitcode = 1;
								goto contin2;
							}
						}
					}
				}
			}

			gettimeofday(&stop, NULL);
			stop.tv_sec -= start.tv_sec;
			if (stop.tv_usec < start.tv_usec) {
				stop.tv_sec--;
				stop.tv_usec += 1000000;
			}
			stop.tv_usec -= start.tv_usec;
			printf(" %lu.%06lus\n", (unsigned long) stop.tv_sec,
						(unsigned long) stop.tv_usec);
 contin2:
			regfree(&rpbuf);
		}
	}
	return exitcode;
}

#define TIMEOUT 20
#define TEST_FUNCTION do_test()
#include "../test-skeleton.c"
