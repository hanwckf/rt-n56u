#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>

#define LIBNAME "libstatic.so"

int load_and_test(void)
{
	void *handle;
	int (*mystatic)(void);

	handle = dlopen(LIBNAME, RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "Could not open ./%s: %s\n", LIBNAME, dlerror());
		return 1;
	}

	mystatic = dlsym(handle, "static_test");
	if (mystatic == NULL) {
		fprintf(stderr, "Could not locate symbol 'static_test': %s\n", dlerror());
		return 1;
	}

	if (!mystatic()) {
		fprintf(stderr, "mystatic() failed: static vars were not setup properly\n");
		return 1;
	}

	dlclose(handle);

	return 0;
}

int main(int argc, char **argv)
{
	int count = 5;
	while (count-- > 0)
		if (load_and_test())
			return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
