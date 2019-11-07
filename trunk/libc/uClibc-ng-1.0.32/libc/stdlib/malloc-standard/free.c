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


/* ------------------------- __malloc_trim -------------------------
   __malloc_trim is an inverse of sorts to __malloc_alloc.  It gives memory
   back to the system (via negative arguments to sbrk) if there is unused
   memory at the `high' end of the malloc pool. It is called automatically by
   free() when top space exceeds the trim threshold. It is also called by the
   public malloc_trim routine.  It returns 1 if it actually released any
   memory, else 0.
*/
static int __malloc_trim(size_t pad, mstate av)
{
    long  top_size;        /* Amount of top-most memory */
    long  extra;           /* Amount to release */
    long  released;        /* Amount actually released */
    char* current_brk;     /* address returned by pre-check sbrk call */
    char* new_brk;         /* address returned by post-check sbrk call */
    size_t pagesz;

    pagesz = av->pagesize;
    top_size = chunksize(av->top);

    /* Release in pagesize units, keeping at least one page */
    extra = ((top_size - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

    if (extra > 0) {

	/*
	   Only proceed if end of memory is where we last set it.
	   This avoids problems if there were foreign sbrk calls.
	   */
	current_brk = (char*)(MORECORE(0));
	if (current_brk == (char*)(av->top) + top_size) {

	    /*
	       Attempt to release memory. We ignore MORECORE return value,
	       and instead call again to find out where new end of memory is.
	       This avoids problems if first call releases less than we asked,
	       of if failure somehow altered brk value. (We could still
	       encounter problems if it altered brk in some very bad way,
	       but the only thing we can do is adjust anyway, which will cause
	       some downstream failure.)
	       */

	    MORECORE(-extra);
	    new_brk = (char*)(MORECORE(0));

	    if (new_brk != (char*)MORECORE_FAILURE) {
		released = (long)(current_brk - new_brk);

		if (released != 0) {
		    /* Success. Adjust top. */
		    av->sbrked_mem -= released;
		    set_head(av->top, (top_size - released) | PREV_INUSE);
		    check_malloc_state();
		    return 1;
		}
	    }
	}
    }
    return 0;
}

/* ------------------------- malloc_trim -------------------------
  malloc_trim(size_t pad);

  If possible, gives memory back to the system (via negative
  arguments to sbrk) if there is unused memory at the `high' end of
  the malloc pool. You can call this after freeing large blocks of
  memory to potentially reduce the system-level memory requirements
  of a program. However, it cannot guarantee to reduce memory. Under
  some allocation patterns, some large free blocks of memory will be
  locked between two used chunks, so they cannot be given back to
  the system.

  The `pad' argument to malloc_trim represents the amount of free
  trailing space to leave untrimmed. If this argument is zero,
  only the minimum amount of memory to maintain internal data
  structures will be left (one page or less). Non-zero arguments
  can be supplied to maintain enough trailing space to service
  future expected allocations without having to re-obtain memory
  from the system.

  Malloc_trim returns 1 if it actually released any memory, else 0.
  On systems that do not support "negative sbrks", it will always
  return 0.
*/
int malloc_trim(size_t pad)
{
  int r;
  __MALLOC_LOCK;
  mstate av = get_malloc_state();
  __malloc_consolidate(av);
  r = __malloc_trim(pad, av);
  __MALLOC_UNLOCK;
  return r;
}

/*
  Initialize a malloc_state struct.

  This is called only from within __malloc_consolidate, which needs
  be called in the same contexts anyway.  It is never called directly
  outside of __malloc_consolidate because some optimizing compilers try
  to inline it at all call points, which turns out not to be an
  optimization at all. (Inlining it in __malloc_consolidate is fine though.)
*/
static void malloc_init_state(mstate av)
{
    int     i;
    mbinptr bin;

    /* Establish circular links for normal bins */
    for (i = 1; i < NBINS; ++i) {
	bin = bin_at(av,i);
	bin->fd = bin->bk = bin;
    }

    av->top_pad        = DEFAULT_TOP_PAD;
    av->n_mmaps_max    = DEFAULT_MMAP_MAX;
    av->mmap_threshold = DEFAULT_MMAP_THRESHOLD;
    av->trim_threshold = DEFAULT_TRIM_THRESHOLD;

#if MORECORE_CONTIGUOUS
    set_contiguous(av);
#else
    set_noncontiguous(av);
#endif


    set_max_fast(av, DEFAULT_MXFAST);

    av->top            = initial_top(av);
    av->pagesize       = malloc_getpagesize;
}


/* ----------------------------------------------------------------------
 *
 * PUBLIC STUFF
 *
 * ----------------------------------------------------------------------*/


/* ------------------------- __malloc_consolidate -------------------------

  __malloc_consolidate is a specialized version of free() that tears
  down chunks held in fastbins.  Free itself cannot be used for this
  purpose since, among other things, it might place chunks back onto
  fastbins.  So, instead, we need to use a minor variant of the same
  code.

  Also, because this routine needs to be called the first time through
  malloc anyway, it turns out to be the perfect place to trigger
  initialization code.
*/
void attribute_hidden __malloc_consolidate(mstate av)
{
    mfastbinptr*    fb;                 /* current fastbin being consolidated */
    mfastbinptr*    maxfb;              /* last fastbin (for loop control) */
    mchunkptr       p;                  /* current chunk being consolidated */
    mchunkptr       nextp;              /* next chunk to consolidate */
    mchunkptr       unsorted_bin;       /* bin header */
    mchunkptr       first_unsorted;     /* chunk to link to */

    /* These have same use as in free() */
    mchunkptr       nextchunk;
    size_t size;
    size_t nextsize;
    size_t prevsize;
    int             nextinuse;
    mchunkptr       bck;
    mchunkptr       fwd;

    /*
       If max_fast is 0, we know that av hasn't
       yet been initialized, in which case do so below
       */

    if (av->max_fast != 0) {
	clear_fastchunks(av);

	unsorted_bin = unsorted_chunks(av);

	/*
	   Remove each chunk from fast bin and consolidate it, placing it
	   then in unsorted bin. Among other reasons for doing this,
	   placing in unsorted bin avoids needing to calculate actual bins
	   until malloc is sure that chunks aren't immediately going to be
	   reused anyway.
	   */

	maxfb = &(av->fastbins[fastbin_index(av->max_fast)]);
	fb = &(av->fastbins[0]);
	do {
	    if ( (p = *fb) != 0) {
		*fb = 0;

		do {
		    check_inuse_chunk(p);
		    nextp = p->fd;

		    /* Slightly streamlined version of consolidation code in free() */
		    size = p->size & ~PREV_INUSE;
		    nextchunk = chunk_at_offset(p, size);
		    nextsize = chunksize(nextchunk);

		    if (!prev_inuse(p)) {
			prevsize = p->prev_size;
			size += prevsize;
			p = chunk_at_offset(p, -((long) prevsize));
			unlink(p, bck, fwd);
		    }

		    if (nextchunk != av->top) {
			nextinuse = inuse_bit_at_offset(nextchunk, nextsize);
			set_head(nextchunk, nextsize);

			if (!nextinuse) {
			    size += nextsize;
			    unlink(nextchunk, bck, fwd);
			}

			first_unsorted = unsorted_bin->fd;
			unsorted_bin->fd = p;
			first_unsorted->bk = p;

			set_head(p, size | PREV_INUSE);
			p->bk = unsorted_bin;
			p->fd = first_unsorted;
			set_foot(p, size);
		    }

		    else {
			size += nextsize;
			set_head(p, size | PREV_INUSE);
			av->top = p;
		    }

		} while ( (p = nextp) != 0);

	    }
	} while (fb++ != maxfb);
    }
    else {
	malloc_init_state(av);
	check_malloc_state();
    }
}


/* ------------------------------ free ------------------------------ */
void free(void* mem)
{
    mstate av;

    mchunkptr       p;           /* chunk corresponding to mem */
    size_t size;        /* its size */
    mfastbinptr*    fb;          /* associated fastbin */
    mchunkptr       nextchunk;   /* next contiguous chunk */
    size_t nextsize;    /* its size */
    int             nextinuse;   /* true if nextchunk is used */
    size_t prevsize;    /* size of previous contiguous chunk */
    mchunkptr       bck;         /* misc temp for linking */
    mchunkptr       fwd;         /* misc temp for linking */

    /* free(0) has no effect */
    if (mem == NULL)
	return;

    __MALLOC_LOCK;
    av = get_malloc_state();
    p = mem2chunk(mem);
    size = chunksize(p);

    check_inuse_chunk(p);

    /*
       If eligible, place chunk on a fastbin so it can be found
       and used quickly in malloc.
       */

    if ((unsigned long)(size) <= (unsigned long)(av->max_fast)

#if TRIM_FASTBINS
	    /* If TRIM_FASTBINS set, don't place chunks
	       bordering top into fastbins */
	    && (chunk_at_offset(p, size) != av->top)
#endif
       ) {

	set_fastchunks(av);
	fb = &(av->fastbins[fastbin_index(size)]);
	p->fd = *fb;
	*fb = p;
    }

    /*
       Consolidate other non-mmapped chunks as they arrive.
       */

    else if (!chunk_is_mmapped(p)) {
	set_anychunks(av);

	nextchunk = chunk_at_offset(p, size);
	nextsize = chunksize(nextchunk);

	/* consolidate backward */
	if (!prev_inuse(p)) {
	    prevsize = p->prev_size;
	    size += prevsize;
	    p = chunk_at_offset(p, -((long) prevsize));
	    unlink(p, bck, fwd);
	}

	if (nextchunk != av->top) {
	    /* get and clear inuse bit */
	    nextinuse = inuse_bit_at_offset(nextchunk, nextsize);
	    set_head(nextchunk, nextsize);

	    /* consolidate forward */
	    if (!nextinuse) {
		unlink(nextchunk, bck, fwd);
		size += nextsize;
	    }

	    /*
	       Place the chunk in unsorted chunk list. Chunks are
	       not placed into regular bins until after they have
	       been given one chance to be used in malloc.
	       */

	    bck = unsorted_chunks(av);
	    fwd = bck->fd;
	    p->bk = bck;
	    p->fd = fwd;
	    bck->fd = p;
	    fwd->bk = p;

	    set_head(p, size | PREV_INUSE);
	    set_foot(p, size);

	    check_free_chunk(p);
	}

	/*
	   If the chunk borders the current high end of memory,
	   consolidate into top
	   */

	else {
	    size += nextsize;
	    set_head(p, size | PREV_INUSE);
	    av->top = p;
	    check_chunk(p);
	}

	/*
	   If freeing a large space, consolidate possibly-surrounding
	   chunks. Then, if the total unused topmost memory exceeds trim
	   threshold, ask malloc_trim to reduce top.

	   Unless max_fast is 0, we don't know if there are fastbins
	   bordering top, so we cannot tell for sure whether threshold
	   has been reached unless fastbins are consolidated.  But we
	   don't want to consolidate on each free.  As a compromise,
	   consolidation is performed if FASTBIN_CONSOLIDATION_THRESHOLD
	   is reached.
	   */

	if ((unsigned long)(size) >= FASTBIN_CONSOLIDATION_THRESHOLD) {
	    if (have_fastchunks(av))
		__malloc_consolidate(av);

	    if ((unsigned long)(chunksize(av->top)) >=
		    (unsigned long)(av->trim_threshold))
		__malloc_trim(av->top_pad, av);
	}

    }
    /*
       If the chunk was allocated via mmap, release via munmap()
       Note that if HAVE_MMAP is false but chunk_is_mmapped is
       true, then user must have overwritten memory. There's nothing
       we can do to catch this error unless DEBUG is set, in which case
       check_inuse_chunk (above) will have triggered error.
       */

    else {
	size_t offset = p->prev_size;
	av->n_mmaps--;
	av->mmapped_mem -= (size + offset);
	munmap((char*)p - offset, size + offset);
    }
    __MALLOC_UNLOCK;
}

/* glibc compatibilty  */
weak_alias(free, __libc_free)
