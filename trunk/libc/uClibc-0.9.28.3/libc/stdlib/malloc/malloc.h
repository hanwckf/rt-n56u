/*
 * libc/stdlib/malloc/malloc.h -- small malloc implementation
 *
 *  Copyright (C) 2002  NEC Corporation
 *  Copyright (C) 2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

/* The alignment we guarantee for malloc return values.  */
#define MALLOC_ALIGNMENT	(sizeof (double))

/* The system pagesize... */
extern size_t __pagesize;
#define MALLOC_PAGE_SIZE	__pagesize

/* The minimum size of block we request from the the system to extend the
   heap for small allocations (we may request a bigger block if necessary to
   satisfy a particularly big request).  */
#define MALLOC_HEAP_EXTEND_SIZE	MALLOC_PAGE_SIZE

/* When a heap free-area grows above this size, try to unmap it, releasing
   the memory back to the system.  */
#define MALLOC_UNMAP_THRESHOLD	(8*MALLOC_PAGE_SIZE)
/* When unmapping a free-area, retain this many bytes if it's the only one,
   to avoid completely emptying the heap.  This is only a heuristic -- the
   existance of another free area, even if it's smaller than
   MALLOC_MIN_SIZE, will cause us not to reserve anything.  */
#define MALLOC_MIN_SIZE		(2*MALLOC_PAGE_SIZE)

/* When realloc shrinks an allocation, it only does so if more than this
   many bytes will be freed; it must at at least HEAP_MIN_SIZE.  Larger
   values increase speed (by reducing heap fragmentation) at the expense of
   space.  */
#define MALLOC_REALLOC_MIN_FREE_SIZE  (HEAP_MIN_SIZE + 16)


/* For systems with an MMU, use sbrk to map/unmap memory for the malloc
   heap, instead of mmap/munmap.  This is a tradeoff -- sbrk is faster than
   mmap/munmap, and guarantees contiguous allocation, but is also less
   flexible, and causes the heap to only be shrinkable from the end.  */
#ifdef __ARCH_HAS_MMU__
# define MALLOC_USE_SBRK
#endif


/* The current implementation of munmap in uClinux doesn't work correctly:
   it requires that ever call to munmap exactly match a corresponding call
   to mmap (that is, it doesn't allow you to unmap only part of a
   previously allocated block, or to unmap two contiguous blocks with a
   single call to munmap).  This behavior is broken, and uClinux should be
   fixed; however, until it is, we add code to work around the problem in
   malloc.  */
#ifdef __UCLIBC_UCLINUX_BROKEN_MUNMAP__

/* A structure recording a block of memory mmapped by malloc.  */
struct malloc_mmb
{
  void *mem;			/* the mmapped block */
  size_t size;			/* its size */
  struct malloc_mmb *next;
};

/* A list of all malloc_mmb structures describing blocsk that malloc has
   mmapped, ordered by the block address.  */
extern struct malloc_mmb *__malloc_mmapped_blocks;

/* A heap used for allocating malloc_mmb structures.  We could allocate
   them from the main heap, but that tends to cause heap fragmentation in
   annoying ways.  */
extern struct heap __malloc_mmb_heap;

/* Define MALLOC_MMB_DEBUGGING to cause malloc to emit debugging info about
   about mmap block allocation/freeing by the `uclinux broken munmap' code
   to stderr, when the variable __malloc_mmb_debug is set to true. */
