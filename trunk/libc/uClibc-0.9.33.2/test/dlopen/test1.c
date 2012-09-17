#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#ifdef __UCLIBC__
extern void _dlinfo(void);
#endif

int main(int argc, char **argv) {
	void *handle;
	int (*mydltest)(const char *s);
	char *error;

	handle = dlopen ("./libtest1.so", RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "Could not open ./libtest1.so: %s\n", dlerror());
		exit(1);
	}

	mydltest = dlsym(handle, "dltest");
	if ((error = dlerror()) != NULL)  {
		fprintf(stderr, "Could not locate symbol 'dltest': %s\n", error);
		exit(1);
	}

	mydltest("hello world!");

	dlclose(handle);

	return EXIT_SUCCESS;
}

