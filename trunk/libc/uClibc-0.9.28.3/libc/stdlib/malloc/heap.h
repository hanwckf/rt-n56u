/*
 * libc/stdlib/malloc/heap.h -- heap allocator used for malloc
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

#include <features.h>


/* On multi-threaded systems, the heap includes a lock.  */
#ifdef __UCLIBC_HAS_THREADS__
# include <pthread.h>
# define HEAP_USE_LOCKING
#endif


/* The heap allocates in multiples of, and aligned to, HEAP_GRANULARITY.
   HEAP_GRANULARITY must be a power of 2.  Malloc depends on this being the
   same as MALLOC_ALIGNMENT.  */
#define HEAP_GRANULARITY_TYPE	double
#define HEAP_GRANULARITY	(sizeof (HEAP_GRANULARITY_TYPE))


/* A heap is a collection of memory blocks, from which smaller blocks
   of memory can be allocated.  */
struct heap
{
  /* A list of memory in the heap available for allocation.  */
  struct heap_free_area *free_areas;

#ifdef HEAP_USE_LOCKING
  /* A lock that can be used by callers to control access to the heap.
     The heap code _does not_ use this lock, it's merely here for the
     convenience of users!  */
  pthread_mutex_t lock;
#endif
};

/* The HEAP_INIT macro can be used as a static initializer for a heap
   variable.  The HEAP_INIT_WITH_FA variant is used to initialize a heap
   with an initial static free-area; its argument FA should be declared
   using HEAP_DECLARE_STATIC_FREE_AREA.  */
#ifdef HEAP_USE_LOCKING
# define HEAP_INIT 		{ 0, PTHREAD_MUTEX_INITIALIZER }
# define HEAP_INIT_WITH_FA(fa)	{ &fa._fa, PTHREAD_MUTEX_INITIALIZER }
#else
# define HEAP_INIT 		{ 0 }
# define HEAP_INIT_WITH_FA(fa) 	{ &fa._fa }
#endif

/* A free-list area `header'.  These are actually stored at the _ends_ of
   free areas (to make allocating from the beginning of the area simpler),
   so one might call it a `footer'.  */
struct heap_free_area
{
	size_t size;
	struct heap_free_area *next, *prev;
};

/* Return the address of the end of the frea area FA.  */
#define HEAP_FREE_AREA_END(fa) ((void *)(fa + 1))
/* Return the address of the beginning of the frea area FA.  FA is
   evaulated multiple times.  */
#define HEAP_FREE_AREA_START(fa) ((void *)((char *)(fa + 1) - (fa)->size))
/* Return the size of the frea area FA.  */
#define HEAP_FREE_AREA_SIZE(fa) ((fa)->size)

/* This rather clumsy macro allows one to declare a static free-area for
   passing to HEAP_INIT_WITH_FA initializer macro.  This is only use for
   which NAME is allowed.  */
#define HEAP_DECLARE_STATIC_FREE_AREA(name, size)			      \
  static struct								      \
  {									      \
    HEAP_GRANULARITY_TYPE aligned_space;				      \
    char space[HEAP_ADJUST_SIZE(size)					      \
	       - sizeof (struct heap_free_area)				      \
	       - HEAP_GRANULARITY];					      \
    struct heap_free_area _fa;						      \
  } name = { (HEAP_GRANULARITY_TYPE)0, "", { HEAP_ADJUST_SIZE(size), 0, 0 } }


/* Rounds SZ up to be a multiple of HEAP_GRANULARITY.  */
#define HEAP_ADJUST_SIZE(sz)  \
   (((sz) + HEAP_GRANULARITY - 1) & ~(HEAP_GRANULARITY - 1))


/* The minimum allocatable size.  */
#define HEAP_MIN_SIZE	HEAP_ADJUST_SIZE (sizeof (struct heap_free_area))

/* The minimum size of a free area; if allocating memory from a free-area
   would make the free-area smaller than this, the allocation is simply
   given the whole free-area instead.  It must include at least enough room
   to hold a struct heap_free_area, plus some extra to avoid excessive heap
   fragmentation (thus increasing speed).  This is only a heuristic -- it's
   possible for smaller free-areas than this to exist (say, by realloc
   returning the tail-end of a previous allocation), but __heap_alloc will
   try to get rid of them when possible.  */
#define HEAP_MIN_FREE_AREA_SIZE  \
  HEAP_ADJUST_SIZE (sizeof (struct heap_free_area) + 32)


/* branch-prediction macros; they may already be defined by libc.  */
#ifndef likely
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#define likely(cond)	__builtin_expect(!!(int)(cond), 1)
#define unlikely(cond)	__builtin_expect((int)(cond), 0)
#else
#define likely(cond)	(cond)
#define unlikely(cond)	(cond)
#endif
#endif /* !likely */


