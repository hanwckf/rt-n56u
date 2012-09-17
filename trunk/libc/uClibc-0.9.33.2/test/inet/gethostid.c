#include <unistd.h>
#include <stdio.h>
int main(void) {
	printf("hostid=%d\n", gethostid());
	return 0;
}
