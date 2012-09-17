#include <stdio.h>

int main(void)
{
	unsigned long long val = -1;
	void *ptr = (void *)-1;
	printf("%p\n", ptr);

	sscanf("123456789", "%Lx", &val);
	printf("val = %Lx\n", val);
	return 0;
}