#ifdef MALLOC_MMB_DEBUGGING
# include <stdio.h>
extern int __malloc_mmb_debug;
# define MALLOC_MMB_DEBUG(indent, fmt, args...)				      \
   (__malloc_mmb_debug ? __malloc_debug_printf (indent, fmt , ##args) : 0)
# define MALLOC_MMB_DEBUG_INDENT(indent)				      \
   (__malloc_mmb_debug ? __malloc_debug_indent (indent) : 0)
# ifndef MALLOC_DEBUGGING
#  define MALLOC_DEBUGGING
# endif
#else /* !MALLOC_MMB_DEBUGGING */
# define MALLOC_MMB_DEBUG(fmt, args...) (void)0
# define MALLOC_MMB_DEBUG_INDENT(indent) (void)0
#endif /* MALLOC_MMB_DEBUGGING */

#endif /* __UCLIBC_UCLINUX_BROKEN_MUNMAP__ */


/* The size of a malloc allocation is stored in a size_t word
   MALLOC_ALIGNMENT bytes prior to the start address of the allocation:

     +--------+---------+-------------------+
     | SIZE   |(unused) | allocation  ...   |
     +--------+---------+-------------------+
     ^ BASE             ^ ADDR
     ^ ADDR - MALLOC_ALIGN
*/

/* The amount of extra space used by the malloc header.  */
#define MALLOC_HEADER_SIZE	MALLOC_ALIGNMENT

/* Set up the malloc header, and return the user address of a malloc block. */
#define MALLOC_SETUP(base, size)  \
  (MALLOC_SET_SIZE (base, size), (void *)((char *)base + MALLOC_HEADER_SIZE))
/* Set the size of a malloc allocation, given the base address.  */
#define MALLOC_SET_SIZE(base, size)	(*(size_t *)(base) = (size))

/* Return base-address of a malloc allocation, given the user address.  */
#define MALLOC_BASE(addr)	((void *)((char *)addr - MALLOC_HEADER_SIZE))
/* Return the size of a malloc allocation, given the user address.  */
#define MALLOC_SIZE(addr)	(*(size_t *)MALLOC_BASE(addr))


/* Locking for multithreaded apps.  */
#ifdef __UCLIBC_HAS_THREADS__

# include <pthread.h>

# define MALLOC_USE_LOCKING

typedef pthread_mutex_t malloc_mutex_t;
# define MALLOC_MUTEX_INIT	PTHREAD_MUTEX_INITIALIZER

# ifdef MALLOC_USE_SBRK
/* This lock is used to serialize uses of the `sbrk' function (in both
   malloc and free, sbrk may be used several times in succession, and
   things will break if these multiple calls are interleaved with another
   thread's use of sbrk!).  */
extern malloc_mutex_t __malloc_sbrk_lock;
#  define __malloc_lock_sbrk()	__pthread_mutex_lock (&__malloc_sbrk_lock)
#  define __malloc_unlock_sbrk() __pthread_mutex_unlock (&__malloc_sbrk_lock)
# endif /* MALLOC_USE_SBRK */

#else /* !__UCLIBC_HAS_THREADS__ */

/* Without threads, mutex operations are a nop.  */
# define __malloc_lock_sbrk()	(void)0
# define __malloc_unlock_sbrk()	(void)0

#endif /* __UCLIBC_HAS_THREADS__ */


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


/* Define MALLOC_DEBUGGING to cause malloc to emit debugging info to stderr
   when the variable __malloc_debug is set to true. */
#ifdef MALLOC_DEBUGGING

extern void __malloc_debug_init (void);

/* The number of spaces in a malloc debug indent level.  */
#define MALLOC_DEBUG_INDENT_SIZE 3

extern int __malloc_debug, __malloc_check;

# define MALLOC_DEBUG(indent, fmt, args...)				      \
   (__malloc_debug ? __malloc_debug_printf (indent, fmt , ##args) : 0)
# define MALLOC_DEBUG_INDENT(indent)					      \
   (__malloc_debug ? __malloc_debug_indent (indent) : 0)

extern int __malloc_debug_cur_indent;

/* Print FMT and args indented at the current debug print level, followed
   by a newline, and change the level by INDENT.  */
extern void __malloc_debug_printf (int indent, const char *fmt, ...);

/* Change the current debug print level by INDENT, and return the value.  */
#define __malloc_debug_indent(indent) (__malloc_debug_cur_indent += indent)

/* Set the current debug print level to LEVEL.  */
#define __malloc_debug_set_indent(level) (__malloc_debug_cur_indent = level)

#else /* !MALLOC_DEBUGGING */
# define MALLOC_DEBUG(fmt, args...) (void)0
# define MALLOC_DEBUG_INDENT(indent) (void)0
#endif /* MALLOC_DEBUGGING */


/* Return SZ rounded down to POWER_OF_2_SIZE (which must be power of 2).  */
#define MALLOC_ROUND_DOWN(sz, power_of_2_size)  \
  ((sz) & ~(power_of_2_size - 1))
/* Return SZ rounded to POWER_OF_2_SIZE (which must be power of 2).  */
#define MALLOC_ROUND_UP(sz, power_of_2_size)				\
  MALLOC_ROUND_DOWN ((sz) + (power_of_2_size - 1), (power_of_2_size))

/* Return SZ rounded down to a multiple MALLOC_PAGE_SIZE.  */
#define MALLOC_ROUND_DOWN_TO_PAGE_SIZE(sz)  \
  MALLOC_ROUND_DOWN (sz, MALLOC_PAGE_SIZE)
/* Return SZ rounded up to a multiple MALLOC_PAGE_SIZE.  */
#define MALLOC_ROUND_UP_TO_PAGE_SIZE(sz)  \
  MALLOC_ROUND_UP (sz, MALLOC_PAGE_SIZE)


/* The malloc heap.  */
extern struct heap __malloc_heap;
