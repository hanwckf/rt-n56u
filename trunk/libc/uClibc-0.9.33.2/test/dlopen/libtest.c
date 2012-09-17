#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

void dltest(uint32_t **value1, uint32_t **value2);
void dltest(uint32_t **value1, uint32_t **value2)
{
	*value1 = (uint32_t *) pthread_once;
	*value2 = (uint32_t *) pthread_self;
}

