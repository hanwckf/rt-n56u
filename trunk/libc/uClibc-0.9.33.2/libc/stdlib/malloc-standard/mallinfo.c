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


/* ------------------------------ mallinfo ------------------------------ */
struct mallinfo mallinfo(void)
{
    mstate av;
    struct mallinfo mi;
    unsigned int i;
    mbinptr b;
    mchunkptr p;
    size_t avail;
    size_t fastavail;
    int nblocks;
    int nfastblocks;

    __MALLOC_LOCK;
    av = get_malloc_state();
    /* Ensure initialization */
    if (av->top == 0)  {
	__malloc_consolidate(av);
    }

    check_malloc_state();

    /* Account for top */
    avail = chunksize(av->top);
    nblocks = 1;  /* top always exists */

    /* traverse fastbins */
    nfastblocks = 0;
    fastavail = 0;

    for (i = 0; i < NFASTBINS; ++i) {
	for (p = av->fastbins[i]; p != 0; p = p->fd) {
	    ++nfastblocks;
	    fastavail += chunksize(p);
	}
    }

    avail += fastavail;

    /* traverse regular bins */
    for (i = 1; i < NBINS; ++i) {
	b = bin_at(av, i);
	for (p = last(b); p != b; p = p->bk) {
	    ++nblocks;
	    avail += chunksize(p);
	}
    }

    mi.smblks = nfastblocks;
    mi.ordblks = nblocks;
    mi.fordblks = avail;
    mi.uordblks = av->sbrked_mem - avail;
    mi.arena = av->sbrked_mem;
    mi.hblks = av->n_mmaps;
    mi.hblkhd = av->mmapped_mem;
    mi.fsmblks = fastavail;
    mi.keepcost = chunksize(av->top);
    mi.usmblks = av->max_total_mem;
    __MALLOC_UNLOCK;
    return mi;
}
libc_hidden_def(mallinfo)

void malloc_stats(FILE *file)
{
    struct mallinfo mi;

    if (file==NULL) {
	file = stderr;
    }

    mi = mallinfo();
    fprintf(file,
	    "total bytes allocated             = %10u\n"
	    "total bytes in use bytes          = %10u\n"
	    "total non-mmapped bytes allocated = %10d\n"
	    "number of mmapped regions         = %10d\n"
	    "total allocated mmap space        = %10d\n"
	    "total allocated sbrk space        = %10d\n"
#if 0
	    "number of free chunks             = %10d\n"
	    "number of fastbin blocks          = %10d\n"
	    "space in freed fastbin blocks     = %10d\n"
#endif
	    "maximum total allocated space     = %10d\n"
	    "total free space                  = %10d\n"
	    "memory releasable via malloc_trim = %10d\n",

	    (unsigned int)(mi.arena + mi.hblkhd),
	    (unsigned int)(mi.uordblks + mi.hblkhd),
	    mi.arena,
	    mi.hblks,
	    mi.hblkhd,
	    mi.uordblks,
#if 0
	    mi.ordblks,
	    mi.smblks,
	    mi.fsmblks,
#endif
	    mi.usmblks,
	    mi.fordblks,
	    mi.keepcost
	   );
}

