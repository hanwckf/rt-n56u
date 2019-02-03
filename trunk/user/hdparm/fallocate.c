/*
 * This function is in a separate file here
 * so that it can locally define _FILE_OFFSET_BITS=64
 * to automatically get 64-bit open/fstat/etc.. functions and types.
 * Eventually, all of hdparm should do that, but for now..
 */
#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/syscall.h>
#include <linux/types.h>
#include <linux/fs.h>

#include "hdparm.h"

int do_fallocate_syscall (const char *path, __u64 bytecount)
{
	int err;

#ifndef SYS_fallocate
	bytecount = 0;
	fprintf(stderr, "Error: this copy of hdparm was built without %s support\n", path);
	err = EINVAL;
#else
	int fd;
	unsigned long long offset = 0;	/* loff_t */
	unsigned long long len;
	int mode = 0;

	fd = open(path, O_WRONLY|O_CREAT|O_EXCL, 0600);
	if (fd == -1) {
		err = errno;
	} else {
		len = bytecount;
		err = syscall(SYS_fallocate, fd, mode, offset, len);
		if (err >= 0) {
			fsync(fd);
			exit(0);
		}
		err = errno;
		unlink(path);
	}
	perror(path);
#endif
	return err;
}
