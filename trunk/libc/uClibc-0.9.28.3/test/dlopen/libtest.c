#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

extern int __pthread_once(void);

void dltest(uint32_t **value1, uint32_t **value2)
{
	*value1 = (uint32_t *) __pthread_once;
	*value2 = (uint32_t *) pthread_self;
}

