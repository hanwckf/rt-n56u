/* Thread-local storage handling in the ELF dynamic linker.  Generic version.
   Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#if defined SHARED || defined NOT_IN_libc
# error in buildsystem: This file is for libc.a
#endif
#include <signal.h>
#include <stdlib.h>
#include <sys/param.h>
#include <tls.h>
#include <dl-tls.h>
#include <ldsodefs.h>
#include <dl-elf.h>
#include <dl-hash.h>

#include <assert.h>
#include <link.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define	_dl_malloc	malloc
#define _dl_memset	memset
#define _dl_mempcpy	mempcpy
#define _dl_dprintf	fprintf
#define _dl_debug_file	stderr
#define _dl_exit	exit

/* Amount of excess space to allocate in the static TLS area
   to allow dynamic loading of modules defining IE-model TLS data.  */
# define TLS_STATIC_SURPLUS	64 + DL_NNS * 100

/* Value used for dtv entries for which the allocation is delayed.  */
# define TLS_DTV_UNALLOCATED	((void *) -1l)

#ifndef SHARED
extern dtv_t static_dtv;
#endif

/* Out-of-memory handler.  */
# ifdef SHARED
static void
__attribute__ ((__noreturn__))
oom (void)
{
	do {
		_dl_dprintf (_dl_debug_file,
			"cannot allocate thread-local memory: ABORT\n");
		_dl_exit (127);
	} while (1);
}
# endif


void *_dl_memalign(size_t alignment, size_t bytes);
void *_dl_memalign(size_t alignment, size_t bytes)
{
	return _dl_malloc(bytes);
}


/*
 * We are trying to perform a static TLS relocation in MAP, but it was
 * dynamically loaded.  This can only work if there is enough surplus in
 * the static TLS area already allocated for each running thread.  If this
 * object's TLS segment is too big to fit, we fail.  If it fits,
 * we set MAP->l_tls_offset and return.
 * This function intentionally does not return any value but signals error
 * directly, as static TLS should be rare and code handling it should
 * not be inlined as much as possible.
 */


void
internal_function __attribute_noinline__
_dl_allocate_static_tls (struct link_map *map)
{
	/* If the alignment requirements are too high fail.  */
	if (map->l_tls_align > _dl_tls_static_align)
	{
fail:
		_dl_dprintf(_dl_debug_file, "cannot allocate memory in static TLS block");
		_dl_exit(30);
	}

# if defined(TLS_TCB_AT_TP)
	size_t freebytes;
	size_t n;
	size_t blsize;

	freebytes = _dl_tls_static_size - _dl_tls_static_used - TLS_TCB_SIZE;

	blsize = map->l_tls_blocksize + map->l_tls_firstbyte_offset;
	if (freebytes < blsize)
		goto fail;

	n = (freebytes - blsize) / map->l_tls_align;

	size_t offset = _dl_tls_static_used + (freebytes - n * map->l_tls_align
		- map->l_tls_firstbyte_offset);

	map->l_tls_offset = _dl_tls_static_used = offset;
# elif defined(TLS_DTV_AT_TP)
	size_t used;
	size_t check;

	size_t offset = roundup (_dl_tls_static_used, map->l_tls_align);
	used = offset + map->l_tls_blocksize;
	check = used;

	/* dl_tls_static_used includes the TCB at the beginning. */
	if (check > _dl_tls_static_size)
		goto fail;

	map->l_tls_offset = offset;
	_dl_tls_static_used = used;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

	/*
	 * If the object is not yet relocated we cannot initialize the
	 * static TLS region.  Delay it.
	 */
	if (((struct elf_resolve *) map)->init_flag & RELOCS_DONE)
    {
#ifdef SHARED
		/*
		 * Update the slot information data for at least the generation of
		 * the DSO we are allocating data for.
		 */
		if (__builtin_expect (THREAD_DTV()[0].counter != _dl_tls_generation, 0))
			(void) _dl_update_slotinfo (map->l_tls_modid);
#endif
		_dl_init_static_tls (map);
	}
	else
		map->l_need_tls_init = 1;
}

