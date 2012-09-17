#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

static int select_files(const struct dirent *dirbuf)
{
	if (dirbuf->d_name[0] == '.')
		return 0;
	else
		return 1;
}

int main(void)
{
	struct dirent **array;
	struct dirent *dirbuf;

	int i, numdir;

	chdir("/");
	numdir = scandir(".", &array, select_files, NULL);
	printf("\nGot %d entries from scandir().\n", numdir);
	for (i = 0; i < numdir; ++i) {
		dirbuf = array[i];
		printf("[%d] %s\n", i, dirbuf->d_name);
		free(array[i]);
	}
	free(array);
	numdir = scandir(".", &array, select_files, alphasort);
	printf("\nGot %d entries from scandir() using alphasort().\n", numdir);
	for (i = 0; i < numdir; ++i) {
		dirbuf = array[i];
		printf("[%d] %s\n", i, dirbuf->d_name);
	}
	printf("\nCalling qsort()\n");
	/* Even though some manpages say that alphasort should be
	 * int alphasort(const void *a, const void *b),
	 * in reality glibc and uclibc have const struct dirent**
	 * instead of const void*.
	 * Therefore we get a warning here unless we use a cast,
	 * which makes people think that alphasort prototype
	 * needs to be fixed in uclibc headers.
	 */
	qsort(array, numdir, sizeof(struct dirent *), (void*) alphasort);
	for (i = 0; i < numdir; ++i) {
		dirbuf = array[i];
		printf("[%d] %s\n", i, dirbuf->d_name);
		free(array[i]);
	}
	free(array);
	return (0);
}
