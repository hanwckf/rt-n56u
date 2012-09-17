#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>

#define LIBNAME "libundef.so"

int main(int argc, char **argv)
{
	void *handle;
	int (*myundefined)(void);

	handle = dlopen(LIBNAME, RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "Could not open ./%s: %s\n", LIBNAME, dlerror());
		return EXIT_FAILURE;
	}

	myundefined = dlsym(handle, "__booga_booga_you_cant_touch_this__");
	if (myundefined != NULL) {
		fprintf(stderr, "dlsym() found a symbol that does not exist!\n");
		return EXIT_FAILURE;
	}

	dlclose(handle);

	return EXIT_SUCCESS;
}