size_t
internal_function
_dl_next_tls_modid (void)
{
  size_t result;

  if (__builtin_expect (GL(dl_tls_dtv_gaps), false))
    {
      size_t disp = 0;
      struct dtv_slotinfo_list *runp = GL(dl_tls_dtv_slotinfo_list);

      /* Note that this branch will never be executed during program
	 start since there are no gaps at that time.  Therefore it
	 does not matter that the dl_tls_dtv_slotinfo is not allocated
	 yet when the function is called for the first times.

	 NB: the offset +1 is due to the fact that DTV[0] is used
	 for something else.  */
      result = GL(dl_tls_static_nelem) + 1;
      if (result <= GL(dl_tls_max_dtv_idx))
	do
	  {
	    while (result - disp < runp->len)
	      {
		if (runp->slotinfo[result - disp].map == NULL)
		  break;

		++result;
		assert (result <= GL(dl_tls_max_dtv_idx) + 1);
	      }

	    if (result - disp < runp->len)
	      break;

	    disp += runp->len;
	  }
	while ((runp = runp->next) != NULL);

      if (result > GL(dl_tls_max_dtv_idx))
	{
	  /* The new index must indeed be exactly one higher than the
	     previous high.  */
	  assert (result == GL(dl_tls_max_dtv_idx) + 1);
	  /* There is no gap anymore.  */
	  GL(dl_tls_dtv_gaps) = false;

	  goto nogaps;
	}
    }
  else
    {
      /* No gaps, allocate a new entry.  */
    nogaps:

      result = ++GL(dl_tls_max_dtv_idx);
    }

  return result;
}


