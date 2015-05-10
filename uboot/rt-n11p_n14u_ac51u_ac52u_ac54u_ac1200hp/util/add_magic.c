#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#ifndef FLSH_SIZE
#define FLSH_SIZE 196599
#endif

#define BOOT_FILE "uboot.trx"

static char *cmdname;
static void usage(void)
{
	fprintf(stderr,
		"Usage: %s [-a] [ -d data_file]\n", cmdname);
	exit(-3);
}

int main(int argc, char* argv[])
{
	size_t cur_size;
	struct stat f_stat;
	int fd, pad_n = 0, append_magic = 0, opt;
	char *pad_buf = NULL;
	const char *datafile = BOOT_FILE;
	const char magic_buf[7] = { 'F', 'L', 'S', 'H', '\0', '\0', '\0' };
	int tmp, flash_size = FLSH_SIZE;

	cmdname = argv[0];
	while ((opt = getopt(argc, argv, "d:af:")) != -1) {
		switch (opt) {
		case 'd':
			datafile = optarg;
			break;
		case 'a':
			append_magic = 1;
			break;
		case 'f':
			tmp = atoi(optarg);
			if (tmp > 7)
				flash_size = tmp;
			break;
		default:
			usage();
		}
	}
	
	if((fd = open(datafile, O_RDWR)) < 0) {
		printf("Open %s fail. (errno %d %s)\n", datafile, errno, strerror(errno));
		return -1;
	}

	if(lstat(datafile, (void*)&f_stat) < 0) {
		close(fd);
		printf("stat %s fail. (errno %d %s)\n", datafile, errno, strerror(errno));
		return -1;
	}

	printf("flash size: %d\n", flash_size);
	cur_size = f_stat.st_size;
	if (!append_magic) {
		if((cur_size = f_stat.st_size) > (flash_size - 7)) {
			printf("current boot file is too large\n");
			return -1;
		} else
			printf("current boot file size is %d\n", cur_size);

		pad_n = (flash_size - 7) - cur_size;
		printf("padding %d bytes\n", pad_n);

		if (!(pad_buf = malloc(pad_n))) {
			printf("allocate %d bytes for padding fail.\n", pad_n);
			close(fd);
			return -2;
		}
		memset(pad_buf, 0, pad_n);
	} else {
		printf("append magic number\n");
	}

	lseek(fd, 0, SEEK_END);
	if (pad_n)
		write(fd, pad_buf, pad_n);
	write(fd, magic_buf, sizeof(magic_buf));
	close(fd);

	printf("add magic done\n");
	
	return 0;
}
