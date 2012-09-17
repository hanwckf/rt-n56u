/*
 * libc/stdlib/malloc/malloc.c -- malloc function
 *
 *  Copyright (C) 2002,03  NEC Electronics Corporation
 *  Copyright (C) 2002,03  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>


#include "malloc.h"
#include "heap.h"


/* The malloc heap.  We provide a bit of initial static space so that
   programs can do a little mallocing without mmaping in more space.  */
HEAP_DECLARE_STATIC_FREE_AREA (initial_fa, 256);
struct heap_free_area *__malloc_heap = HEAP_INIT_WITH_FA (initial_fa);
#ifdef HEAP_USE_LOCKING
malloc_mutex_t __malloc_heap_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

#if defined(MALLOC_USE_LOCKING) && defined(MALLOC_USE_SBRK)
/* A lock protecting our use of sbrk.  */
malloc_mutex_t __malloc_sbrk_lock;
#endif /* MALLOC_USE_LOCKING && MALLOC_USE_SBRK */


#ifdef __UCLIBC_UCLINUX_BROKEN_MUNMAP__
/* A list of all malloc_mmb structures describing blocks that
   malloc has mmapped, ordered by the block address.  */
struct malloc_mmb *__malloc_mmapped_blocks = 0;

/* A heap used for allocating malloc_mmb structures.  We could allocate
   them from the main heap, but that tends to cause heap fragmentation in
   annoying ways.  */
HEAP_DECLARE_STATIC_FREE_AREA (initial_mmb_fa, 48); /* enough for 3 mmbs */
struct heap_free_area *__malloc_mmb_heap = HEAP_INIT_WITH_FA (initial_mmb_fa);
#ifdef HEAP_USE_LOCKING
malloc_mutex_t __malloc_mmb_heap_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif
#endif /* __UCLIBC_UCLINUX_BROKEN_MUNMAP__ */