# ifdef SHARED
void
internal_function
_dl_determine_tlsoffset (void)
{
  size_t max_align = TLS_TCB_ALIGN;
  size_t freetop = 0;
  size_t freebottom = 0;

  /* The first element of the dtv slot info list is allocated.  */
  assert (GL(dl_tls_dtv_slotinfo_list) != NULL);
  /* There is at this point only one element in the
     dl_tls_dtv_slotinfo_list list.  */
  assert (GL(dl_tls_dtv_slotinfo_list)->next == NULL);

  struct dtv_slotinfo *slotinfo = GL(dl_tls_dtv_slotinfo_list)->slotinfo;

  /* Determining the offset of the various parts of the static TLS
     block has several dependencies.  In addition we have to work
     around bugs in some toolchains.

     Each TLS block from the objects available at link time has a size
     and an alignment requirement.  The GNU ld computes the alignment
     requirements for the data at the positions *in the file*, though.
     I.e, it is not simply possible to allocate a block with the size
     of the TLS program header entry.  The data is layed out assuming
     that the first byte of the TLS block fulfills

       p_vaddr mod p_align == &TLS_BLOCK mod p_align

     This means we have to add artificial padding at the beginning of
     the TLS block.  These bytes are never used for the TLS data in
     this module but the first byte allocated must be aligned
     according to mod p_align == 0 so that the first byte of the TLS
     block is aligned according to p_vaddr mod p_align.  This is ugly
     and the linker can help by computing the offsets in the TLS block
     assuming the first byte of the TLS block is aligned according to
     p_align.

     The extra space which might be allocated before the first byte of
     the TLS block need not go unused.  The code below tries to use
     that memory for the next TLS block.  This can work if the total
     memory requirement for the next TLS block is smaller than the
     gap.  */

# if defined(TLS_TCB_AT_TP)
  /* We simply start with zero.  */
  size_t offset = 0;

  size_t cnt;
  for (cnt = 0; slotinfo[cnt].map != NULL; ++cnt)
    {
      assert (cnt < GL(dl_tls_dtv_slotinfo_list)->len);

      size_t firstbyte = (-slotinfo[cnt].map->l_tls_firstbyte_offset
			  & (slotinfo[cnt].map->l_tls_align - 1));
      size_t off;
      max_align = MAX (max_align, slotinfo[cnt].map->l_tls_align);

      if (freebottom - freetop >= slotinfo[cnt].map->l_tls_blocksize)
	{
	  off = roundup (freetop + slotinfo[cnt].map->l_tls_blocksize
			 - firstbyte, slotinfo[cnt].map->l_tls_align)
		+ firstbyte;
	  if (off <= freebottom)
	    {
	      freetop = off;

	      /* XXX For some architectures we perhaps should store the
		 negative offset.  */
	      slotinfo[cnt].map->l_tls_offset = off;
	      continue;
	    }
	}

      off = roundup (offset + slotinfo[cnt].map->l_tls_blocksize - firstbyte,
		     slotinfo[cnt].map->l_tls_align) + firstbyte;
      if (off > offset + slotinfo[cnt].map->l_tls_blocksize
		+ (freebottom - freetop))
	{
	  freetop = offset;
	  freebottom = off - slotinfo[cnt].map->l_tls_blocksize;
	}
      offset = off;

      /* XXX For some architectures we perhaps should store the
	 negative offset.  */
      slotinfo[cnt].map->l_tls_offset = off;
    }

  GL(dl_tls_static_used) = offset;
  GL(dl_tls_static_size) = (roundup (offset + TLS_STATIC_SURPLUS, max_align)
			    + TLS_TCB_SIZE);
# elif defined(TLS_DTV_AT_TP)
  /* The TLS blocks start right after the TCB.  */
  size_t offset = TLS_TCB_SIZE;
  size_t cnt;

  for (cnt = 0; slotinfo[cnt].map != NULL; ++cnt)
    {
      assert (cnt < GL(dl_tls_dtv_slotinfo_list)->len);

      size_t firstbyte = (-slotinfo[cnt].map->l_tls_firstbyte_offset
			  & (slotinfo[cnt].map->l_tls_align - 1));
      size_t off;
      max_align = MAX (max_align, slotinfo[cnt].map->l_tls_align);

      if (slotinfo[cnt].map->l_tls_blocksize <= freetop - freebottom)
	{
	  off = roundup (freebottom, slotinfo[cnt].map->l_tls_align);
	  if (off - freebottom < firstbyte)
	    off += slotinfo[cnt].map->l_tls_align;
	  if (off + slotinfo[cnt].map->l_tls_blocksize - firstbyte <= freetop)
	    {
	      slotinfo[cnt].map->l_tls_offset = off - firstbyte;
	      freebottom = (off + slotinfo[cnt].map->l_tls_blocksize
			    - firstbyte);
	      continue;
	    }
	}

      off = roundup (offset, slotinfo[cnt].map->l_tls_align);
      if (off - offset < firstbyte)
	off += slotinfo[cnt].map->l_tls_align;

      slotinfo[cnt].map->l_tls_offset = off - firstbyte;
      if (off - firstbyte - offset > freetop - freebottom)
	{
	  freebottom = offset;
	  freetop = off - firstbyte;
	}

      offset = off + slotinfo[cnt].map->l_tls_blocksize - firstbyte;
    }

  GL(dl_tls_static_used) = offset;
  GL(dl_tls_static_size) = roundup (offset + TLS_STATIC_SURPLUS,
				    TLS_TCB_ALIGN);
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* The alignment requirement for the static TLS block.  */
  GL(dl_tls_static_align) = max_align;
}


/* This is called only when the data structure setup was skipped at startup,
   when there was no need for it then.  Now we have dynamically loaded
   something needing TLS, or libpthread needs it.  */
int
internal_function
_dl_tls_setup (void)
{
  assert (GL(dl_tls_dtv_slotinfo_list) == NULL);
  assert (GL(dl_tls_max_dtv_idx) == 0);

  const size_t nelem = 2 + TLS_SLOTINFO_SURPLUS;

  GL(dl_tls_dtv_slotinfo_list)
    = calloc (1, (sizeof (struct dtv_slotinfo_list)
		  + nelem * sizeof (struct dtv_slotinfo)));
  if (GL(dl_tls_dtv_slotinfo_list) == NULL)
    return -1;

  GL(dl_tls_dtv_slotinfo_list)->len = nelem;

  /* Number of elements in the static TLS block.  It can't be zero
     because of various assumptions.  The one element is null.  */
  GL(dl_tls_static_nelem) = GL(dl_tls_max_dtv_idx) = 1;

  /* This initializes more variables for us.  */
  _dl_determine_tlsoffset ();

  return 0;
}
# endif

