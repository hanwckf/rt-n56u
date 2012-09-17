/* Distilled from issue found with tar and symlinks.
 * Make sure that the whole stat struct between runs
 * is agreeable.
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

static void show_stat(struct stat *st)
{
	printf(
		"------------------\n"
		"st_dev     = %li\n"
		"st_ino     = %li\n"
		"st_mode    = %li\n"
		"st_nlink   = %li\n"
		"st_uid     = %li\n"
		"st_gid     = %li\n"
		"st_rdev    = %li\n"
		"st_size    = %li\n"
		"st_blksize = %li\n"
		"st_blocks  = %li\n"
		"st_atime   = %li\n"
		"st_ansec   = %li\n"
		"st_mtime   = %li\n"
		"st_mnsec   = %li\n"
		"st_ctime   = %li\n"
		"st_cnsec   = %li\n",
		(long int)st->st_dev,
		(long int)st->st_ino,
		(long int)st->st_mode,
		(long int)st->st_nlink,
		(long int)st->st_uid,
		(long int)st->st_gid,
		(long int)st->st_rdev,
		(long int)st->st_size,
		(long int)st->st_blksize,
		(long int)st->st_blocks,
#if !defined(__UCLIBC__) || defined(__USE_MISC)
		(long int)st->st_atime,
		(long int)st->st_atim.tv_nsec,
		(long int)st->st_mtime,
		(long int)st->st_mtim.tv_nsec,
		(long int)st->st_ctime,
		(long int)st->st_ctim.tv_nsec
#else
		(long int)st->st_atime,
		(long int)st->st_atimensec,
		(long int)st->st_mtime,
		(long int)st->st_mtimensec,
		(long int)st->st_ctime,
		(long int)st->st_ctimensec
#endif
	);
}

int main(void)
{
	int ret;
	int fd;
	struct stat fst, st;

	memset(&fst, 0xAA, sizeof(fst));
	memset(&st, 0x55, sizeof(st));

	unlink(".testfile");
	fd = open(".testfile", O_WRONLY | O_CREAT | O_EXCL, 0);
	if (fd < 0) {
		perror("open(.testfile) failed");
		return 1;
	}
	ret = fstat(fd, &fst);
	if (ret != 0) {
		perror("fstat(.testfile) failed");
		return 1;
	}
	close(fd);

	ret = stat(".testfile", &st);
	if (ret != 0) {
		perror("stat(.testfile) failed");
		return 1;
	}

	ret = memcmp(&fst, &st, sizeof(fst));
	if (ret != 0) {
		printf("FAILED: memcmp() = %i\n", ret);
		show_stat(&fst);
		show_stat(&st);
	}

	unlink(".testfile");

	return ret;
}
