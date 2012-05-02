/* alloc.c
 *
 * Written by Erik Andersen <andersee@codepoet.org>
 * LGPLv2
 *
 * Parts of the memalign code were stolen from malloc-930716.
 */

#define _GNU_SOURCE
#include <features.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>


#ifdef L_malloc
void *malloc(size_t size)
{
	void *result;

	if (unlikely(size == 0)) {
#if defined(__MALLOC_GLIBC_COMPAT__)
		size++;
#else
		/* Some programs will call malloc (0).  Lets be strict and return NULL */
		return 0;
#endif
	}

#ifdef __ARCH_HAS_MMU__
# define MMAP_FLAGS MAP_PRIVATE | MAP_ANONYMOUS
#else
# define MMAP_FLAGS MAP_SHARED | MAP_ANONYMOUS
#endif

	result = mmap((void *) 0, size + sizeof(size_t), PROT_READ | PROT_WRITE,
	              MMAP_FLAGS, 0, 0);
	if (result == MAP_FAILED)
		return 0;
	* (size_t *) result = size;
	return(result + sizeof(size_t));
}
#endif

#ifdef L_calloc
void * calloc(size_t nmemb, size_t lsize)
{
	void *result;
	size_t size=lsize * nmemb;

	/* guard vs integer overflow, but allow nmemb
	 * to fall through and call malloc(0) */
	if (nmemb && lsize != (size / nmemb)) {
		__set_errno(ENOMEM);
		return NULL;
	}
	result=malloc(size);
#if 0
	/* Standard unix mmap using /dev/zero clears memory so calloc
	 * doesn't need to actually zero anything....
	 */
	if (result != NULL) {
		memset(result, 0, size);
	}
#endif
	return result;
}
#endif

#ifdef L_realloc
void *realloc(void *ptr, size_t size)
{
	void *newptr = NULL;

	if (!ptr)
		return malloc(size);
	if (!size) {
		free(ptr);
		return malloc(0);
	}

	newptr = malloc(size);
	if (newptr) {
		memcpy(newptr, ptr, *((size_t *) (ptr - sizeof(size_t))));
		free(ptr);
	}
	return newptr;
}
#endif

#ifdef L_free
extern int weak_function __libc_free_aligned(void *ptr);
void free(void *ptr)
{
	if (unlikely(ptr == NULL))
		return;
	if (unlikely(__libc_free_aligned != NULL)) {
		if (__libc_free_aligned(ptr))
			return;
	}
	ptr -= sizeof(size_t);
	munmap(ptr, * (size_t *) ptr + sizeof(size_t));
}
#endif

#ifdef L_memalign

#include <bits/uClibc_mutex.h>

__UCLIBC_MUTEX_EXTERN(__malloc_lock);

#define __MALLOC_LOCK		__UCLIBC_MUTEX_LOCK(__malloc_lock)
#define __MALLOC_UNLOCK		__UCLIBC_MUTEX_UNLOCK(__malloc_lock)


/* List of blocks allocated with memalign or valloc */
struct alignlist
{
	struct alignlist *next;
	__ptr_t aligned;	/* The address that memaligned returned.  */
	__ptr_t exact;	/* The address that malloc returned.  */
};
struct alignlist *_aligned_blocks;

/* Return memory to the heap. */
int __libc_free_aligned(void *ptr)
{
	struct alignlist *l;

	if (ptr == NULL)
		return 0;

	__MALLOC_LOCK;
	for (l = _aligned_blocks; l != NULL; l = l->next) {
		if (l->aligned == ptr) {
			/* Mark the block as free */
			l->aligned = NULL;
			ptr = l->exact;
			ptr -= sizeof(size_t);
			munmap(ptr, * (size_t *) ptr + sizeof(size_t));
			return 1;
		}
	}
	__MALLOC_UNLOCK;
	return 0;
}
void * memalign (size_t alignment, size_t size)
{
	void * result;
	unsigned long int adj;

	result = malloc (size + alignment - 1);
	if (result == NULL)
		return NULL;

	adj = (unsigned long int) ((unsigned long int) ((char *) result -
	      (char *) NULL)) % alignment;
	if (adj != 0) {
		struct alignlist *l;
		__MALLOC_LOCK;
		for (l = _aligned_blocks; l != NULL; l = l->next)
			if (l->aligned == NULL)
				/* This slot is free.  Use it.  */
				break;
		if (l == NULL) {
			l = (struct alignlist *) malloc (sizeof (struct alignlist));
			if (l == NULL) {
				free(result);
				result = NULL;
				goto DONE;
			}
			l->next = _aligned_blocks;
			_aligned_blocks = l;
		}
		l->exact = result;
		result = l->aligned = (char *) result + alignment - adj;
	DONE:
		__MALLOC_UNLOCK;
	}

	return result;
}
#endif