static void *
internal_function
allocate_dtv (void *result)
{
  dtv_t *dtv;
  size_t dtv_length;

  /* We allocate a few more elements in the dtv than are needed for the
     initial set of modules.  This should avoid in most cases expansions
     of the dtv.  */
  dtv_length = GL(dl_tls_max_dtv_idx) + DTV_SURPLUS;
  dtv = calloc (dtv_length + 2, sizeof (dtv_t));
  if (dtv != NULL)
    {
      /* This is the initial length of the dtv.  */
      dtv[0].counter = dtv_length;

      /* The rest of the dtv (including the generation counter) is
	 Initialize with zero to indicate nothing there.  */

      /* Add the dtv to the thread data structures.  */
      INSTALL_DTV (result, dtv);
    }
  else
    result = NULL;

  return result;
}


/* Get size and alignment requirements of the static TLS block.  */
void
internal_function
_dl_get_tls_static_info (size_t *sizep, size_t *alignp)
{
  *sizep = GL(dl_tls_static_size);
  *alignp = GL(dl_tls_static_align);
}


void *
internal_function
_dl_allocate_tls_storage (void)
{
  void *result;
  size_t size = GL(dl_tls_static_size);

# if defined(TLS_DTV_AT_TP)
  /* Memory layout is:
     [ TLS_PRE_TCB_SIZE ] [ TLS_TCB_SIZE ] [ TLS blocks ]
			  ^ This should be returned.  */
  size += (TLS_PRE_TCB_SIZE + GL(dl_tls_static_align) - 1)
	  & ~(GL(dl_tls_static_align) - 1);
# endif

  /* Allocate a correctly aligned chunk of memory.  */
  result = _dl_memalign (GL(dl_tls_static_align), size);
  if (__builtin_expect (result != NULL, 1))
    {
      /* Allocate the DTV.  */
      void *allocated = result;

# if defined(TLS_TCB_AT_TP)
      /* The TCB follows the TLS blocks.  */
      result = (char *) result + size - TLS_TCB_SIZE;

      /* Clear the TCB data structure.  We can't ask the caller (i.e.
	 libpthread) to do it, because we will initialize the DTV et al.  */
      _dl_memset (result, '\0', TLS_TCB_SIZE);
# elif defined(TLS_DTV_AT_TP)
      result = (char *) result + size - GL(dl_tls_static_size);

      /* Clear the TCB data structure and TLS_PRE_TCB_SIZE bytes before it.
	 We can't ask the caller (i.e. libpthread) to do it, because we will
	 initialize the DTV et al.  */
      _dl_memset ((char *) result - TLS_PRE_TCB_SIZE, '\0',
	      TLS_PRE_TCB_SIZE + TLS_TCB_SIZE);
# endif

      result = allocate_dtv (result);
      if (result == NULL)
	free (allocated);
    }

  return result;
}


void *
internal_function
_dl_allocate_tls_init (void *result)
{
  if (result == NULL)
    /* The memory allocation failed.  */
    return NULL;

  dtv_t *dtv = GET_DTV (result);
  struct dtv_slotinfo_list *listp;
  size_t total = 0;
  size_t maxgen = 0;

  /* We have to prepare the dtv for all currently loaded modules using
     TLS.  For those which are dynamically loaded we add the values
     indicating deferred allocation.  */
  listp = GL(dl_tls_dtv_slotinfo_list);
  while (1)
    {
      size_t cnt;

      for (cnt = total == 0 ? 1 : 0; cnt < listp->len; ++cnt)
	{
	  struct link_map *map;
	  void *dest;

	  /* Check for the total number of used slots.  */
	  if (total + cnt > GL(dl_tls_max_dtv_idx))
	    break;

	  map = listp->slotinfo[cnt].map;
	  if (map == NULL)
	    /* Unused entry.  */
	    continue;

	  /* Keep track of the maximum generation number.  This might
	     not be the generation counter.  */
	  maxgen = MAX (maxgen, listp->slotinfo[cnt].gen);

	  if (map->l_tls_offset == NO_TLS_OFFSET)
	    {
	      /* For dynamically loaded modules we simply store
		 the value indicating deferred allocation.  */
	      dtv[map->l_tls_modid].pointer.val = TLS_DTV_UNALLOCATED;
	      dtv[map->l_tls_modid].pointer.is_static = false;
	      continue;
	    }

	  assert (map->l_tls_modid == cnt);
	  assert (map->l_tls_blocksize >= map->l_tls_initimage_size);
# if defined(TLS_TCB_AT_TP)
	  assert ((size_t) map->l_tls_offset >= map->l_tls_blocksize);
	  dest = (char *) result - map->l_tls_offset;
# elif defined(TLS_DTV_AT_TP)
	  dest = (char *) result + map->l_tls_offset;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

	  /* Copy the initialization image and clear the BSS part.  */
	  dtv[map->l_tls_modid].pointer.val = dest;
	  dtv[map->l_tls_modid].pointer.is_static = true;
	  _dl_memset (_dl_mempcpy (dest, map->l_tls_initimage,
			     map->l_tls_initimage_size), '\0',
		  map->l_tls_blocksize - map->l_tls_initimage_size);
	}

      total += cnt;
      if (total >= GL(dl_tls_max_dtv_idx))
	break;

      listp = listp->next;
      assert (listp != NULL);
    }

  /* The DTV version is up-to-date now.  */
  dtv[0].counter = maxgen;

  return result;
}

