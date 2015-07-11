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

#include <features.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "malloc.h"


/* ------------------------------ memalign ------------------------------ */
void* memalign(size_t alignment, size_t bytes)
{
    size_t nb;             /* padded  request size */
    char*           m;              /* memory returned by malloc call */
    mchunkptr       p;              /* corresponding chunk */
    char*           _brk;            /* alignment point within p */
    mchunkptr       newp;           /* chunk to return */
    size_t newsize;        /* its size */
    size_t leadsize;       /* leading space before alignment point */
    mchunkptr       remainder;      /* spare room at end to split off */
    unsigned long    remainder_size; /* its size */
    size_t size;
    void *retval;

    /* If need less alignment than we give anyway, just relay to malloc */

    if (alignment <= MALLOC_ALIGNMENT) return malloc(bytes);

    /* Otherwise, ensure that it is at least a minimum chunk size */

    if (alignment <  MINSIZE) alignment = MINSIZE;

    /* Make sure alignment is power of 2 (in case MINSIZE is not).  */
    if ((alignment & (alignment - 1)) != 0) {
	size_t a = MALLOC_ALIGNMENT * 2;
	while ((unsigned long)a < (unsigned long)alignment) a <<= 1;
	alignment = a;
    }

    checked_request2size(bytes, nb);
    __MALLOC_LOCK;

    /* Strategy: find a spot within that chunk that meets the alignment
     * request, and then possibly free the leading and trailing space.  */


    /* Call malloc with worst case padding to hit alignment. */

    m  = (char*)(malloc(nb + alignment + MINSIZE));

    if (m == 0) {
	retval = 0; /* propagate failure */
	goto DONE;
    }

    p = mem2chunk(m);

    if ((((unsigned long)(m)) % alignment) != 0) { /* misaligned */

	/*
	   Find an aligned spot inside chunk.  Since we need to give back
	   leading space in a chunk of at least MINSIZE, if the first
	   calculation places us at a spot with less than MINSIZE leader,
	   we can move to the next aligned spot -- we've allocated enough
	   total room so that this is always possible.
	   */

	_brk = (char*)mem2chunk((unsigned long)(((unsigned long)(m + alignment - 1)) &
		    -((signed long) alignment)));
	if ((unsigned long)(_brk - (char*)(p)) < MINSIZE)
	    _brk += alignment;

	newp = (mchunkptr)_brk;
	leadsize = _brk - (char*)(p);
	newsize = chunksize(p) - leadsize;

	/* For mmapped chunks, just adjust offset */
	if (chunk_is_mmapped(p)) {
	    newp->prev_size = p->prev_size + leadsize;
	    set_head(newp, newsize|IS_MMAPPED);
	    retval = chunk2mem(newp);
	    goto DONE;
	}

	/* Otherwise, give back leader, use the rest */
	set_head(newp, newsize | PREV_INUSE);
	set_inuse_bit_at_offset(newp, newsize);
	set_head_size(p, leadsize);
	free(chunk2mem(p));
	p = newp;

	assert (newsize >= nb &&
		(((unsigned long)(chunk2mem(p))) % alignment) == 0);
    }

    /* Also give back spare room at the end */
    if (!chunk_is_mmapped(p)) {
	size = chunksize(p);
	if ((unsigned long)(size) > (unsigned long)(nb + MINSIZE)) {
	    remainder_size = size - nb;
	    remainder = chunk_at_offset(p, nb);
	    set_head(remainder, remainder_size | PREV_INUSE);
	    set_head_size(p, nb);
	    free(chunk2mem(remainder));
	}
    }

    check_inuse_chunk(p);
    retval = chunk2mem(p);

 DONE:
    __MALLOC_UNLOCK;
	return retval;
}
libc_hidden_def(memalign)
