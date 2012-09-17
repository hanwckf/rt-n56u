/*
 * libc/stdlib/malloc/memalign.c -- memalign (`aligned malloc') function
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
#include <unistd.h>
#include <sys/mman.h>
#include <sys/param.h> /* MAX */

#include "malloc.h"
#include "heap.h"


/*
      ______________________ TOTAL _________________________
     /                                                      \
    +---------------+-------------------------+--------------+
    |               |                         |              |
    +---------------+-------------------------+--------------+
    \____ INIT ____/ \______ RETURNED _______/ \____ END ___/
*/

void *memalign (size_t alignment, size_t size);
void *
memalign (size_t alignment, size_t size)
{
  void *mem, *base;
  unsigned long tot_addr, tot_end_addr, addr, end_addr;
  struct heap_free_area **heap = &__malloc_heap;

  /* Make SIZE something we like.  */
  size = HEAP_ADJUST_SIZE (size);

  /* Use malloc to do the initial allocation, since it deals with getting
     system memory.  We over-allocate enough to be sure that we'll get
     enough memory to hold a properly aligned block of size SIZE,
     _somewhere_ in the result.  */
  mem = malloc (size + 2 * alignment);
  if (! mem)
    /* Allocation failed, we can't do anything.  */
    return 0;
  if (alignment < MALLOC_ALIGNMENT)
    return mem;

  /* Remember the base-address, of the allocation, although we normally
     use the user-address for calculations, since that's where the
     alignment matters.  */
  base = MALLOC_BASE (mem);

  /* The bounds of the initial allocation.  */
  tot_addr = (unsigned long)mem;
  tot_end_addr = (unsigned long)base + MALLOC_SIZE (mem);

  /* Find a likely place inside MEM with the right alignment.  */
  addr = MALLOC_ROUND_UP (tot_addr, alignment);

  /* Unless TOT_ADDR was already aligned correctly, we need to return the
     initial part of MEM to the heap.  */
  if (addr != tot_addr)
    {
      size_t init_size = addr - tot_addr;

      /* Ensure that memory returned to the heap is large enough.  */
      if (init_size < HEAP_MIN_SIZE)
	{
	  addr = MALLOC_ROUND_UP (tot_addr + HEAP_MIN_SIZE, alignment);
	  init_size = addr - tot_addr;
	}

      __heap_free (heap, base, init_size);

      /* Remember that we've freed the initial part of MEM.  */
      base += init_size;
    }

  /* Return the end part of MEM to the heap, unless it's too small.  */
  end_addr = addr + size;
  if (end_addr + MALLOC_REALLOC_MIN_FREE_SIZE < tot_end_addr)
    __heap_free (heap, (void *)end_addr, tot_end_addr - end_addr);
  else
    /* We didn't free the end, so include it in the size.  */
    end_addr = tot_end_addr;

  return MALLOC_SETUP (base, end_addr - (unsigned long)base);
}
