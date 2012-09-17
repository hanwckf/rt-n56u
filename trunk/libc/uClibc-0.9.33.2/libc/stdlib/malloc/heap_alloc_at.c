/*
 * libc/stdlib/malloc/heap_alloc_at.c -- allocate at a specific address
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


/* Allocate SIZE bytes at address MEM in HEAP.  Return the actual size
   allocated, or 0 if we failed.  */
size_t
__heap_alloc_at (struct heap_free_area **heap, void *mem, size_t size)
{
  struct heap_free_area *fa;
  size_t alloced = 0;

  size = HEAP_ADJUST_SIZE (size);

  HEAP_DEBUG (*heap, "before __heap_alloc_at");

  /* Look for a free area that can contain SIZE bytes.  */
  for (fa = *heap; fa; fa = fa->next)
    {
      void *fa_mem = HEAP_FREE_AREA_START (fa);
      if (fa_mem <= mem)
	{
	  if (fa_mem == mem && fa->size >= size)
	    /* FA has the right addr, and is big enough! */
	    alloced = __heap_free_area_alloc (heap, fa, size);
	  break;
	}
    }

  HEAP_DEBUG (*heap, "after __heap_alloc_at");

  return alloced;
}
