#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct list {
	struct list *next;
};

int main(void)
{
	int z=999;
	int *y=&z;
	int *x=NULL;
	struct list *save;
	struct list *lp;
	int i;


	printf("pointer to x is %p\n", x);
	printf("pointer to y is %p\n", y);
	x=malloc(sizeof(int)*2000);
	printf("pointer to x is %p\n", x);
	y=malloc(sizeof(int)*100);
	printf("pointer to y is %p\n", y);
	free(x);
	free(y);
	printf("about to free(0)\n");
	free(0);
	
	x=malloc(13);
	printf("x = %p\n", x);
	memcpy(x, "Small string", 13);
	printf("0x%p test string1: %s\n", x, (char *)x);
	y = realloc(x, 36);
	printf("0x%p test string1: %s\n", y, (char *)y);
	memcpy(y, "********** Larger string **********", 36);
	printf("0x%p test string2: %s\n", y, (char *)y);
	free(y);
	
	
	printf("Allocate 100 nodes 500 bytes each\n");
	save = 0;
	for (i=0; i<100; i++) {
		lp = malloc(500);
		if (lp == 0) {
			printf("loop 1: malloc returned 0\n");
			goto Failed;
		}
		lp->next = save;
		save = lp;
	}
	
	printf("freeing 100 nodes\n");
	while (save) {
		lp = save;
		save = save->next;
		free(lp);
	}
	
	printf("try realloc 100 times \n");
	lp = 0;
	for (i=1; i<=100; i++) {
		lp = realloc(lp, i*200);
		if (lp == 0) {
			printf("loop 3: realloc returned 0\n");
			goto Failed;
		}
	}
	realloc(lp, 0);
	
	printf("Allocate another 100 nodes 600 bytes each\n");
	save = 0;
	for (i=0; i<100; i++) {
		lp = malloc(600);
		if (lp == 0) {
			printf("loop 2: malloc returned 0\n");
			goto Failed;
		}
		lp->next = save;
		save = lp;
	}
	
	printf("freeing 100 nodes\n");
	while (save) {
		lp = save;
		save = save->next;
		free(lp);
	}
	

	printf("alloc test PASSED\n");
	exit(0);

Failed:
	printf("!!!!!!!!!!!! alloc test FAILED. !!!!!!!!!!!!!!!\n");
	exit(1);
}
