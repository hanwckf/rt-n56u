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


__UCLIBC_MUTEX_INIT(__malloc_lock, PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP);

/*
   There is exactly one instance of this struct in this malloc.
   If you are adapting this malloc in a way that does NOT use a static
   malloc_state, you MUST explicitly zero-fill it before using. This
   malloc relies on the property that malloc_state is initialized to
   all zeroes (as is true of C statics).
*/
struct malloc_state __malloc_state;  /* never directly referenced */

/* forward declaration */
static int __malloc_largebin_index(unsigned int sz);

#ifdef __UCLIBC_MALLOC_DEBUGGING__

/*
  Debugging support

  Because freed chunks may be overwritten with bookkeeping fields, this
  malloc will often die when freed memory is overwritten by user
  programs.  This can be very effective (albeit in an annoying way)
  in helping track down dangling pointers.

  If you compile with __UCLIBC_MALLOC_DEBUGGING__, a number of assertion checks are
  enabled that will catch more memory errors. You probably won't be
  able to make much sense of the actual assertion errors, but they
  should help you locate incorrectly overwritten memory.  The
  checking is fairly extensive, and will slow down execution
  noticeably. Calling malloc_stats or mallinfo with __UCLIBC_MALLOC_DEBUGGING__ set will
  attempt to check every non-mmapped allocated and free chunk in the
  course of computing the summmaries. (By nature, mmapped regions
  cannot be checked very much automatically.)

  Setting __UCLIBC_MALLOC_DEBUGGING__ may also be helpful if you are trying to modify
  this code. The assertions in the check routines spell out in more
  detail the assumptions and invariants underlying the algorithms.

  Setting __UCLIBC_MALLOC_DEBUGGING__ does NOT provide an automated mechanism for checking
  that all accesses to malloced memory stay within their
  bounds. However, there are several add-ons and adaptations of this
  or other mallocs available that do this.
*/

/* Properties of all chunks */
void __do_check_chunk(mchunkptr p)
{
    mstate av = get_malloc_state();
#ifdef __DOASSERTS__
    /* min and max possible addresses assuming contiguous allocation */
    char* max_address = (char*)(av->top) + chunksize(av->top);
    char* min_address = max_address - av->sbrked_mem;
    unsigned long  sz = chunksize(p);
#endif

    if (!chunk_is_mmapped(p)) {

	/* Has legal address ... */
	if (p != av->top) {
	    if (contiguous(av)) {
		assert(((char*)p) >= min_address);
		assert(((char*)p + sz) <= ((char*)(av->top)));
	    }
	}
	else {
	    /* top size is always at least MINSIZE */
	    assert((unsigned long)(sz) >= MINSIZE);
	    /* top predecessor always marked inuse */
	    assert(prev_inuse(p));
	}

    }
    else {
	/* address is outside main heap  */
	if (contiguous(av) && av->top != initial_top(av)) {
	    assert(((char*)p) < min_address || ((char*)p) > max_address);
	}
	/* chunk is page-aligned */
	assert(((p->prev_size + sz) & (av->pagesize-1)) == 0);
	/* mem is aligned */
	assert(aligned_OK(chunk2mem(p)));
    }
}

/* Properties of free chunks */
void __do_check_free_chunk(mchunkptr p)
{
    size_t sz = p->size & ~PREV_INUSE;
#ifdef __DOASSERTS__
    mstate av = get_malloc_state();
    mchunkptr next = chunk_at_offset(p, sz);
#endif

    __do_check_chunk(p);

    /* Chunk must claim to be free ... */
    assert(!inuse(p));
    assert (!chunk_is_mmapped(p));

    /* Unless a special marker, must have OK fields */
    if ((unsigned long)(sz) >= MINSIZE)
    {
	assert((sz & MALLOC_ALIGN_MASK) == 0);
	assert(aligned_OK(chunk2mem(p)));
	/* ... matching footer field */
	assert(next->prev_size == sz);
	/* ... and is fully consolidated */
	assert(prev_inuse(p));
	assert (next == av->top || inuse(next));

	/* ... and has minimally sane links */
	assert(p->fd->bk == p);
	assert(p->bk->fd == p);
    }
    else /* markers are always of size (sizeof(size_t)) */
	assert(sz == (sizeof(size_t)));
}

