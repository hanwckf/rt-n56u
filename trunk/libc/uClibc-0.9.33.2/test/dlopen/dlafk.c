#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define LIBNAME "libafk.so"

#define LIBAFK "libafk-temp.so"
#define LIBAFK_BAK ".libafk-temp.so.temp"

int main(int argc, char **argv)
{
	void *handle;

	if (rename(LIBAFK, LIBAFK_BAK)) {
		fprintf(stderr, "Unable to rename %s: %s\n", LIBAFK, strerror(errno));
		return EXIT_FAILURE;
	}

	handle = dlopen(LIBNAME, RTLD_NOW);
	if (!handle) {
		fprintf(stderr, "Could not open ./%s: %s\n", LIBNAME, dlerror());
		return EXIT_FAILURE;
	}

	if (rename(LIBAFK_BAK, LIBAFK)) {
		fprintf(stderr, "Unable to rename %s: %s\n", LIBAFK_BAK, strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