#ifdef HEAP_USE_LOCKING
#define malloc_from_heap(size, heap, lck) __malloc_from_heap(size, heap, lck)
#else
#define malloc_from_heap(size, heap, lck) __malloc_from_heap(size, heap)
#endif
static void *
__malloc_from_heap (size_t size, struct heap_free_area **heap
#ifdef HEAP_USE_LOCKING
		, malloc_mutex_t *heap_lock
#endif
		)
{
  void *mem;

  MALLOC_DEBUG (1, "malloc: %d bytes", size);

  /* Include extra space to record the size of the allocated block.  */
  size += MALLOC_HEADER_SIZE;

  __heap_lock (heap_lock);

  /* First try to get memory that's already in our heap.  */
  mem = __heap_alloc (heap, &size);

  __heap_unlock (heap_lock);

  if (unlikely (! mem))
    /* We couldn't allocate from the heap, so grab some more
       from the system, add it to the heap, and try again.  */
    {
      /* If we're trying to allocate a block bigger than the default
	 MALLOC_HEAP_EXTEND_SIZE, make sure we get enough to hold it. */
      void *block;
      size_t block_size
	= (size < MALLOC_HEAP_EXTEND_SIZE
	   ? MALLOC_HEAP_EXTEND_SIZE
	   : MALLOC_ROUND_UP_TO_PAGE_SIZE (size));

      /* Allocate the new heap block.  */
#ifdef MALLOC_USE_SBRK

      __malloc_lock_sbrk ();

      /* Use sbrk we can, as it's faster than mmap, and guarantees
	 contiguous allocation.  */
      block = sbrk (block_size);
      if (likely (block != (void *)-1))
	{
	  /* Because sbrk can return results of arbitrary
	     alignment, align the result to a MALLOC_ALIGNMENT boundary.  */
	  long aligned_block = MALLOC_ROUND_UP ((long)block, MALLOC_ALIGNMENT);
	  if (block != (void *)aligned_block)
	    /* Have to adjust.  We should only have to actually do this
	       the first time (after which we will have aligned the brk
	       correctly).  */
	    {
	      /* Move the brk to reflect the alignment; our next allocation
		 should start on exactly the right alignment.  */
	      sbrk (aligned_block - (long)block);
	      block = (void *)aligned_block;
	    }
	}

      __malloc_unlock_sbrk ();

#else /* !MALLOC_USE_SBRK */

      /* Otherwise, use mmap.  */
#ifdef __ARCH_USE_MMU__
      block = mmap ((void *)0, block_size, PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
#else
      block = mmap ((void *)0, block_size, PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS | MAP_UNINITIALIZE, 0, 0);
#endif

#endif /* MALLOC_USE_SBRK */

      if (likely (block != (void *)-1))
	{
#if !defined(MALLOC_USE_SBRK) && defined(__UCLIBC_UCLINUX_BROKEN_MUNMAP__)
	  struct malloc_mmb *mmb, *prev_mmb, *new_mmb;
#endif

	  MALLOC_DEBUG (1, "adding system memory to heap: 0x%lx - 0x%lx (%d bytes)",
			(long)block, (long)block + block_size, block_size);

	  /* Get back the heap lock.  */
	  __heap_lock (heap_lock);

	  /* Put BLOCK into the heap.  */
	  __heap_free (heap, block, block_size);

	  MALLOC_DEBUG_INDENT (-1);

	  /* Try again to allocate.  */
	  mem = __heap_alloc (heap, &size);


#if !defined(MALLOC_USE_SBRK) && defined(__UCLIBC_UCLINUX_BROKEN_MUNMAP__)
	  /* Insert a record of BLOCK in sorted order into the
	     __malloc_mmapped_blocks list.  */

	  new_mmb = malloc_from_heap (sizeof *new_mmb, &__malloc_mmb_heap, &__malloc_mmb_heap_lock);

	  for (prev_mmb = 0, mmb = __malloc_mmapped_blocks;
	       mmb;
	       prev_mmb = mmb, mmb = mmb->next)
	    if (block < mmb->mem)
	      break;

	  new_mmb->next = mmb;
	  new_mmb->mem = block;
	  new_mmb->size = block_size;

	  if (prev_mmb)
	    prev_mmb->next = new_mmb;
	  else
	    __malloc_mmapped_blocks = new_mmb;

	  MALLOC_MMB_DEBUG (0, "new mmb at 0x%x: 0x%x[%d]",
			    (unsigned)new_mmb,
			    (unsigned)new_mmb->mem, block_size);
#endif /* !MALLOC_USE_SBRK && __UCLIBC_UCLINUX_BROKEN_MUNMAP__ */
	  __heap_unlock (heap_lock);
	}
    }

  if (likely (mem))
    /* Record the size of the block and get the user address.  */
    {
      mem = MALLOC_SETUP (mem, size);

      MALLOC_DEBUG (-1, "malloc: returning 0x%lx (base:0x%lx, total_size:%ld)",
		    (long)mem, (long)MALLOC_BASE(mem), (long)MALLOC_SIZE(mem));
    }
  else
    MALLOC_DEBUG (-1, "malloc: returning 0");

  return mem;
}

void *
malloc (size_t size)
{
  void *mem;
#ifdef MALLOC_DEBUGGING
  static smallint debugging_initialized;
  if (! debugging_initialized)
    {
      debugging_initialized = 1;
      __malloc_debug_init ();
    }
  if (__malloc_check)
    __heap_check (__malloc_heap, "malloc");
#endif

#ifdef __MALLOC_GLIBC_COMPAT__
  if (unlikely (size == 0))
    size++;
#else
  /* Some programs will call malloc (0).  Lets be strict and return NULL */
  if (unlikely (size == 0))
    goto oom;
#endif

  /* Check if they are doing something dumb like malloc(-1) */
  if (unlikely(((unsigned long)size > (unsigned long)(MALLOC_HEADER_SIZE*-2))))
    goto oom;

  mem = malloc_from_heap (size, &__malloc_heap, &__malloc_heap_lock);
  if (unlikely (!mem))
    {
    oom:
      __set_errno (ENOMEM);
      return 0;
    }

  return mem;
}
