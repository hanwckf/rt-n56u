
/* The mmap test is useful, since syscalls with 6 arguments
 * (as mmap) are done differently on various architectures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

#define SIZEOF_ARRAY(type) (sizeof(type)/sizeof(*type))

struct mmap_test {
	void *ret;
	int err;
	struct {
		void *start;
		size_t length;
		int prot;
		int flags;
		int fd;
		off_t offset;
	} args;
};

struct mmap_test tests[] = {
	[0] {
		.err = 0,
		.args.start = NULL,
		.args.length = 4096,
		.args.prot = PROT_READ|PROT_WRITE,
		.args.flags = MAP_PRIVATE|MAP_ANONYMOUS,
		.args.fd = 0,
		.args.offset = 0
	},
};

#define err(fmt, args...) \
	do { \
		fprintf(stderr, fmt "\n" , ## args); \
		exit(1); \
	} while (0)
#define errp(fmt, args...) err(fmt ": %s" , ## args , strerror(errno))

int main(int argc, char **argv)
{
	int i;
	struct mmap_test *t;

	for (i=0; i<SIZEOF_ARRAY(tests); ++i) {
		t = tests + i;

		errno = 0;
		t->ret = mmap(t->args.start, t->args.length, t->args.prot,
		              t->args.flags, t->args.fd, t->args.offset);

		if (t->err) {
			if (t->ret != MAP_FAILED)
				err("mmap test %i should have failed, but gave us %p", i, t->ret);
			else if (t->err != errno)
				errp("mmap test %i failed, but gave us wrong errno (got %i instead of %i)", i, errno, t->err);
		} else {
			if (t->ret == MAP_FAILED)
				errp("mmap test %i failed", i);
			else if (munmap(t->ret, t->args.length) != 0)
				errp("munmap test %i failed", i);
		}
	}

	exit(0);
}