/* Properties of inuse chunks */
void __do_check_inuse_chunk(mchunkptr p)
{
    mstate av = get_malloc_state();
    mchunkptr next;
    __do_check_chunk(p);

    if (chunk_is_mmapped(p))
	return; /* mmapped chunks have no next/prev */

    /* Check whether it claims to be in use ... */
    assert(inuse(p));

    next = next_chunk(p);

    /* ... and is surrounded by OK chunks.
       Since more things can be checked with free chunks than inuse ones,
       if an inuse chunk borders them and debug is on, it's worth doing them.
       */
    if (!prev_inuse(p))  {
	/* Note that we cannot even look at prev unless it is not inuse */
	mchunkptr prv = prev_chunk(p);
	assert(next_chunk(prv) == p);
	__do_check_free_chunk(prv);
    }

    if (next == av->top) {
	assert(prev_inuse(next));
	assert(chunksize(next) >= MINSIZE);
    }
    else if (!inuse(next))
	__do_check_free_chunk(next);
}

/* Properties of chunks recycled from fastbins */
void __do_check_remalloced_chunk(mchunkptr p, size_t s)
{
#ifdef __DOASSERTS__
    size_t sz = p->size & ~PREV_INUSE;
#endif

    __do_check_inuse_chunk(p);

    /* Legal size ... */
    assert((sz & MALLOC_ALIGN_MASK) == 0);
    assert((unsigned long)(sz) >= MINSIZE);
    /* ... and alignment */
    assert(aligned_OK(chunk2mem(p)));
    /* chunk is less than MINSIZE more than request */
    assert((long)(sz) - (long)(s) >= 0);
    assert((long)(sz) - (long)(s + MINSIZE) < 0);
}

/* Properties of nonrecycled chunks at the point they are malloced */
void __do_check_malloced_chunk(mchunkptr p, size_t s)
{
    /* same as recycled case ... */
    __do_check_remalloced_chunk(p, s);

    /*
       ... plus,  must obey implementation invariant that prev_inuse is
       always true of any allocated chunk; i.e., that each allocated
       chunk borders either a previously allocated and still in-use
       chunk, or the base of its memory arena. This is ensured
       by making all allocations from the the `lowest' part of any found
       chunk.  This does not necessarily hold however for chunks
       recycled via fastbins.
       */

    assert(prev_inuse(p));
}


/*
  Properties of malloc_state.

  This may be useful for debugging malloc, as well as detecting user
  programmer errors that somehow write into malloc_state.

  If you are extending or experimenting with this malloc, you can
  probably figure out how to hack this routine to print out or
  display chunk addresses, sizes, bins, and other instrumentation.
*/
void __do_check_malloc_state(void)
{
    mstate av = get_malloc_state();
    int i;
    mchunkptr p;
    mchunkptr q;
    mbinptr b;
    unsigned int binbit;
    int empty;
    unsigned int idx;
    size_t size;
    unsigned long  total = 0;
    int max_fast_bin;

    /* internal size_t must be no wider than pointer type */
    assert(sizeof(size_t) <= sizeof(char*));

    /* alignment is a power of 2 */
    assert((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-1)) == 0);

    /* cannot run remaining checks until fully initialized */
    if (av->top == 0 || av->top == initial_top(av))
	return;

    /* pagesize is a power of 2 */
    assert((av->pagesize & (av->pagesize-1)) == 0);

    /* properties of fastbins */

    /* max_fast is in allowed range */
    assert(get_max_fast(av) <= request2size(MAX_FAST_SIZE));

    max_fast_bin = fastbin_index(av->max_fast);

    for (i = 0; i < NFASTBINS; ++i) {
	p = av->fastbins[i];

	/* all bins past max_fast are empty */
	if (i > max_fast_bin)
	    assert(p == 0);

	while (p != 0) {
	    /* each chunk claims to be inuse */
	    __do_check_inuse_chunk(p);
	    total += chunksize(p);
	    /* chunk belongs in this bin */
	    assert(fastbin_index(chunksize(p)) == i);
	    p = p->fd;
	}
    }

    if (total != 0)
	assert(have_fastchunks(av));
    else if (!have_fastchunks(av))
	assert(total == 0);

    /* check normal bins */
    for (i = 1; i < NBINS; ++i) {
	b = bin_at(av,i);

	/* binmap is accurate (except for bin 1 == unsorted_chunks) */
	if (i >= 2) {
	    binbit = get_binmap(av,i);
	    empty = last(b) == b;
	    if (!binbit)
		assert(empty);
	    else if (!empty)
		assert(binbit);
	}

	for (p = last(b); p != b; p = p->bk) {
	    /* each chunk claims to be free */
	    __do_check_free_chunk(p);
	    size = chunksize(p);
	    total += size;
	    if (i >= 2) {
		/* chunk belongs in bin */
		idx = bin_index(size);
		assert(idx == i);
		/* lists are sorted */
		if ((unsigned long) size >= (unsigned long)(FIRST_SORTED_BIN_SIZE)) {
		    assert(p->bk == b ||
			    (unsigned long)chunksize(p->bk) >=
			    (unsigned long)chunksize(p));
		}
	    }
	    /* chunk is followed by a legal chain of inuse chunks */
	    for (q = next_chunk(p);
		    (q != av->top && inuse(q) &&
		     (unsigned long)(chunksize(q)) >= MINSIZE);
		    q = next_chunk(q))
		__do_check_inuse_chunk(q);
	}
    }

    /* top chunk is OK */
    __do_check_chunk(av->top);

    /* sanity checks for statistics */

    assert(total <= (unsigned long)(av->max_total_mem));
    assert(av->n_mmaps >= 0);
    assert(av->n_mmaps <= av->max_n_mmaps);

    assert((unsigned long)(av->sbrked_mem) <=
	    (unsigned long)(av->max_sbrked_mem));

    assert((unsigned long)(av->mmapped_mem) <=
	    (unsigned long)(av->max_mmapped_mem));

    assert((unsigned long)(av->max_total_mem) >=
	    (unsigned long)(av->mmapped_mem) + (unsigned long)(av->sbrked_mem));
}
#endif


