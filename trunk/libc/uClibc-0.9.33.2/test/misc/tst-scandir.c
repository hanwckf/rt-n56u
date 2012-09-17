#include <dirent.h>
#include <errno.h>
#include <stdio.h> /* perror() */
#include <stdlib.h>

int skip_all(const struct dirent *dirbuf)
{
	errno = EBADF;
	return 0;
}

int main(void)
{
	struct dirent **namelist;
	int n;

	n = scandir(".", &namelist, skip_all, 0);
	if (n < 0) {
		perror("scandir");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