void *
internal_function
_dl_allocate_tls (void *mem)
{
  return _dl_allocate_tls_init (mem == NULL
				? _dl_allocate_tls_storage ()
				: allocate_dtv (mem));
}


void
internal_function
_dl_deallocate_tls (void *tcb, bool dealloc_tcb)
{
  dtv_t *dtv = GET_DTV (tcb);
  size_t cnt;

  /* We need to free the memory allocated for non-static TLS.  */
  for (cnt = 0; cnt < dtv[-1].counter; ++cnt)
    if (! dtv[1 + cnt].pointer.is_static
	&& dtv[1 + cnt].pointer.val != TLS_DTV_UNALLOCATED)
      free (dtv[1 + cnt].pointer.val);

  /* The array starts with dtv[-1].  */
#ifdef SHARED
  if (dtv != GL(dl_initial_dtv))
#else
  if ((dtv - 1) != &static_dtv)
#endif
    free (dtv - 1);

  if (dealloc_tcb)
    {
# if defined(TLS_TCB_AT_TP)
      /* The TCB follows the TLS blocks.  Back up to free the whole block.  */
      tcb -= GL(dl_tls_static_size) - TLS_TCB_SIZE;
# elif defined(TLS_DTV_AT_TP)
      /* Back up the TLS_PRE_TCB_SIZE bytes.  */
      tcb -= (TLS_PRE_TCB_SIZE + GL(dl_tls_static_align) - 1)
	     & ~(GL(dl_tls_static_align) - 1);
# endif
      free (tcb);
    }
}


# ifdef SHARED
/* The __tls_get_addr function has two basic forms which differ in the
   arguments.  The IA-64 form takes two parameters, the module ID and
   offset.  The form used, among others, on IA-32 takes a reference to
   a special structure which contain the same information.  The second
   form seems to be more often used (in the moment) so we default to
   it.  Users of the IA-64 form have to provide adequate definitions
   of the following macros.  */
#  ifndef GET_ADDR_ARGS
#   define GET_ADDR_ARGS tls_index *ti
#  endif
#  ifndef GET_ADDR_MODULE
#   define GET_ADDR_MODULE ti->ti_module
#  endif
#  ifndef GET_ADDR_OFFSET
#   define GET_ADDR_OFFSET ti->ti_offset
#  endif


static void *
allocate_and_init (struct link_map *map)
{
  void *newp;

  newp = _dl_memalign (map->l_tls_align, map->l_tls_blocksize);
  if (newp == NULL)
    oom ();

  /* Initialize the memory.  */
  _dl_memset (_dl_mempcpy (newp, map->l_tls_initimage, map->l_tls_initimage_size),
	  '\0', map->l_tls_blocksize - map->l_tls_initimage_size);

  return newp;
}