/* ----------- Routines dealing with system allocation -------------- */

/*
  sysmalloc handles malloc cases requiring more memory from the system.
  On entry, it is assumed that av->top does not have enough
  space to service request for nb bytes, thus requiring that av->top
  be extended or replaced.
*/
static void* __malloc_alloc(size_t nb, mstate av)
{
    mchunkptr       old_top;        /* incoming value of av->top */
    size_t old_size;       /* its size */
    char*           old_end;        /* its end address */

    long            size;           /* arg to first MORECORE or mmap call */
    char*           fst_brk;        /* return value from MORECORE */

    long            correction;     /* arg to 2nd MORECORE call */
    char*           snd_brk;        /* 2nd return val */

    size_t front_misalign; /* unusable bytes at front of new space */
    size_t end_misalign;   /* partial page left at end of new space */
    char*           aligned_brk;    /* aligned offset into brk */

    mchunkptr       p;              /* the allocated/returned chunk */
    mchunkptr       remainder;      /* remainder from allocation */
    unsigned long    remainder_size; /* its size */

    unsigned long    sum;            /* for updating stats */

    size_t          pagemask  = av->pagesize - 1;

    /*
       If there is space available in fastbins, consolidate and retry
       malloc from scratch rather than getting memory from system.  This
       can occur only if nb is in smallbin range so we didn't consolidate
       upon entry to malloc. It is much easier to handle this case here
       than in malloc proper.
       */

    if (have_fastchunks(av)) {
	assert(in_smallbin_range(nb));
	__malloc_consolidate(av);
	return malloc(nb - MALLOC_ALIGN_MASK);
    }


    /*
       If have mmap, and the request size meets the mmap threshold, and
       the system supports mmap, and there are few enough currently
       allocated mmapped regions, try to directly map this request
       rather than expanding top.
       */

    if ((unsigned long)(nb) >= (unsigned long)(av->mmap_threshold) &&
	    (av->n_mmaps < av->n_mmaps_max)) {

	char* mm;             /* return value from mmap call*/

	/*
	   Round up size to nearest page.  For mmapped chunks, the overhead
	   is one (sizeof(size_t)) unit larger than for normal chunks, because there
	   is no following chunk whose prev_size field could be used.
	   */
	size = (nb + (sizeof(size_t)) + MALLOC_ALIGN_MASK + pagemask) & ~pagemask;

	/* Don't try if size wraps around 0 */
	if ((unsigned long)(size) > (unsigned long)(nb)) {

	    mm = (char*)(MMAP(0, size, PROT_READ|PROT_WRITE));

	    if (mm != (char*)(MORECORE_FAILURE)) {

		/*
		   The offset to the start of the mmapped region is stored
		   in the prev_size field of the chunk. This allows us to adjust
		   returned start address to meet alignment requirements here
		   and in memalign(), and still be able to compute proper
		   address argument for later munmap in free() and realloc().
		   */

		front_misalign = (size_t)chunk2mem(mm) & MALLOC_ALIGN_MASK;
		if (front_misalign > 0) {
		    correction = MALLOC_ALIGNMENT - front_misalign;
		    p = (mchunkptr)(mm + correction);
		    p->prev_size = correction;
		    set_head(p, (size - correction) |IS_MMAPPED);
		}
		else {
		    p = (mchunkptr)mm;
		    p->prev_size = 0;
		    set_head(p, size|IS_MMAPPED);
		}

		/* update statistics */

		if (++av->n_mmaps > av->max_n_mmaps)
		    av->max_n_mmaps = av->n_mmaps;

		sum = av->mmapped_mem += size;
		if (sum > (unsigned long)(av->max_mmapped_mem))
		    av->max_mmapped_mem = sum;
		sum += av->sbrked_mem;
		if (sum > (unsigned long)(av->max_total_mem))
		    av->max_total_mem = sum;

		check_chunk(p);

		return chunk2mem(p);
	    }
	}
    }

    /* Record incoming configuration of top */

    old_top  = av->top;
    old_size = chunksize(old_top);
    old_end  = (char*)(chunk_at_offset(old_top, old_size));

    fst_brk = snd_brk = (char*)(MORECORE_FAILURE);

    /* If not the first time through, we require old_size to
     * be at least MINSIZE and to have prev_inuse set.  */

    assert((old_top == initial_top(av) && old_size == 0) ||
	    ((unsigned long) (old_size) >= MINSIZE &&
	     prev_inuse(old_top)));

    /* Precondition: not enough current space to satisfy nb request */
    assert((unsigned long)(old_size) < (unsigned long)(nb + MINSIZE));

    /* Precondition: all fastbins are consolidated */
    assert(!have_fastchunks(av));


    /* Request enough space for nb + pad + overhead */

    size = nb + av->top_pad + MINSIZE;

    /*
       If contiguous, we can subtract out existing space that we hope to
       combine with new space. We add it back later only if
       we don't actually get contiguous space.
       */

    if (contiguous(av))
	size -= old_size;

    /*
       Round to a multiple of page size.
       If MORECORE is not contiguous, this ensures that we only call it
       with whole-page arguments.  And if MORECORE is contiguous and
       this is not first time through, this preserves page-alignment of
       previous calls. Otherwise, we correct to page-align below.
       */

    size = (size + pagemask) & ~pagemask;

    /*
       Don't try to call MORECORE if argument is so big as to appear
       negative. Note that since mmap takes size_t arg, it may succeed
       below even if we cannot call MORECORE.
       */

    if (size > 0)
	fst_brk = (char*)(MORECORE(size));

    /*
       If have mmap, try using it as a backup when MORECORE fails or
       cannot be used. This is worth doing on systems that have "holes" in
       address space, so sbrk cannot extend to give contiguous space, but
       space is available elsewhere.  Note that we ignore mmap max count
       and threshold limits, since the space will not be used as a
       segregated mmap region.
       */

    if (fst_brk == (char*)(MORECORE_FAILURE)) {

	/* Cannot merge with old top, so add its size back in */
	if (contiguous(av))
	    size = (size + old_size + pagemask) & ~pagemask;

	/* If we are relying on mmap as backup, then use larger units */
	if ((unsigned long)(size) < (unsigned long)(MMAP_AS_MORECORE_SIZE))
	    size = MMAP_AS_MORECORE_SIZE;

	/* Don't try if size wraps around 0 */
	if ((unsigned long)(size) > (unsigned long)(nb)) {

	    fst_brk = (char*)(MMAP(0, size, PROT_READ|PROT_WRITE));

	    if (fst_brk != (char*)(MORECORE_FAILURE)) {

		/* We do not need, and cannot use, another sbrk call to find end */
		snd_brk = fst_brk + size;

		/* Record that we no longer have a contiguous sbrk region.
		   After the first time mmap is used as backup, we do not
		   ever rely on contiguous space since this could incorrectly
		   bridge regions.
		   */
		set_noncontiguous(av);
	    }
	}
    }

    if (fst_brk != (char*)(MORECORE_FAILURE)) {
	av->sbrked_mem += size;

	/*
	   If MORECORE extends previous space, we can likewise extend top size.
	   */

	if (fst_brk == old_end && snd_brk == (char*)(MORECORE_FAILURE)) {
	    set_head(old_top, (size + old_size) | PREV_INUSE);
	}

	/*
	   Otherwise, make adjustments:

	 * If the first time through or noncontiguous, we need to call sbrk
	 just to find out where the end of memory lies.

	 * We need to ensure that all returned chunks from malloc will meet
	 MALLOC_ALIGNMENT

	 * If there was an intervening foreign sbrk, we need to adjust sbrk
	 request size to account for fact that we will not be able to
	 combine new space with existing space in old_top.

	 * Almost all systems internally allocate whole pages at a time, in
	 which case we might as well use the whole last page of request.
	 So we allocate enough more memory to hit a page boundary now,
	 which in turn causes future contiguous calls to page-align.
	 */

	else {
	    front_misalign = 0;
	    end_misalign = 0;
	    correction = 0;
	    aligned_brk = fst_brk;

	    /*
	       If MORECORE returns an address lower than we have seen before,
	       we know it isn't really contiguous.  This and some subsequent
	       checks help cope with non-conforming MORECORE functions and
	       the presence of "foreign" calls to MORECORE from outside of
	       malloc or by other threads.  We cannot guarantee to detect
	       these in all cases, but cope with the ones we do detect.
	       */
	    if (contiguous(av) && old_size != 0 && fst_brk < old_end) {
		set_noncontiguous(av);
	    }

	    /* handle contiguous cases */
	    if (contiguous(av)) {

		/* We can tolerate forward non-contiguities here (usually due
		   to foreign calls) but treat them as part of our space for
		   stats reporting.  */
		if (old_size != 0)
		    av->sbrked_mem += fst_brk - old_end;

		/* Guarantee alignment of first new chunk made from this space */

		front_misalign = (size_t)chunk2mem(fst_brk) & MALLOC_ALIGN_MASK;
		if (front_misalign > 0) {

		    /*
		       Skip over some bytes to arrive at an aligned position.
		       We don't need to specially mark these wasted front bytes.
		       They will never be accessed anyway because
		       prev_inuse of av->top (and any chunk created from its start)
		       is always true after initialization.
		       */

		    correction = MALLOC_ALIGNMENT - front_misalign;
		    aligned_brk += correction;
		}

		/*
		   If this isn't adjacent to existing space, then we will not
		   be able to merge with old_top space, so must add to 2nd request.
		   */

		correction += old_size;

		/* Extend the end address to hit a page boundary */
		end_misalign = (size_t)(fst_brk + size + correction);
		correction += ((end_misalign + pagemask) & ~pagemask) - end_misalign;

		assert(correction >= 0);
		snd_brk = (char*)(MORECORE(correction));

		if (snd_brk == (char*)(MORECORE_FAILURE)) {
		    /*
		       If can't allocate correction, try to at least find out current
		       brk.  It might be enough to proceed without failing.
		       */
		    correction = 0;
		    snd_brk = (char*)(MORECORE(0));
		}
		else if (snd_brk < fst_brk) {
		    /*
		       If the second call gives noncontiguous space even though
		       it says it won't, the only course of action is to ignore
		       results of second call, and conservatively estimate where
		       the first call left us. Also set noncontiguous, so this
		       won't happen again, leaving at most one hole.

		       Note that this check is intrinsically incomplete.  Because
		       MORECORE is allowed to give more space than we ask for,
		       there is no reliable way to detect a noncontiguity
		       producing a forward gap for the second call.
		       */
		    snd_brk = fst_brk + size;
		    correction = 0;
		    set_noncontiguous(av);
		}

	    }

	    /* handle non-contiguous cases */
	    else {
		/* MORECORE/mmap must correctly align */
		assert(aligned_OK(chunk2mem(fst_brk)));

		/* Find out current end of memory */
		if (snd_brk == (char*)(MORECORE_FAILURE)) {
		    snd_brk = (char*)(MORECORE(0));
		    av->sbrked_mem += snd_brk - fst_brk - size;
		}
	    }

	    /* Adjust top based on results of second sbrk */
	    if (snd_brk != (char*)(MORECORE_FAILURE)) {
		av->top = (mchunkptr)aligned_brk;
		set_head(av->top, (snd_brk - aligned_brk + correction) | PREV_INUSE);
		av->sbrked_mem += correction;

		/*
		   If not the first time through, we either have a
		   gap due to foreign sbrk or a non-contiguous region.  Insert a
		   double fencepost at old_top to prevent consolidation with space
		   we don't own. These fenceposts are artificial chunks that are
		   marked as inuse and are in any case too small to use.  We need
		   two to make sizes and alignments work out.
		   */

		if (old_size != 0) {
		    /* Shrink old_top to insert fenceposts, keeping size a
		       multiple of MALLOC_ALIGNMENT. We know there is at least
		       enough space in old_top to do this.
		       */
		    old_size = (old_size - 3*(sizeof(size_t))) & ~MALLOC_ALIGN_MASK;
		    set_head(old_top, old_size | PREV_INUSE);

		    /*
		       Note that the following assignments completely overwrite
		       old_top when old_size was previously MINSIZE.  This is
		       intentional. We need the fencepost, even if old_top otherwise gets
		       lost.
		       */
		    chunk_at_offset(old_top, old_size          )->size =
			(sizeof(size_t))|PREV_INUSE;

		    chunk_at_offset(old_top, old_size + (sizeof(size_t)))->size =
			(sizeof(size_t))|PREV_INUSE;

		    /* If possible, release the rest, suppressing trimming.  */
		    if (old_size >= MINSIZE) {
			size_t tt = av->trim_threshold;
			av->trim_threshold = (size_t)(-1);
			free(chunk2mem(old_top));
			av->trim_threshold = tt;
		    }
		}
	    }
	}

	/* Update statistics */
	sum = av->sbrked_mem;
	if (sum > (unsigned long)(av->max_sbrked_mem))
	    av->max_sbrked_mem = sum;

	sum += av->mmapped_mem;
	if (sum > (unsigned long)(av->max_total_mem))
	    av->max_total_mem = sum;

	check_malloc_state();

	/* finally, do the allocation */

	p = av->top;
	size = chunksize(p);

	/* check that one of the above allocation paths succeeded */
	if ((unsigned long)(size) >= (unsigned long)(nb + MINSIZE)) {
	    remainder_size = size - nb;
	    remainder = chunk_at_offset(p, nb);
	    av->top = remainder;
	    set_head(p, nb | PREV_INUSE);
	    set_head(remainder, remainder_size | PREV_INUSE);
	    check_malloced_chunk(p, nb);
	    return chunk2mem(p);
	}

    }

    /* catch all failure paths */
    errno = ENOMEM;
    return 0;
}


