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
	int fd, n, cur_size;
	char buf[8], flash_buf[FLSH_SIZE];
	struct stat f_stat;
	int i;
	
	if((fd = open(BOOT_FILE, O_RDONLY)) < 0)
	{
		perror("open boot");
		return -1;
	}

	if(lstat(BOOT_FILE, &f_stat) < 0)
	{
		close(fd);
		perror("stat boot");
		return -1;
	}

	if(lseek(fd, -8, SEEK_END) < 0)
	{
		close(fd);
		perror("seek boot");
		return -1;
	}

	if((n = read(fd, buf, sizeof(buf))) < 0)
	{
		close(fd);
		perror("read boot");
		return -1;
	}
	
	for(i=0; i<n; ++i)
	{
		printf("[%x] ", (unsigned char)buf[i]);
	}

	printf("read magic done\n");
	close(fd);

	
	return 0;
}
