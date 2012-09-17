/*
 * libc/stdlib/malloc/free.c -- free function
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
#include <sys/mman.h>

#include "malloc.h"
#include "heap.h"


static void
free_to_heap (void *mem, struct heap *heap)
{
  size_t size;
  struct heap_free_area *fa;

  /* Check for special cases.  */
  if (unlikely (! mem))
    return;

  /* Normal free.  */

  MALLOC_DEBUG (1, "free: 0x%lx (base = 0x%lx, total_size = %d)",
		(long)mem, (long)MALLOC_BASE (mem), MALLOC_SIZE (mem));

  size = MALLOC_SIZE (mem);
  mem = MALLOC_BASE (mem);

  __heap_lock (heap);

  /* Put MEM back in the heap, and get the free-area it was placed in.  */
  fa = __heap_free (heap, mem, size);

  /* See if the free-area FA has grown big enough that it should be
     unmapped.  */
  if (HEAP_FREE_AREA_SIZE (fa) < MALLOC_UNMAP_THRESHOLD)
    /* Nope, nothing left to do, just release the lock.  */
    __heap_unlock (heap);
  else
    /* Yup, try to unmap FA.  */
    {
      unsigned long start = (unsigned long)HEAP_FREE_AREA_START (fa);
      unsigned long end = (unsigned long)HEAP_FREE_AREA_END (fa);
#ifndef MALLOC_USE_SBRK
# ifdef __UCLIBC_UCLINUX_BROKEN_MUNMAP__
      struct malloc_mmb *mmb, *prev_mmb;
      unsigned long mmb_start, mmb_end;
# else /* !__UCLIBC_UCLINUX_BROKEN_MUNMAP__ */
      unsigned long unmap_start, unmap_end;
# endif /* __UCLIBC_UCLINUX_BROKEN_MUNMAP__ */
#endif /* !MALLOC_USE_SBRK */

#ifdef MALLOC_USE_SBRK
      /* Get the sbrk lock so that the two possible calls to sbrk below
	 are guaranteed to be contiguous.  */
      __malloc_lock_sbrk ();
      /* When using sbrk, we only shrink the heap from the end.  It would
	 be possible to allow _both_ -- shrinking via sbrk when possible,
	 and otherwise shrinking via munmap, but this results in holes in
	 memory that prevent the brk from every growing back down; since
	 we only ever grow the heap via sbrk, this tends to produce a
	 continuously growing brk (though the actual memory is unmapped),
	 which could eventually run out of address space.  Note that
	 `sbrk(0)' shouldn't normally do a system call, so this test is
	 reasonably cheap.  */
      if ((void *)end != sbrk (0))
	{
	  MALLOC_DEBUG (-1, "not unmapping: 0x%lx - 0x%lx (%ld bytes)",
			start, end, end - start);
	  __malloc_unlock_sbrk ();
	  __heap_unlock (heap);
	  return;
	}
#endif

      MALLOC_DEBUG (0, "unmapping: 0x%lx - 0x%lx (%ld bytes)",
		    start, end, end - start);

      /* Remove FA from the heap.  */
      __heap_delete (heap, fa);

      if (__heap_is_empty (heap))
	/* We want to avoid the heap from losing all memory, so reserve
	   a bit.  This test is only a heuristic -- the existance of
	   another free area, even if it's smaller than
	   MALLOC_MIN_SIZE, will cause us not to reserve anything.  */
	{
	  /* Put the reserved memory back in the heap; we asssume that
	     MALLOC_UNMAP_THRESHOLD is greater than MALLOC_MIN_SIZE, so
	     we use the latter unconditionally here.  */
	  __heap_free (heap, (void *)start, MALLOC_MIN_SIZE);
	  start += MALLOC_MIN_SIZE;
	}

#ifdef MALLOC_USE_SBRK

      /* Release the heap lock; we're still holding the sbrk lock.  */
      __heap_unlock (heap);
      /* Lower the brk.  */
      sbrk (start - end);
      /* Release the sbrk lock too; now we hold no locks.  */
      __malloc_unlock_sbrk ();

#else /* !MALLOC_USE_SBRK */

# ifdef __UCLIBC_UCLINUX_BROKEN_MUNMAP__
      /* Using the uClinux broken munmap, we have to only munmap blocks
	 exactly as we got them from mmap, so scan through our list of
	 mmapped blocks, and return them in order.  */

      MALLOC_MMB_DEBUG (1, "walking mmb list for region 0x%x[%d]...",
			start, end - start);

      prev_mmb = 0;
      mmb = __malloc_mmapped_blocks;
      while (mmb
	     && ((mmb_end = (mmb_start = (unsigned long)mmb->mem) + mmb->size)
		 <= end))
	{
	  MALLOC_MMB_DEBUG (1, "considering mmb at 0x%x: 0x%x[%d]",
			    (unsigned)mmb, mmb_start, mmb_end - mmb_start);

	  if (mmb_start >= start
	      /* If the space between START and MMB_START is non-zero, but
		 too small to return to the heap, we can't unmap MMB.  */
	      && (start == mmb_start
		  || mmb_start - start > HEAP_MIN_FREE_AREA_SIZE))
	    {
	      struct malloc_mmb *next_mmb = mmb->next;

	      if (mmb_end != end && mmb_end + HEAP_MIN_FREE_AREA_SIZE > end)
		/* There's too little space left at the end to deallocate
		   this block, so give up.  */
		break;

	      MALLOC_MMB_DEBUG (1, "unmapping mmb at 0x%x: 0x%x[%d]",
				(unsigned)mmb, mmb_start, mmb_end - mmb_start);

	      if (mmb_start != start)
		/* We're going to unmap a part of the heap that begins after
		   start, so put the intervening region back into the heap.  */
		{
		  MALLOC_MMB_DEBUG (0, "putting intervening region back into heap: 0x%x[%d]",
				    start, mmb_start - start);
		  __heap_free (heap, (void *)start, mmb_start - start);
		}

	      MALLOC_MMB_DEBUG_INDENT (-1);

	      /* Unlink MMB from the list.  */
	      if (prev_mmb)
		prev_mmb->next = next_mmb;
	      else
		__malloc_mmapped_blocks = next_mmb;

	      /* Start searching again from the end of this block.  */
	      start = mmb_end;

	      /* We have to unlock the heap before we recurse to free the mmb
		 descriptor, because we might be unmapping from the mmb
		 heap.  */
	      __heap_unlock (heap);

	      /* Release the descriptor block we used.  */
	      free_to_heap (mmb, &__malloc_mmb_heap);

	      /* Do the actual munmap.  */
	      munmap ((void *)mmb_start, mmb_end - mmb_start);

	      __heap_lock (heap);

#  ifdef __UCLIBC_HAS_THREADS__
	      /* In a multi-threaded program, it's possible that PREV_MMB has
		 been invalidated by another thread when we released the
		 heap lock to do the munmap system call, so just start over
		 from the beginning of the list.  It sucks, but oh well;
		 it's probably not worth the bother to do better.  */
	      prev_mmb = 0;
	      mmb = __malloc_mmapped_blocks;
#  else
	      mmb = next_mmb;
#  endif
	    }
	  else
	    {
	      prev_mmb = mmb;
	      mmb = mmb->next;
	    }

	  MALLOC_MMB_DEBUG_INDENT (-1);
	}

      if (start != end)
	/* Hmm, well there's something we couldn't unmap, so put it back
	   into the heap.  */
	{
	  MALLOC_MMB_DEBUG (0, "putting tail region back into heap: 0x%x[%d]",
			    start, end - start);
	  __heap_free (heap, (void *)start, end - start);
	}

      /* Finally release the lock for good.  */
      __heap_unlock (heap);

      MALLOC_MMB_DEBUG_INDENT (-1);

# else /* !__UCLIBC_UCLINUX_BROKEN_MUNMAP__ */

      /* MEM/LEN may not be page-aligned, so we have to page-align them,
	 and return any left-over bits on the end to the heap.  */
      unmap_start = MALLOC_ROUND_UP_TO_PAGE_SIZE (start);
      unmap_end = MALLOC_ROUND_DOWN_TO_PAGE_SIZE (end);

      /* We have to be careful that any left-over bits are large enough to
	 return.  Note that we _don't check_ to make sure there's room to
	 grow/shrink the start/end by another page, we just assume that
	 the unmap threshold is high enough so that this is always safe
	 (i.e., it should probably be at least 3 pages).  */
      if (unmap_start > start)
	{
	  if (unmap_start - start < HEAP_MIN_FREE_AREA_SIZE)
	    unmap_start += MALLOC_PAGE_SIZE;
	  __heap_free (heap, (void *)start, unmap_start - start);
	}
      if (end > unmap_end)
	{
	  if (end - unmap_end < HEAP_MIN_FREE_AREA_SIZE)
	    unmap_end -= MALLOC_PAGE_SIZE;
	  __heap_free (heap, (void *)unmap_end, end - unmap_end);
	}

      /* Release the heap lock before we do the system call.  */
      __heap_unlock (heap);

      if (unmap_end > unmap_start)
	/* Finally, actually unmap the memory.  */
	munmap ((void *)unmap_start, unmap_end - unmap_start);

# endif /* __UCLIBC_UCLINUX_BROKEN_MUNMAP__ */

#endif /* MALLOC_USE_SBRK */
    }

  MALLOC_DEBUG_INDENT (-1);
}

void
free (void *mem)
{
  free_to_heap (mem, &__malloc_heap);
}