/*
  Compute index for size. We expect this to be inlined when
  compiled with optimization, else not, which works out well.
*/
static int __malloc_largebin_index(unsigned int sz)
{
    unsigned int  x = sz >> SMALLBIN_WIDTH;
    unsigned int m;            /* bit position of highest set bit of m */

    if (x >= 0x10000) return NBINS-1;

    /* On intel, use BSRL instruction to find highest bit */
#if defined(__GNUC__) && defined(i386)

    __asm__("bsrl %1,%0\n\t"
	    : "=r" (m)
	    : "g"  (x));

#else
    {
	/*
	   Based on branch-free nlz algorithm in chapter 5 of Henry
	   S. Warren Jr's book "Hacker's Delight".
	   */

	unsigned int n = ((x - 0x100) >> 16) & 8;
	x <<= n;
	m = ((x - 0x1000) >> 16) & 4;
	n += m;
	x <<= m;
	m = ((x - 0x4000) >> 16) & 2;
	n += m;
	x = (x << m) >> 14;
	m = 13 - n + (x & ~(x>>1));
    }
#endif

    /* Use next 2 bits to create finer-granularity bins */
    return NSMALLBINS + (m << 2) + ((sz >> (m + 6)) & 3);
}



/* ----------------------------------------------------------------------
 *
 * PUBLIC STUFF
 *
 * ----------------------------------------------------------------------*/


