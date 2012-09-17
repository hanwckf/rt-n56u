/*
 * libc/stdlib/malloc/realloc.c -- realloc function
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

#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include "malloc.h"
#include "heap.h"


void *
realloc (void *mem, size_t new_size)
{
  size_t size;
  char *base_mem;

  /* Check for special cases.  */
  if (! new_size)
    {
      free (mem);
      return malloc (new_size);
    }
  if (! mem)
    return malloc (new_size);
  /* This matches the check in malloc() */
  if (unlikely(((unsigned long)new_size > (unsigned long)(MALLOC_HEADER_SIZE*-2))))
    return NULL;

  /* Normal realloc.  */

  base_mem = MALLOC_BASE (mem);
  size = MALLOC_SIZE (mem);

  /* Include extra space to record the size of the allocated block.
     Also make sure that we're dealing in a multiple of the heap
     allocation unit (SIZE is already guaranteed to be so).*/
  new_size = HEAP_ADJUST_SIZE (new_size + MALLOC_HEADER_SIZE);

  if (new_size < sizeof (struct heap_free_area))
    /* Because we sometimes must use a freed block to hold a free-area node,
       we must make sure that every allocated block can hold one.  */
    new_size = HEAP_ADJUST_SIZE (sizeof (struct heap_free_area));

  MALLOC_DEBUG (1, "realloc: 0x%lx, %d (base = 0x%lx, total_size = %d)",
		(long)mem, new_size, (long)base_mem, size);

  if (new_size > size)
    /* Grow the block.  */
    {
      size_t extra = new_size - size;

      __heap_lock (&__malloc_heap_lock);
      extra = __heap_alloc_at (&__malloc_heap, base_mem + size, extra);
      __heap_unlock (&__malloc_heap_lock);

      if (extra)
	/* Record the changed size.  */
	MALLOC_SET_SIZE (base_mem, size + extra);
      else
	/* Our attempts to extend MEM in place failed, just
	   allocate-and-copy.  */
	{
	  void *new_mem = malloc (new_size - MALLOC_HEADER_SIZE);
	  if (new_mem)
	    {
	      memcpy (new_mem, mem, size - MALLOC_HEADER_SIZE);
	      free (mem);
	    }
	  mem = new_mem;
	}
    }
  else if (new_size + MALLOC_REALLOC_MIN_FREE_SIZE <= size)
    /* Shrink the block.  */
    {
      __heap_lock (&__malloc_heap_lock);
      __heap_free (&__malloc_heap, base_mem + new_size, size - new_size);
      __heap_unlock (&__malloc_heap_lock);
      MALLOC_SET_SIZE (base_mem, new_size);
    }

  if (mem)
    MALLOC_DEBUG (-1, "realloc: returning 0x%lx (base:0x%lx, total_size:%d)",
		  (long)mem, (long)MALLOC_BASE(mem), (long)MALLOC_SIZE(mem));
  else
    MALLOC_DEBUG (-1, "realloc: returning 0");

  return mem;
}
