/*
  malloc_usable_size - fully inspired by musl implementation
*/

#include "malloc.h"

/* for malloc_usable_size  */
#define OVERHEAD (2*sizeof(size_t))
#define CHUNK_SIZE(c) ((c)->size & -2)
#define MEM_TO_CHUNK(p) (struct malloc_chunk *)((char *)(p) - OVERHEAD)

size_t malloc_usable_size(void *p) {
	return p ? CHUNK_SIZE(MEM_TO_CHUNK(p)) - OVERHEAD : 0;
}
