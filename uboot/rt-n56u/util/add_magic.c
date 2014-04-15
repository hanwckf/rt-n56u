#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define FLSH_SIZE 196599
#define BOOT_FILE "uboot.bin"

int
main(int argc, char* argv[])
{
	int fd, pad_n, cur_size;
	char buf[8], flash_buf[FLSH_SIZE];
	struct stat f_stat;
	int i;
	
	if((fd = open(BOOT_FILE, O_RDWR)) < 0)
	{
		perror("open boot");
		return -1;
	}

	if(lstat(BOOT_FILE, (void*)&f_stat) < 0)
	{
		close(fd);
		perror("stat boot");
		return -1;
	}

	if((cur_size = f_stat.st_size) > (FLSH_SIZE - 7))
	{
		printf("current boot file is too large\n");
		return -1;
	} else
		printf("current boot file size is %d\n", cur_size);

	pad_n = (FLSH_SIZE - 7) - cur_size;
	printf("padding %d bytes\n", pad_n);

	lseek(fd, 0, SEEK_END);

	char buf1[pad_n], buf2[3], *magic_str="FLSH";

	write(fd, buf1, pad_n);
	//write(fd, buf1, 10);	// tmp test, it will fail
	write(fd, magic_str, 4);
	write(fd, buf2, 3);
	close(fd);

	printf("add magic done\n");

	
	return 0;
}
