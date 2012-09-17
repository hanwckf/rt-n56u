#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	void *ptr = NULL;
	ptr = realloc(ptr, 0);
	printf("realloc(NULL, 0)  --  pointer = %p\n", ptr);

	ptr = malloc(0);
	printf("malloc(0)  --  pointer = %p\n", ptr);
	return 0;
}
