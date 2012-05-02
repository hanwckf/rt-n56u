
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define N_PTRS 1000
#define N_ALLOCS 10000
#define MAX_SIZE 0x10000

#define random_size()	(random()%MAX_SIZE)
#define random_ptr()	(random()%N_PTRS)

int test1(void);
int test2(void);

int main(int argc, char *argv[])
{
	return test1() + test2();
}

int test1(void)
{
	void **ptrs;
	int i,j;
	int size;
	int ret = 0;

	srandom(0x19730929);

	ptrs = malloc(N_PTRS*sizeof(void *));

	for(i=0; i<N_PTRS; i++){
		if ((ptrs[i] = malloc(random_size())) == NULL) {
			printf("malloc random failed! %i\n", i);
			++ret;
		}
	}
	for(i=0; i<N_ALLOCS; i++){
		j = random_ptr();
		free(ptrs[j]);

		size = random_size();
		ptrs[j] = malloc(size);
		if (!ptrs[j]) {
			printf("malloc failed! %d\n", i);
			++ret;
		}
		memset(ptrs[j],0,size);
	}
	for(i=0; i<N_PTRS; i++){
		free(ptrs[i]);
	}

	return ret;
}

int test2(void)
{
	void *ptr = NULL;
	int ret = 0;

	ptr = realloc(ptr,100);
	if (!ptr) {
		printf("couldn't realloc() a NULL pointer\n");
		++ret;
	} else {
		free(ptr);
	}
	
	ptr = malloc(100);
	ptr = realloc(ptr, 0);
	if (ptr) {
		printf("realloc(,0) failed\n");
		++ret;
		free(ptr);
	}

	return ret;
}