/* Define HEAP_DEBUGGING to cause the heap routines to emit debugging info
   to stderr when the variable __heap_debug is set to true.  */
#ifdef HEAP_DEBUGGING
extern int __heap_debug;
#define HEAP_DEBUG(heap, str) (__heap_debug ? __heap_dump (heap, str) : 0)
#else
#define HEAP_DEBUG(heap, str) (void)0
#endif

/* Output a text representation of HEAP to stderr, labelling it with STR.  */
extern void __heap_dump (struct heap *heap, const char *str);

/* Do some consistency checks on HEAP.  If they fail, output an error
   message to stderr, and exit.  STR is printed with the failure message.  */
extern void __heap_check (struct heap *heap, const char *str);


#ifdef HEAP_USE_LOCKING
# define __heap_lock(heap)	__pthread_mutex_lock (&(heap)->lock)
# define __heap_unlock(heap)	__pthread_mutex_unlock (&(heap)->lock)
#else /* !__UCLIBC_HAS_THREADS__ */
/* Without threads, mutex operations are a nop.  */
# define __heap_lock(heap)	(void)0
# define __heap_unlock(heap)	(void)0
#endif /* HEAP_USE_LOCKING */


/* Delete the free-area FA from HEAP.  */
static inline void
__heap_delete (struct heap *heap, struct heap_free_area *fa)
{
  if (fa->next)
    fa->next->prev = fa->prev;
  if (fa->prev)
    fa->prev->next = fa->next;
  else
    heap->free_areas = fa->next;
}


/* Link the free-area FA between the existing free-area's PREV and NEXT in
   HEAP.  PREV and NEXT may be 0; if PREV is 0, FA is installed as the
   first free-area.  */
static inline void
__heap_link_free_area (struct heap *heap, struct heap_free_area *fa,
		       struct heap_free_area *prev,
		       struct heap_free_area *next)
{
  fa->next = next;
  fa->prev = prev;

  if (prev)
    prev->next = fa;
  else
    heap->free_areas = fa;
  if (next)
    next->prev = fa;
}

/* Update the mutual links between the free-areas PREV and FA in HEAP.
   PREV may be 0, in which case FA is installed as the first free-area (but
   FA may not be 0).  */
static inline void
__heap_link_free_area_after (struct heap *heap,
			     struct heap_free_area *fa,
			     struct heap_free_area *prev)
{
  if (prev)
    prev->next = fa;
  else
    heap->free_areas = fa;
  fa->prev = prev;
}

/* Add a new free-area MEM, of length SIZE, in between the existing
   free-area's PREV and NEXT in HEAP, and return a pointer to its header.
   PREV and NEXT may be 0; if PREV is 0, MEM is installed as the first
   free-area.  */
static inline struct heap_free_area *
__heap_add_free_area (struct heap *heap, void *mem, size_t size,
		      struct heap_free_area *prev,
		      struct heap_free_area *next)
{
  struct heap_free_area *fa = (struct heap_free_area *)
    ((char *)mem + size - sizeof (struct heap_free_area));

  fa->size = size;

  __heap_link_free_area (heap, fa, prev, next);

  return fa;
}


/* Allocate SIZE bytes from the front of the free-area FA in HEAP, and
   return the amount actually allocated (which may be more than SIZE).  */
static inline size_t
__heap_free_area_alloc (struct heap *heap,
			struct heap_free_area *fa, size_t size)
{
  size_t fa_size = fa->size;

  if (fa_size < size + HEAP_MIN_FREE_AREA_SIZE)
    /* There's not enough room left over in FA after allocating the block, so
       just use the whole thing, removing it from the list of free areas.  */
    {
      __heap_delete (heap, fa);
      /* Remember that we've alloced the whole area.  */
      size = fa_size;
    }
  else
    /* Reduce size of FA to account for this allocation.  */
    fa->size = fa_size - size;

  return size;
}


/* Allocate and return a block at least *SIZE bytes long from HEAP.
   *SIZE is adjusted to reflect the actual amount allocated (which may be
   greater than requested).  */
extern void *__heap_alloc (struct heap *heap, size_t *size);

/* Allocate SIZE bytes at address MEM in HEAP.  Return the actual size
   allocated, or 0 if we failed.  */
extern size_t __heap_alloc_at (struct heap *heap, void *mem, size_t size);

/* Return the memory area MEM of size SIZE to HEAP.
   Returns the heap free area into which the memory was placed.  */
extern struct heap_free_area *__heap_free (struct heap *heap,
					   void *mem, size_t size);

/* Return true if HEAP contains absolutely no memory.  */
#define __heap_is_empty(heap) (! (heap)->free_areas)
