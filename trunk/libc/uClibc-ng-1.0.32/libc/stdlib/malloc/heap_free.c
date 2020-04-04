/*
 * libc/stdlib/malloc/heap_free.c -- return memory to a heap
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

#include "heap.h"


/* Return the block of memory at MEM, of size SIZE, to HEAP.  */
struct heap_free_area *
__heap_free (struct heap_free_area **heap, void *mem, size_t size)
{
  struct heap_free_area *fa, *prev_fa;
  void *end = (char *)mem + size;

  HEAP_DEBUG (*heap, "before __heap_free");

  /* Find the right position in the free-list entry to place the new block.
     This is the most speed critical loop in this malloc implementation:
     since we use a simple linked-list for the free-list, and we keep it in
     address-sorted order, it can become very expensive to insert something
     in the free-list when it becomes fragmented and long.  [A better
     implemention would use a balanced tree or something for the free-list,
     though that bloats the code-size and complexity quite a bit.]  */
  for (prev_fa = 0, fa = *heap; fa; prev_fa = fa, fa = fa->next)
    if (unlikely (HEAP_FREE_AREA_END (fa) >= mem))
      break;

  if (fa && HEAP_FREE_AREA_START (fa) <= end)
    /* The free-area FA is adjacent to the new block, merge them.  */
    {
      size_t fa_size = fa->size + size;

      if (HEAP_FREE_AREA_START (fa) == end)
	/* FA is just after the new block, grow down to encompass it. */
	{
	  /* See if FA can now be merged with its predecessor. */
	  if (prev_fa && mem == HEAP_FREE_AREA_END (prev_fa))
	    /* Yup; merge PREV_FA's info into FA.  */
	    {
	      fa_size += prev_fa->size;
	      __heap_link_free_area_after (heap, fa, prev_fa->prev);
	    }
	}
      else
	/* FA is just before the new block, expand to encompass it. */
	{
	  struct heap_free_area *next_fa = fa->next;

	  /* See if FA can now be merged with its successor. */
	  if (next_fa && end == HEAP_FREE_AREA_START (next_fa))
	    /* Yup; merge FA's info into NEXT_FA.  */
	    {
	      fa_size += next_fa->size;
	      __heap_link_free_area_after (heap, next_fa, prev_fa);
	      fa = next_fa;
	    }
	  else
	    /* FA can't be merged; move the descriptor for it to the tail-end
	       of the memory block.  */
	    {
	      /* The new descriptor is at the end of the extended block,
		 SIZE bytes later than the old descriptor.  */
	      fa = (struct heap_free_area *)((char *)fa + size);
	      /* Update links with the neighbors in the list.  */
	      __heap_link_free_area (heap, fa, prev_fa, next_fa);
	    }
	}

      fa->size = fa_size;
    }
  else
    /* Make the new block into a separate free-list entry.  */
    fa = __heap_add_free_area (heap, mem, size, prev_fa, fa);

  HEAP_DEBUG (*heap, "after __heap_free");

  return fa;
}
