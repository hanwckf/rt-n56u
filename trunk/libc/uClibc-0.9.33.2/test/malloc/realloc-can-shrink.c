/* make sure that realloc() can properly shrink buffers */

#include <stdlib.h>

#define LARGE_BUFFER (1 << 20) /* idea is to span a lot of pages */

int main(int argc, char *argv[])
{
	int count = 20;
	char *ptr = NULL;
	while (count--) {
		ptr = realloc(ptr, LARGE_BUFFER);
		ptr = realloc(ptr, 1);
	}
	free(ptr);
	return 0;
}
