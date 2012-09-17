/*
 * libc/stdlib/malloc/heap_alloc.c -- allocate memory from a heap
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


/* Allocate and return a block at least *SIZE bytes long from HEAP.
   *SIZE is adjusted to reflect the actual amount allocated (which may be
   greater than requested).  */
void *
__heap_alloc (struct heap *heap, size_t *size)
{
  struct heap_free_area *fa;
  size_t _size = *size;
  void *mem = 0;

  _size = HEAP_ADJUST_SIZE (_size);
  
  if (_size < sizeof (struct heap_free_area))
    /* Because we sometimes must use a freed block to hold a free-area node,
       we must make sure that every allocated block can hold one.  */
    _size = HEAP_ADJUST_SIZE (sizeof (struct heap_free_area));

  HEAP_DEBUG (heap, "before __heap_alloc");

  /* Look for a free area that can contain _SIZE bytes.  */
  for (fa = heap->free_areas; fa; fa = fa->next)
    if (fa->size >= _size)
      {
	/* Found one!  */
	mem = HEAP_FREE_AREA_START (fa);
	*size = __heap_free_area_alloc (heap, fa, _size);
	break;
      }

  HEAP_DEBUG (heap, "after __heap_alloc");

  return mem;
}
