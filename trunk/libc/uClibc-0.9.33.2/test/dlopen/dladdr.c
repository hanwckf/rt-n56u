#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	Dl_info info;
	int res = 0;

	memset(&info, '\0', sizeof(Dl_info));
	res = dladdr((void *)1, &info);
	if (res != 0) {
		fprintf(stderr, "dladdr() should fail\n");
		fprintf(stderr, "dli_fname = %s\n", info.dli_fname);
		fprintf(stderr, "dli_fbase = 0x%08x\n", (unsigned int)info.dli_fbase);
		fprintf(stderr, "dli_sname = %s\n", info.dli_sname);
		fprintf(stderr, "dli_saddr = 0x%08x\n", (unsigned int)info.dli_saddr);
		exit(1);
        }

	fprintf(stderr, "dladdr() failed as expected\n");
	return EXIT_SUCCESS;
}

