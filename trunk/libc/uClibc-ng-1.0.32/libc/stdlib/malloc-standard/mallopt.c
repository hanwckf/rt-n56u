/*
  This is a version (aka dlmalloc) of malloc/free/realloc written by
  Doug Lea and released to the public domain.  Use, modify, and
  redistribute this code without permission or acknowledgement in any
  way you wish.  Send questions, comments, complaints, performance
  data, etc to dl@cs.oswego.edu

  VERSION 2.7.2 Sat Aug 17 09:07:30 2002  Doug Lea  (dl at gee)

  Note: There may be an updated version of this malloc obtainable at
           ftp://gee.cs.oswego.edu/pub/misc/malloc.c
  Check before installing!

  Hacked up for uClibc by Erik Andersen <andersen@codepoet.org>
*/

#include "malloc.h"


/* ------------------------------ mallopt ------------------------------ */
int mallopt(int param_number, int value)
{
    int ret;
    mstate av;

    ret = 0;

    __MALLOC_LOCK;
    av = get_malloc_state();
    /* Ensure initialization/consolidation */
    __malloc_consolidate(av);

    switch(param_number) {
	case M_MXFAST:
	    if (value >= 0 && value <= MAX_FAST_SIZE) {
		set_max_fast(av, value);
		ret = 1;
	    }
	    break;

	case M_TRIM_THRESHOLD:
	    av->trim_threshold = value;
	    ret = 1;
	    break;

	case M_TOP_PAD:
	    av->top_pad = value;
	    ret = 1;
	    break;

	case M_MMAP_THRESHOLD:
	    av->mmap_threshold = value;
	    ret = 1;
	    break;

	case M_MMAP_MAX:
	    av->n_mmaps_max = value;
	    ret = 1;
	    break;
    }
    __MALLOC_UNLOCK;
    return ret;
}