/* ------------------------------ malloc ------------------------------ */
void* malloc(size_t bytes)
{
    mstate av;

    size_t nb;               /* normalized request size */
    unsigned int    idx;              /* associated bin index */
    mbinptr         bin;              /* associated bin */
    mfastbinptr*    fb;               /* associated fastbin */

    mchunkptr       victim;           /* inspected/selected chunk */
    size_t size;             /* its size */
    int             victim_index;     /* its bin index */

    mchunkptr       remainder;        /* remainder from a split */
    unsigned long    remainder_size;   /* its size */

    unsigned int    block;            /* bit map traverser */
    unsigned int    bit;              /* bit map traverser */
    unsigned int    map;              /* current word of binmap */

    mchunkptr       fwd;              /* misc temp for linking */
    mchunkptr       bck;              /* misc temp for linking */
    void *          sysmem;
    void *          retval;

#if !defined(__MALLOC_GLIBC_COMPAT__)
    if (!bytes) {
        __set_errno(ENOMEM);
        return NULL;
    }
#endif

    /*
       Convert request size to internal form by adding (sizeof(size_t)) bytes
       overhead plus possibly more to obtain necessary alignment and/or
       to obtain a size of at least MINSIZE, the smallest allocatable
       size. Also, checked_request2size traps (returning 0) request sizes
       that are so large that they wrap around zero when padded and
       aligned.
       */

    checked_request2size(bytes, nb);

    __MALLOC_LOCK;
    av = get_malloc_state();

    /*
       Bypass search if no frees yet
       */
    if (!have_anychunks(av)) {
	if (av->max_fast == 0) /* initialization check */
	    __malloc_consolidate(av);
	goto use_top;
    }

    /*
       If the size qualifies as a fastbin, first check corresponding bin.
       */

    if ((unsigned long)(nb) <= (unsigned long)(av->max_fast)) {
	fb = &(av->fastbins[(fastbin_index(nb))]);
	if ( (victim = *fb) != 0) {
	    *fb = victim->fd;
	    check_remalloced_chunk(victim, nb);
	    retval = chunk2mem(victim);
	    goto DONE;
	}
    }

    /*
       If a small request, check regular bin.  Since these "smallbins"
       hold one size each, no searching within bins is necessary.
       (For a large request, we need to wait until unsorted chunks are
       processed to find best fit. But for small ones, fits are exact
       anyway, so we can check now, which is faster.)
       */

    if (in_smallbin_range(nb)) {
	idx = smallbin_index(nb);
	bin = bin_at(av,idx);

	if ( (victim = last(bin)) != bin) {
	    bck = victim->bk;
	    set_inuse_bit_at_offset(victim, nb);
	    bin->bk = bck;
	    bck->fd = bin;

	    check_malloced_chunk(victim, nb);
	    retval = chunk2mem(victim);
	    goto DONE;
	}
    }

    /* If this is a large request, consolidate fastbins before continuing.
       While it might look excessive to kill all fastbins before
       even seeing if there is space available, this avoids
       fragmentation problems normally associated with fastbins.
       Also, in practice, programs tend to have runs of either small or
       large requests, but less often mixtures, so consolidation is not
       invoked all that often in most programs. And the programs that
       it is called frequently in otherwise tend to fragment.
       */

    else {
	idx = __malloc_largebin_index(nb);
	if (have_fastchunks(av))
	    __malloc_consolidate(av);
    }

    /*
       Process recently freed or remaindered chunks, taking one only if
       it is exact fit, or, if this a small request, the chunk is remainder from
       the most recent non-exact fit.  Place other traversed chunks in
       bins.  Note that this step is the only place in any routine where
       chunks are placed in bins.
       */

    while ( (victim = unsorted_chunks(av)->bk) != unsorted_chunks(av)) {
	bck = victim->bk;
	size = chunksize(victim);

	/* If a small request, try to use last remainder if it is the
	   only chunk in unsorted bin.  This helps promote locality for
	   runs of consecutive small requests. This is the only
	   exception to best-fit, and applies only when there is
	   no exact fit for a small chunk.
	   */

	if (in_smallbin_range(nb) &&
		bck == unsorted_chunks(av) &&
		victim == av->last_remainder &&
		(unsigned long)(size) > (unsigned long)(nb + MINSIZE)) {

	    /* split and reattach remainder */
	    remainder_size = size - nb;
	    remainder = chunk_at_offset(victim, nb);
	    unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
	    av->last_remainder = remainder;
	    remainder->bk = remainder->fd = unsorted_chunks(av);

	    set_head(victim, nb | PREV_INUSE);
	    set_head(remainder, remainder_size | PREV_INUSE);
	    set_foot(remainder, remainder_size);

	    check_malloced_chunk(victim, nb);
	    retval = chunk2mem(victim);
	    goto DONE;
	}

	/* remove from unsorted list */
	unsorted_chunks(av)->bk = bck;
	bck->fd = unsorted_chunks(av);

	/* Take now instead of binning if exact fit */

	if (size == nb) {
	    set_inuse_bit_at_offset(victim, size);
	    check_malloced_chunk(victim, nb);
	    retval = chunk2mem(victim);
	    goto DONE;
	}

	/* place chunk in bin */

	if (in_smallbin_range(size)) {
	    victim_index = smallbin_index(size);
	    bck = bin_at(av, victim_index);
	    fwd = bck->fd;
	}
	else {
	    victim_index = __malloc_largebin_index(size);
	    bck = bin_at(av, victim_index);
	    fwd = bck->fd;

	    if (fwd != bck) {
		/* if smaller than smallest, place first */
		if ((unsigned long)(size) < (unsigned long)(bck->bk->size)) {
		    fwd = bck;
		    bck = bck->bk;
		}
		else if ((unsigned long)(size) >=
			(unsigned long)(FIRST_SORTED_BIN_SIZE)) {

		    /* maintain large bins in sorted order */
		    size |= PREV_INUSE; /* Or with inuse bit to speed comparisons */
		    while ((unsigned long)(size) < (unsigned long)(fwd->size))
			fwd = fwd->fd;
		    bck = fwd->bk;
		}
	    }
	}

	mark_bin(av, victim_index);
	victim->bk = bck;
	victim->fd = fwd;
	fwd->bk = victim;
	bck->fd = victim;
    }

    /*
       If a large request, scan through the chunks of current bin to
       find one that fits.  (This will be the smallest that fits unless
       FIRST_SORTED_BIN_SIZE has been changed from default.)  This is
       the only step where an unbounded number of chunks might be
       scanned without doing anything useful with them. However the
       lists tend to be short.
       */

    if (!in_smallbin_range(nb)) {
	bin = bin_at(av, idx);

	for (victim = last(bin); victim != bin; victim = victim->bk) {
	    size = chunksize(victim);

	    if ((unsigned long)(size) >= (unsigned long)(nb)) {
		remainder_size = size - nb;
		unlink(victim, bck, fwd);

		/* Exhaust */
		if (remainder_size < MINSIZE)  {
		    set_inuse_bit_at_offset(victim, size);
		    check_malloced_chunk(victim, nb);
		    retval = chunk2mem(victim);
		    goto DONE;
		}
		/* Split */
		else {
		    remainder = chunk_at_offset(victim, nb);
		    unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
		    remainder->bk = remainder->fd = unsorted_chunks(av);
		    set_head(victim, nb | PREV_INUSE);
		    set_head(remainder, remainder_size | PREV_INUSE);
		    set_foot(remainder, remainder_size);
		    check_malloced_chunk(victim, nb);
		    retval = chunk2mem(victim);
		    goto DONE;
		}
	    }
	}
    }

    /*
       Search for a chunk by scanning bins, starting with next largest
       bin. This search is strictly by best-fit; i.e., the smallest
       (with ties going to approximately the least recently used) chunk
       that fits is selected.

       The bitmap avoids needing to check that most blocks are nonempty.
       */

    ++idx;
    bin = bin_at(av,idx);
    block = idx2block(idx);
    map = av->binmap[block];
    bit = idx2bit(idx);

    for (;;) {

	/* Skip rest of block if there are no more set bits in this block.  */
	if (bit > map || bit == 0) {
	    do {
		if (++block >= BINMAPSIZE)  /* out of bins */
		    goto use_top;
	    } while ( (map = av->binmap[block]) == 0);

	    bin = bin_at(av, (block << BINMAPSHIFT));
	    bit = 1;
	}

	/* Advance to bin with set bit. There must be one. */
	while ((bit & map) == 0) {
	    bin = next_bin(bin);
	    bit <<= 1;
	    assert(bit != 0);
	}

	/* Inspect the bin. It is likely to be non-empty */
	victim = last(bin);

	/*  If a false alarm (empty bin), clear the bit. */
	if (victim == bin) {
	    av->binmap[block] = map &= ~bit; /* Write through */
	    bin = next_bin(bin);
	    bit <<= 1;
	}

	else {
	    size = chunksize(victim);

	    /*  We know the first chunk in this bin is big enough to use. */
	    assert((unsigned long)(size) >= (unsigned long)(nb));

	    remainder_size = size - nb;

	    /* unlink */
	    bck = victim->bk;
	    bin->bk = bck;
	    bck->fd = bin;

	    /* Exhaust */
	    if (remainder_size < MINSIZE) {
		set_inuse_bit_at_offset(victim, size);
		check_malloced_chunk(victim, nb);
		retval = chunk2mem(victim);
		goto DONE;
	    }

	    /* Split */
	    else {
		remainder = chunk_at_offset(victim, nb);

		unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
		remainder->bk = remainder->fd = unsorted_chunks(av);
		/* advertise as last remainder */
		if (in_smallbin_range(nb))
		    av->last_remainder = remainder;

		set_head(victim, nb | PREV_INUSE);
		set_head(remainder, remainder_size | PREV_INUSE);
		set_foot(remainder, remainder_size);
		check_malloced_chunk(victim, nb);
		retval = chunk2mem(victim);
		goto DONE;
	    }
	}
    }

use_top:
    /*
       If large enough, split off the chunk bordering the end of memory
       (held in av->top). Note that this is in accord with the best-fit
       search rule.  In effect, av->top is treated as larger (and thus
       less well fitting) than any other available chunk since it can
       be extended to be as large as necessary (up to system
       limitations).

       We require that av->top always exists (i.e., has size >=
       MINSIZE) after initialization, so if it would otherwise be
       exhuasted by current request, it is replenished. (The main
       reason for ensuring it exists is that we may need MINSIZE space
       to put in fenceposts in sysmalloc.)
       */

    victim = av->top;
    size = chunksize(victim);

    if ((unsigned long)(size) >= (unsigned long)(nb + MINSIZE)) {
	remainder_size = size - nb;
	remainder = chunk_at_offset(victim, nb);
	av->top = remainder;
	set_head(victim, nb | PREV_INUSE);
	set_head(remainder, remainder_size | PREV_INUSE);

	check_malloced_chunk(victim, nb);
	retval = chunk2mem(victim);
	goto DONE;
    }

    /* If no space in top, relay to handle system-dependent cases */
    sysmem = __malloc_alloc(nb, av);
    retval = sysmem;
DONE:
    __MALLOC_UNLOCK;
    return retval;
}

