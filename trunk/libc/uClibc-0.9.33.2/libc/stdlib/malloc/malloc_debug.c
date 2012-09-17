/*
 * libc/stdlib/malloc/malloc_debug.c -- malloc debugging support
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
#include <unistd.h>
#include <stdarg.h>


#include "malloc.h"
#include "heap.h"

int __malloc_debug = 0, __malloc_check = 0;

#ifdef MALLOC_MMB_DEBUGGING
int __malloc_mmb_debug = 0;
#endif

/* Debugging output is indented this may levels.  */
int __malloc_debug_cur_indent = 0;


/* Print FMT and args indented at the current debug print level, followed
   by a newline, and change the level by INDENT.  */
void
__malloc_debug_printf (int indent, const char *fmt, ...)
{
  unsigned spaces = __malloc_debug_cur_indent * MALLOC_DEBUG_INDENT_SIZE;
  va_list val;

  while (spaces > 0)
    {
      putc (' ', stderr);
      spaces--;
    }

  va_start (val, fmt);
  vfprintf (stderr, fmt, val);
  va_end (val);

  putc ('\n', stderr);

  __malloc_debug_indent (indent);
}

void
__malloc_debug_init (void)
{
  char *ev = getenv ("MALLOC_DEBUG");
  if (ev)
    {
      int val = atoi (ev);

      if (val & 1)
	__malloc_check = 1;

      if (val & 2)
	__malloc_debug = 1;

#ifdef MALLOC_MMB_DEBUGGING
      if (val & 4)
	__malloc_mmb_debug = 1;
#endif

#ifdef HEAP_DEBUGGING
      if (val & 8)
	__heap_debug = 1;
#endif

      if (val)
	__malloc_debug_printf
	  (0, "malloc_debug: initialized to %d (check = %d, dump = %d, dump_mmb = %d, dump_heap = %d)",
	   val,
	   !!(val & 1), !!(val & 2),
	   !!(val & 4), !!(val & 8));
    }
}