struct link_map *
_dl_update_slotinfo (unsigned long int req_modid)
{
  struct link_map *the_map = NULL;
  dtv_t *dtv = THREAD_DTV ();

  /* The global dl_tls_dtv_slotinfo array contains for each module
     index the generation counter current when the entry was created.
     This array never shrinks so that all module indices which were
     valid at some time can be used to access it.  Before the first
     use of a new module index in this function the array was extended
     appropriately.  Access also does not have to be guarded against
     modifications of the array.  It is assumed that pointer-size
     values can be read atomically even in SMP environments.  It is
     possible that other threads at the same time dynamically load
     code and therefore add to the slotinfo list.  This is a problem
     since we must not pick up any information about incomplete work.
     The solution to this is to ignore all dtv slots which were
     created after the one we are currently interested.  We know that
     dynamic loading for this module is completed and this is the last
     load operation we know finished.  */
  unsigned long int idx = req_modid;
  struct dtv_slotinfo_list *listp = GL(dl_tls_dtv_slotinfo_list);

  while (idx >= listp->len)
    {
      idx -= listp->len;
      listp = listp->next;
    }

  if (dtv[0].counter < listp->slotinfo[idx].gen)
    {
      /* The generation counter for the slot is higher than what the
	 current dtv implements.  We have to update the whole dtv but
	 only those entries with a generation counter <= the one for
	 the entry we need.  */
      size_t new_gen = listp->slotinfo[idx].gen;
      size_t total = 0;

      /* We have to look through the entire dtv slotinfo list.  */
      listp =  GL(dl_tls_dtv_slotinfo_list);
      do
	{
	  size_t cnt;

	  for (cnt = total == 0 ? 1 : 0; cnt < listp->len; ++cnt)
	    {
	      size_t gen = listp->slotinfo[cnt].gen;

	      if (gen > new_gen)
		/* This is a slot for a generation younger than the
		   one we are handling now.  It might be incompletely
		   set up so ignore it.  */
		continue;

	      /* If the entry is older than the current dtv layout we
		 know we don't have to handle it.  */
	      if (gen <= dtv[0].counter)
		continue;

	      /* If there is no map this means the entry is empty.  */
	      struct link_map *map = listp->slotinfo[cnt].map;
	      if (map == NULL)
		{
		  /* If this modid was used at some point the memory
		     might still be allocated.  */
		  if (! dtv[total + cnt].pointer.is_static
		      && dtv[total + cnt].pointer.val != TLS_DTV_UNALLOCATED)
		    {
		      free (dtv[total + cnt].pointer.val);
		      dtv[total + cnt].pointer.val = TLS_DTV_UNALLOCATED;
		    }

		  continue;
		}

	      /* Check whether the current dtv array is large enough.  */
	      size_t modid = map->l_tls_modid;
	      assert (total + cnt == modid);
	      if (dtv[-1].counter < modid)
		{
		  /* Reallocate the dtv.  */
		  dtv_t *newp;
		  size_t newsize = GL(dl_tls_max_dtv_idx) + DTV_SURPLUS;
		  size_t oldsize = dtv[-1].counter;

		  assert (map->l_tls_modid <= newsize);

		  if (dtv == GL(dl_initial_dtv))
		    {
		      /* This is the initial dtv that was allocated
			 during rtld startup using the dl-minimal.c
			 malloc instead of the real malloc.  We can't
			 free it, we have to abandon the old storage.  */

		      newp = malloc ((2 + newsize) * sizeof (dtv_t));
		      if (newp == NULL)
			oom ();
		      _dl_memcpy (newp, &dtv[-1], oldsize * sizeof (dtv_t));
		    }
		  else
		    {
		      newp = realloc (&dtv[-1],
				      (2 + newsize) * sizeof (dtv_t));
		      if (newp == NULL)
			oom ();
		    }

		  newp[0].counter = newsize;

		  /* Clear the newly allocated part.  */
		  _dl_memset (newp + 2 + oldsize, '\0',
			  (newsize - oldsize) * sizeof (dtv_t));

		  /* Point dtv to the generation counter.  */
		  dtv = &newp[1];

		  /* Install this new dtv in the thread data
		     structures.  */
		  INSTALL_NEW_DTV (dtv);
		}

	      /* If there is currently memory allocate for this
		 dtv entry free it.  */
	      /* XXX Ideally we will at some point create a memory
		 pool.  */
	      if (! dtv[modid].pointer.is_static
		  && dtv[modid].pointer.val != TLS_DTV_UNALLOCATED)
		/* Note that free is called for NULL is well.  We
		   deallocate even if it is this dtv entry we are
		   supposed to load.  The reason is that we call
		   memalign and not malloc.  */
		free (dtv[modid].pointer.val);

	      /* This module is loaded dynamically- We defer memory
		 allocation.  */
	      dtv[modid].pointer.is_static = false;
	      dtv[modid].pointer.val = TLS_DTV_UNALLOCATED;

	      if (modid == req_modid)
		the_map = map;
	    }

	  total += listp->len;
	}
      while ((listp = listp->next) != NULL);

      /* This will be the new maximum generation counter.  */
      dtv[0].counter = new_gen;
    }

  return the_map;
}


