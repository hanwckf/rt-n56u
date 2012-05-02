
/* The mmap test is useful, since syscalls with 6 arguments
 * (as mmap) are done differently on various architectures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char **argv)
{
	void *ptr;

	ptr = mmap64(NULL, 4096, PROT_READ|PROT_WRITE,
		MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);

	if (ptr == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	printf("mmap returned %p\n", ptr);
	exit(0);
}
