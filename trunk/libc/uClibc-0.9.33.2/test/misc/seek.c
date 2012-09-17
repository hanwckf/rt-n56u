#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(*arr))

int main(void)
{
	struct {
		off_t offset;
		int   whence;
	} tests[] = {
		{ 0x00, SEEK_SET },
		{ 0x01, SEEK_SET },
		{ 0xFF, SEEK_SET }
	};
	char buf[2000];
	off_t ret;
	int i, fd;
	FILE *fp;
	int tmp;

	fd = open("lseek.out", O_RDWR|O_CREAT, 0600);
	if (fd == -1) {
		perror("open(lseek.out) failed");
		return 1;
	}
	unlink("lseek.out");
	fp = fdopen(fd, "rw");
	if (fp == NULL) {
		perror("fopen(lseek.out) failed");
		return 1;
	}

	memset(buf, 0xAB, sizeof(buf));
	ret = write(fd, buf, sizeof(buf));
	if (ret != sizeof(buf)) {
		fprintf(stderr, "write() failed to write %zi bytes (wrote %li): ", sizeof(buf), (long)ret);
		perror("");
		return 1;
	}

	tmp = fseeko(fp, 1024, SEEK_SET);
	assert(tmp == 0);
	tmp = fseeko(fp, (off_t)-16, SEEK_CUR);
	assert(tmp == 0);
	ret = ftell(fp);
	if (ret != (1024-16)) {
		fprintf(stderr, "ftell() failed, we wanted pos %i but got %li: ", (1024-16), (long)ret);
		perror("");
		return 1;
	}

	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		ret = lseek(fd, tests[i].offset, tests[i].whence);
		if (ret != tests[i].offset) {
			fprintf(stderr, "lseek(%li,%i) failed (wanted %li, got %li): ", (long)tests[i].offset,
			        tests[i].whence, (long)tests[i].offset, (long)ret);
			perror("");
			return 1;
		}
		ret = fseek(fp, tests[i].offset, tests[i].whence);
		if (ret != 0) {
			fprintf(stderr, "fseek(%li,%i) failed (wanted 0, got %li): ", (long)tests[i].offset,
			        tests[i].whence, (long)ret);
			perror("");
			return 1;
		}
	}

	fclose(fp);
	close(fd);

	printf("Success!\n");

	return 0;
}