/* The generic dynamic and local dynamic model cannot be used in
   statically linked applications.  */
void *
__tls_get_addr (GET_ADDR_ARGS)
{
  dtv_t *dtv = THREAD_DTV ();
  struct link_map *the_map = NULL;
  void *p;

  if (__builtin_expect (dtv[0].counter != GL(dl_tls_generation), 0))
    the_map = _dl_update_slotinfo (GET_ADDR_MODULE);

  p = dtv[GET_ADDR_MODULE].pointer.val;

  if (__builtin_expect (p == TLS_DTV_UNALLOCATED, 0))
    {
      /* The allocation was deferred.  Do it now.  */
      if (the_map == NULL)
	{
	  /* Find the link map for this module.  */
	  size_t idx = GET_ADDR_MODULE;
	  struct dtv_slotinfo_list *listp = GL(dl_tls_dtv_slotinfo_list);

	  while (idx >= listp->len)
	    {
	      idx -= listp->len;
	      listp = listp->next;
	    }

	  the_map = listp->slotinfo[idx].map;
	}

      p = dtv[GET_ADDR_MODULE].pointer.val = allocate_and_init (the_map);
      dtv[GET_ADDR_MODULE].pointer.is_static = false;
    }

  return (char *) p + GET_ADDR_OFFSET;
}
# endif



void _dl_add_to_slotinfo (struct link_map  *l);
void
_dl_add_to_slotinfo (struct link_map  *l)
{
  /* Now that we know the object is loaded successfully add
     modules containing TLS data to the dtv info table.  We
     might have to increase its size.  */
  struct dtv_slotinfo_list *listp;
  struct dtv_slotinfo_list *prevp;
  size_t idx = l->l_tls_modid;

  /* Find the place in the dtv slotinfo list.  */
  listp = GL(dl_tls_dtv_slotinfo_list);
  prevp = NULL;		/* Needed to shut up gcc.  */
  do
    {
      /* Does it fit in the array of this list element?  */
      if (idx < listp->len)
	break;
      idx -= listp->len;
      prevp = listp;
      listp = listp->next;
    }
  while (listp != NULL);

  if (listp == NULL)
    {
      /* When we come here it means we have to add a new element
	 to the slotinfo list.  And the new module must be in
	 the first slot.  */
      assert (idx == 0);

      listp = prevp->next = (struct dtv_slotinfo_list *)
	malloc (sizeof (struct dtv_slotinfo_list)
		+ TLS_SLOTINFO_SURPLUS * sizeof (struct dtv_slotinfo));
      if (listp == NULL)
	{
	  /* We ran out of memory.  We will simply fail this
	     call but don't undo anything we did so far.  The
	     application will crash or be terminated anyway very
	     soon.  */

	  /* We have to do this since some entries in the dtv
	     slotinfo array might already point to this
	     generation.  */
	  ++GL(dl_tls_generation);

	  _dl_dprintf (_dl_debug_file,
			"cannot create TLS data structures: ABORT\n");
	  _dl_exit (127);
	}

      listp->len = TLS_SLOTINFO_SURPLUS;
      listp->next = NULL;
      _dl_memset (listp->slotinfo, '\0',
	      TLS_SLOTINFO_SURPLUS * sizeof (struct dtv_slotinfo));
    }

  /* Add the information into the slotinfo data structure.  */
  listp->slotinfo[idx].map = l;
  listp->slotinfo[idx].gen = GL(dl_tls_generation) + 1;
}
