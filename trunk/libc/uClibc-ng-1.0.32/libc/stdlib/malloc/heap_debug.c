/*
 * libc/stdlib/malloc/heap_debug.c -- optional heap debugging routines
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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>


#include "malloc.h"
#include "heap.h"


#ifdef HEAP_DEBUGGING
int __heap_debug = 0;
#endif


static void
__heap_dump_freelist (struct heap_free_area *heap)
{
  struct heap_free_area *fa;
  for (fa = heap; fa; fa = fa->next)
    __malloc_debug_printf (0,
			   "0x%lx:  0x%lx - 0x%lx  (%d)\tP=0x%lx, N=0x%lx",
			   (long)fa,
			   (long)HEAP_FREE_AREA_START (fa),
			   (long)HEAP_FREE_AREA_END (fa),
			   fa->size,
			   (long)fa->prev,
			   (long)fa->next);
}

/* Output a text representation of HEAP to stderr, labelling it with STR.  */
void
__heap_dump (struct heap_free_area *heap, const char *str)
{
  static smallint recursed;

  if (! recursed)
    {
      __heap_check (heap, str);

      recursed = 1;

      __malloc_debug_printf (1, "%s: heap @0x%lx:", str, (long)heap);
      __heap_dump_freelist (heap);
      __malloc_debug_indent (-1);

      recursed = 0;
    }
}


/* Output an error message to stderr, and exit.  STR is printed with the
   failure message.  */
static void attribute_noreturn
__heap_check_failure (struct heap_free_area *heap, struct heap_free_area *fa,
		      const char *str, char *fmt, ...)
{
  va_list val;

  if (str)
    fprintf (stderr, "\nHEAP CHECK FAILURE %s: ", str);
  else
    fprintf (stderr, "\nHEAP CHECK FAILURE: ");

  va_start (val, fmt);
  vfprintf (stderr, fmt, val);
  va_end (val);

  fprintf (stderr, "\n");

  __malloc_debug_set_indent (0);
  __malloc_debug_printf (1, "heap dump:");
  __heap_dump_freelist (heap);

  _exit (22);
}

/* Do some consistency checks on HEAP.  If they fail, output an error
   message to stderr, and exit.  STR is printed with the failure message.  */
void
__heap_check (struct heap_free_area *heap, const char *str)
{
  typedef unsigned long ul_t;
  struct heap_free_area *fa, *prev;
  struct heap_free_area *first_fa = heap;

  if (first_fa && first_fa->prev)
    __heap_check_failure (heap, first_fa, str,
"first free-area has non-zero prev pointer:\n\
    first free-area = 0x%lx\n\
    (0x%lx)->prev   = 0x%lx\n",
			      (ul_t)first_fa,
			      (ul_t)first_fa, (ul_t)first_fa->prev);

  for (prev = 0, fa = first_fa; fa; prev = fa, fa = fa->next)
    {
      if (((ul_t)HEAP_FREE_AREA_END (fa) & (HEAP_GRANULARITY - 1))
	  || (fa->size & (HEAP_GRANULARITY - 1)))
	__heap_check_failure (heap, fa, str, "alignment error:\n\
    (0x%lx)->start = 0x%lx\n\
    (0x%lx)->size  = 0x%lx\n",
			      (ul_t)fa,
			      (ul_t)HEAP_FREE_AREA_START (fa),
			      (ul_t)fa, fa->size);

      if (fa->prev != prev)
	__heap_check_failure (heap, fa, str, "prev pointer corrupted:\n\
    (0x%lx)->next = 0x%lx\n\
    (0x%lx)->prev = 0x%lx\n",
			      (ul_t)prev, (ul_t)prev->next,
			      (ul_t)fa, (ul_t)fa->prev);

      if (prev)
	{
	  ul_t start = (ul_t)HEAP_FREE_AREA_START (fa);
	  ul_t prev_end = (ul_t)HEAP_FREE_AREA_END (prev);

	  if (prev_end >= start)
	    __heap_check_failure (heap, fa, str,
				  "start %s with prev free-area end:\n\
    (0x%lx)->prev  = 0x%lx\n\
    (0x%lx)->start = 0x%lx\n\
    (0x%lx)->end   = 0x%lx\n",
				  (prev_end == start ? "unmerged" : "overlaps"),
				  (ul_t)fa, (ul_t)prev,
				  (ul_t)fa, start,
				  (ul_t)prev, prev_end);
	}
    }
}
