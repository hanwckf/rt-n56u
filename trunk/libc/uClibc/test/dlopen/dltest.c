#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>

#ifdef __UCLIBC__
extern void _dlinfo(void);
#endif

int main(int argc, char **argv) 
{
	int ret = EXIT_SUCCESS;
	void *handle;
	void (*mydltest)(void *value1, void *value2);
	char *error;
	uint32_t *value1, *value2;

	handle = dlopen (LIBNAME, RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "Could not open ./libtest.so: %s\n", dlerror());
		exit(1);
	}

	mydltest = dlsym(handle, "dltest");
	if ((error = dlerror()) != NULL)  {
		fprintf(stderr, "Could not locate symbol 'dltest': %s\n", error);
		exit(1);
	}

	mydltest(&value1, &value2);
	printf("dltest: __pthread_once=%p\n", value1);
	printf("dltest: pthread_self=%p\n", value2);
	if (value1 == value2) {
	    ret = EXIT_FAILURE;
	    printf("dltest: values should NOT be equal  Weak values resolved incorrectly!\n");
	} else {
	    printf("dltest: weak symbols resoved correctly.\n");
	}

	dlclose(handle);

	return ret;
}

