/* Copyright (C) 2002-2007, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <tls.h>
#include <lowlevellock.h>
#include <link.h>
#include <bits/kernel-features.h>


#ifndef NEED_SEPARATE_REGISTER_STACK

/* Most architectures have exactly one stack pointer.  Some have more.  */
# define STACK_VARIABLES void *stackaddr = NULL

/* How to pass the values to the 'create_thread' function.  */
# define STACK_VARIABLES_ARGS stackaddr

/* How to declare function which gets there parameters.  */
# define STACK_VARIABLES_PARMS void *stackaddr

/* How to declare allocate_stack.  */
# define ALLOCATE_STACK_PARMS void **stack

/* This is how the function is called.  We do it this way to allow
   other variants of the function to have more parameters.  */
# define ALLOCATE_STACK(attr, pd) allocate_stack (attr, pd, &stackaddr)

#else

/* We need two stacks.  The kernel will place them but we have to tell
   the kernel about the size of the reserved address space.  */
# define STACK_VARIABLES void *stackaddr = NULL; size_t stacksize = 0

/* How to pass the values to the 'create_thread' function.  */
# define STACK_VARIABLES_ARGS stackaddr, stacksize

/* How to declare function which gets there parameters.  */
# define STACK_VARIABLES_PARMS void *stackaddr, size_t stacksize

/* How to declare allocate_stack.  */
# define ALLOCATE_STACK_PARMS void **stack, size_t *stacksize

/* This is how the function is called.  We do it this way to allow
   other variants of the function to have more parameters.  */
# define ALLOCATE_STACK(attr, pd) \
  allocate_stack (attr, pd, &stackaddr, &stacksize)

#endif


/* Default alignment of stack.  */
#ifndef STACK_ALIGN
# define STACK_ALIGN __alignof__ (long double)
#endif

/* Default value for minimal stack size after allocating thread
   descriptor and guard.  */
#ifndef MINIMAL_REST_STACK
# define MINIMAL_REST_STACK	4096
#endif


/* Newer kernels have the MAP_STACK flag to indicate a mapping is used for
   a stack.  Use it when possible.  */
#ifndef MAP_STACK
# define MAP_STACK 0
#endif

/* This yields the pointer that TLS support code calls the thread pointer.  */
#if defined(TLS_TCB_AT_TP)
# define TLS_TPADJ(pd) (pd)
#elif defined(TLS_DTV_AT_TP)
# define TLS_TPADJ(pd) ((struct pthread *)((char *) (pd) + TLS_PRE_TCB_SIZE))
#endif

/* Cache handling for not-yet free stacks.  */

/*
   Maximum size in kB of cache. GNU libc default is 40MiB
   embedded systems don't have enough ram for big dirty stack caches,
   reduce it to 16MiB. 4 does not work, f.e. tst-kill4 segfaults.
*/
static size_t stack_cache_maxsize = 16 * 1024 * 1024;
static size_t stack_cache_actsize;

/* Mutex protecting this variable.  */
static int stack_cache_lock = LLL_LOCK_INITIALIZER;

/* List of queued stack frames.  */
static LIST_HEAD (stack_cache);

/* List of the stacks in use.  */
static LIST_HEAD (stack_used);

/* We need to record what list operations we are going to do so that,
   in case of an asynchronous interruption due to a fork() call, we
   can correct for the work.  */
static uintptr_t in_flight_stack;

/* List of the threads with user provided stacks in use.  No need to
   initialize this, since it's done in __pthread_initialize_minimal.  */
list_t __stack_user __attribute__ ((nocommon));
hidden_data_def (__stack_user)

#if defined COLORING_INCREMENT && COLORING_INCREMENT != 0
/* Number of threads created.  */
static unsigned int nptl_ncreated;
#endif


/* Check whether the stack is still used or not.  */
#define FREE_P(descr) ((descr)->tid <= 0)


static void
stack_list_del (list_t *elem)
{
  in_flight_stack = (uintptr_t) elem;

  atomic_write_barrier ();

  list_del (elem);

  atomic_write_barrier ();

  in_flight_stack = 0;
}


static void
stack_list_add (list_t *elem, list_t *list)
{
  in_flight_stack = (uintptr_t) elem | 1;

  atomic_write_barrier ();

  list_add (elem, list);

  atomic_write_barrier ();

  in_flight_stack = 0;
}


/* We create a double linked list of all cache entries.  Double linked
   because this allows removing entries from the end.  */


/* Get a stack frame from the cache.  We have to match by size since
   some blocks might be too small or far too large.  */
static struct pthread *
get_cached_stack (size_t *sizep, void **memp)
{
  size_t size = *sizep;
  struct pthread *result = NULL;
  list_t *entry;

  lll_lock (stack_cache_lock, LLL_PRIVATE);

  /* Search the cache for a matching entry.  We search for the
     smallest stack which has at least the required size.  Note that
     in normal situations the size of all allocated stacks is the
     same.  As the very least there are only a few different sizes.
     Therefore this loop will exit early most of the time with an
     exact match.  */
  list_for_each (entry, &stack_cache)
    {
      struct pthread *curr;

      curr = list_entry (entry, struct pthread, list);
      if (FREE_P (curr) && curr->stackblock_size >= size)
	{
	  if (curr->stackblock_size == size)
	    {
	      result = curr;
	      break;
	    }

	  if (result == NULL
	      || result->stackblock_size > curr->stackblock_size)
	    result = curr;
	}
    }

  if (__builtin_expect (result == NULL, 0)
      /* Make sure the size difference is not too excessive.  In that
	 case we do not use the block.  */
      || __builtin_expect (result->stackblock_size > 4 * size, 0))
    {
      /* Release the lock.  */
      lll_unlock (stack_cache_lock, LLL_PRIVATE);

      return NULL;
    }

  /* Dequeue the entry.  */
  stack_list_del (&result->list);

  /* And add to the list of stacks in use.  */
  stack_list_add (&result->list, &stack_used);

  /* And decrease the cache size.  */
  stack_cache_actsize -= result->stackblock_size;

  /* Release the lock early.  */
  lll_unlock (stack_cache_lock, LLL_PRIVATE);

  /* Report size and location of the stack to the caller.  */
  *sizep = result->stackblock_size;
  *memp = result->stackblock;

  /* Cancellation handling is back to the default.  */
  result->cancelhandling = 0;
  result->cleanup = NULL;

  /* No pending event.  */
  result->nextevent = NULL;

  /* Clear the DTV.  */
  dtv_t *dtv = GET_DTV (TLS_TPADJ (result));
  memset (dtv, '\0', (dtv[-1].counter + 1) * sizeof (dtv_t));

  /* Re-initialize the TLS.  */
  _dl_allocate_tls_init (TLS_TPADJ (result));

  return result;
}


/* Free stacks until cache size is lower than LIMIT.  */
void
__free_stacks (size_t limit)
{
  /* We reduce the size of the cache.  Remove the last entries until
     the size is below the limit.  */
  list_t *entry;
  list_t *prev;

  /* Search from the end of the list.  */
  list_for_each_prev_safe (entry, prev, &stack_cache)
    {
      struct pthread *curr;

      curr = list_entry (entry, struct pthread, list);
      if (FREE_P (curr))
	{
	  /* Unlink the block.  */
	  stack_list_del (entry);

	  /* Account for the freed memory.  */
	  stack_cache_actsize -= curr->stackblock_size;

	  /* Free the memory associated with the ELF TLS.  */
	  _dl_deallocate_tls (TLS_TPADJ (curr), false);

	  /* Remove this block.  This should never fail.  If it does
	     something is really wrong.  */
	  if (munmap (curr->stackblock, curr->stackblock_size) != 0)
	    abort ();

	  /* Maybe we have freed enough.  */
	  if (stack_cache_actsize <= limit)
	    break;
	}
    }
}


/* Add a stack frame which is not used anymore to the stack.  Must be
   called with the cache lock held.  */
static inline void
__attribute ((always_inline))
queue_stack (struct pthread *stack)
{
  /* We unconditionally add the stack to the list.  The memory may
     still be in use but it will not be reused until the kernel marks
     the stack as not used anymore.  */
  stack_list_add (&stack->list, &stack_cache);

  stack_cache_actsize += stack->stackblock_size;
  if (__builtin_expect (stack_cache_actsize > stack_cache_maxsize, 0))
    __free_stacks (stack_cache_maxsize);
}


static int
internal_function
change_stack_perm (struct pthread *pd
#ifdef NEED_SEPARATE_REGISTER_STACK
		   , size_t pagemask
#endif
		   )
{
#ifdef NEED_SEPARATE_REGISTER_STACK
  void *stack = (pd->stackblock
		 + (((((pd->stackblock_size - pd->guardsize) / 2)
		      & pagemask) + pd->guardsize) & pagemask));
  size_t len = pd->stackblock + pd->stackblock_size - stack;
#elif defined _STACK_GROWS_DOWN
  void *stack = pd->stackblock + pd->guardsize;
  size_t len = pd->stackblock_size - pd->guardsize;
#elif defined _STACK_GROWS_UP
  void *stack = pd->stackblock;
  size_t len = (uintptr_t) pd - pd->guardsize - (uintptr_t) pd->stackblock;
#else
# error "Define either _STACK_GROWS_DOWN or _STACK_GROWS_UP"
#endif
#ifdef __ARCH_USE_MMU__
  if (mprotect (stack, len, PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
    return errno;
#endif

  return 0;
}


static int
allocate_stack (const struct pthread_attr *attr, struct pthread **pdp,
		ALLOCATE_STACK_PARMS)
{
  struct pthread *pd;
  size_t size;
  size_t pagesize_m1 = __getpagesize () - 1;
  void *stacktop;

  assert (attr != NULL);
  assert (powerof2 (pagesize_m1 + 1));
  assert (TCB_ALIGNMENT >= STACK_ALIGN);

  /* Get the stack size from the attribute if it is set.  Otherwise we
     use the default we determined at start time.  */
  size = attr->stacksize ?: __default_stacksize;

  /* Get memory for the stack.  */
  if (__builtin_expect (attr->flags & ATTR_FLAG_STACKADDR, 0))
    {
      uintptr_t adj;

      /* If the user also specified the size of the stack make sure it
	 is large enough.  */
      if (attr->stacksize != 0
	  && attr->stacksize < (__static_tls_size + MINIMAL_REST_STACK))
	return EINVAL;

      /* Adjust stack size for alignment of the TLS block.  */
#if defined(TLS_TCB_AT_TP)
      adj = ((uintptr_t) attr->stackaddr - TLS_TCB_SIZE)
	    & __static_tls_align_m1;
      assert (size > adj + TLS_TCB_SIZE);
#elif defined(TLS_DTV_AT_TP)
      adj = ((uintptr_t) attr->stackaddr - __static_tls_size)
	    & __static_tls_align_m1;
      assert (size > adj);
#endif

      /* The user provided some memory.  Let's hope it matches the
	 size...  We do not allocate guard pages if the user provided
	 the stack.  It is the user's responsibility to do this if it
	 is wanted.  */
#if defined(TLS_TCB_AT_TP)
      pd = (struct pthread *) ((uintptr_t) attr->stackaddr
			       - TLS_TCB_SIZE - adj);
#elif defined(TLS_DTV_AT_TP)
      pd = (struct pthread *) (((uintptr_t) attr->stackaddr
			        - __static_tls_size - adj)
			       - TLS_PRE_TCB_SIZE);
#endif

      /* The user provided stack memory needs to be cleared.  */
      memset (pd, '\0', sizeof (struct pthread));

      /* The first TSD block is included in the TCB.  */
      pd->specific[0] = pd->specific_1stblock;

      /* Remember the stack-related values.  */
      pd->stackblock = (char *) attr->stackaddr - size;
      pd->stackblock_size = size;

      /* This is a user-provided stack.  It will not be queued in the
	 stack cache nor will the memory (except the TLS memory) be freed.  */
      pd->user_stack = true;

      /* This is at least the second thread.  */
      pd->header.multiple_threads = 1;
#ifndef TLS_MULTIPLE_THREADS_IN_TCB
      __pthread_multiple_threads = *__libc_multiple_threads_ptr = 1;
#endif

#ifndef __ASSUME_PRIVATE_FUTEX
      /* The thread must know when private futexes are supported.  */
      pd->header.private_futex = THREAD_GETMEM (THREAD_SELF,
						header.private_futex);
#endif

#ifdef NEED_DL_SYSINFO
      /* Copy the sysinfo value from the parent.  */
      THREAD_SYSINFO(pd) = THREAD_SELF_SYSINFO;
#endif

      /* Allocate the DTV for this thread.  */
      if (_dl_allocate_tls (TLS_TPADJ (pd)) == NULL)
	{
	  /* Something went wrong.  */
	  assert (errno == ENOMEM);
	  return EAGAIN;
	}


      /* Prepare to modify global data.  */
      lll_lock (stack_cache_lock, LLL_PRIVATE);

      /* And add to the list of stacks in use.  */
      list_add (&pd->list, &__stack_user);

      lll_unlock (stack_cache_lock, LLL_PRIVATE);
    }
  else
    {
      /* Allocate some anonymous memory.  If possible use the cache.  */
      size_t guardsize;
      size_t reqsize;
      void *mem = 0;
      const int prot = (PROT_READ | PROT_WRITE);

#if defined COLORING_INCREMENT && COLORING_INCREMENT != 0
      /* Add one more page for stack coloring.  Don't do it for stacks
	 with 16 times pagesize or larger.  This might just cause
	 unnecessary misalignment.  */
      if (size <= 16 * pagesize_m1)
	size += pagesize_m1 + 1;
#endif

      /* Adjust the stack size for alignment.  */
      size &= ~__static_tls_align_m1;
      assert (size != 0);

      /* Make sure the size of the stack is enough for the guard and
	 eventually the thread descriptor.  */
      guardsize = (attr->guardsize + pagesize_m1) & ~pagesize_m1;
      if (__builtin_expect (size < ((guardsize + __static_tls_size
				     + MINIMAL_REST_STACK + pagesize_m1)
				    & ~pagesize_m1),
			    0))
	/* The stack is too small (or the guard too large).  */
	return EINVAL;

      /* Try to get a stack from the cache.  */
      reqsize = size;
      pd = get_cached_stack (&size, &mem);
      if (pd == NULL)
	{
	  /* To avoid aliasing effects on a larger scale than pages we
	     adjust the allocated stack size if necessary.  This way
	     allocations directly following each other will not have
	     aliasing problems.  */
#if defined MULTI_PAGE_ALIASING && MULTI_PAGE_ALIASING != 0
	  if ((size % MULTI_PAGE_ALIASING) == 0)
	    size += pagesize_m1 + 1;
#endif

	  mem = mmap (NULL, size, prot,
		      MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

	  if (__builtin_expect (mem == MAP_FAILED, 0))
	    {
	      if (errno == ENOMEM)
		__set_errno (EAGAIN);

	       return errno;
	    }

	  /* SIZE is guaranteed to be greater than zero.
	     So we can never get a null pointer back from mmap.  */
	  assert (mem != NULL);

#if defined COLORING_INCREMENT && COLORING_INCREMENT != 0
	  /* Atomically increment NCREATED.  */
	  unsigned int ncreated = atomic_increment_val (&nptl_ncreated);

	  /* We chose the offset for coloring by incrementing it for
	     every new thread by a fixed amount.  The offset used
	     module the page size.  Even if coloring would be better
	     relative to higher alignment values it makes no sense to
	     do it since the mmap() interface does not allow us to
	     specify any alignment for the returned memory block.  */
	  size_t coloring = (ncreated * COLORING_INCREMENT) & pagesize_m1;

	  /* Make sure the coloring offsets does not disturb the alignment
	     of the TCB and static TLS block.  */
	  if (__builtin_expect ((coloring & __static_tls_align_m1) != 0, 0))
	    coloring = (((coloring + __static_tls_align_m1)
			 & ~(__static_tls_align_m1))
			& ~pagesize_m1);
#else
	  /* Unless specified we do not make any adjustments.  */
# define coloring 0
#endif

	  /* Place the thread descriptor at the end of the stack.  */
#if defined(TLS_TCB_AT_TP)
	  pd = (struct pthread *) ((char *) mem + size - coloring) - 1;
#elif defined(TLS_DTV_AT_TP)
	  pd = (struct pthread *) ((((uintptr_t) mem + size - coloring
				    - __static_tls_size)
				    & ~__static_tls_align_m1)
				   - TLS_PRE_TCB_SIZE);
#endif

	  /* Remember the stack-related values.  */
	  pd->stackblock = mem;
	  pd->stackblock_size = size;

	  /* We allocated the first block thread-specific data array.
	     This address will not change for the lifetime of this
	     descriptor.  */
	  pd->specific[0] = pd->specific_1stblock;

	  /* This is at least the second thread.  */
	  pd->header.multiple_threads = 1;
#ifndef TLS_MULTIPLE_THREADS_IN_TCB
	  __pthread_multiple_threads = *__libc_multiple_threads_ptr = 1;
#endif

#ifndef __ASSUME_PRIVATE_FUTEX
	  /* The thread must know when private futexes are supported.  */
	  pd->header.private_futex = THREAD_GETMEM (THREAD_SELF,
                                                    header.private_futex);
#endif

#ifdef NEED_DL_SYSINFO
	  /* Copy the sysinfo value from the parent.  */
	  THREAD_SYSINFO(pd) = THREAD_SELF_SYSINFO;
#endif

	  /* Allocate the DTV for this thread.  */
	  if (_dl_allocate_tls (TLS_TPADJ (pd)) == NULL)
	    {
	      /* Something went wrong.  */
	      assert (errno == ENOMEM);

	      /* Free the stack memory we just allocated.  */
	      (void) munmap (mem, size);

	      return EAGAIN;
	    }


	  /* Prepare to modify global data.  */
	  lll_lock (stack_cache_lock, LLL_PRIVATE);

	  /* And add to the list of stacks in use.  */
	  stack_list_add (&pd->list, &stack_used);

	  lll_unlock (stack_cache_lock, LLL_PRIVATE);


	  /* Note that all of the stack and the thread descriptor is
	     zeroed.  This means we do not have to initialize fields
	     with initial value zero.  This is specifically true for
	     the 'tid' field which is always set back to zero once the
	     stack is not used anymore and for the 'guardsize' field
	     which will be read next.  */
	}

      /* Create or resize the guard area if necessary.  */
      if (__builtin_expect (guardsize > pd->guardsize, 0))
	{
#ifdef NEED_SEPARATE_REGISTER_STACK
	  char *guard = mem + (((size - guardsize) / 2) & ~pagesize_m1);
#elif defined _STACK_GROWS_DOWN
	  char *guard = mem;
#elif defined _STACK_GROWS_UP
	  char *guard = (char *) (((uintptr_t) pd - guardsize) & ~pagesize_m1);
#endif
#ifdef __ARCH_USE_MMU__
	  if (mprotect (guard, guardsize, PROT_NONE) != 0)
	    {
	      int err;
	    mprot_error:
	      err = errno;

	      lll_lock (stack_cache_lock, LLL_PRIVATE);

	      /* Remove the thread from the list.  */
	      stack_list_del (&pd->list);

	      lll_unlock (stack_cache_lock, LLL_PRIVATE);

	      /* Get rid of the TLS block we allocated.  */
	      _dl_deallocate_tls (TLS_TPADJ (pd), false);

	      /* Free the stack memory regardless of whether the size
		 of the cache is over the limit or not.  If this piece
		 of memory caused problems we better do not use it
		 anymore.  Uh, and we ignore possible errors.  There
		 is nothing we could do.  */
	      (void) munmap (mem, size);

	      return err;
	    }
#endif

	  pd->guardsize = guardsize;
	}
      else if (__builtin_expect (pd->guardsize - guardsize > size - reqsize,
				 0))
	{
	  /* The old guard area is too large.  */

#ifdef NEED_SEPARATE_REGISTER_STACK
	  char *guard = mem + (((size - guardsize) / 2) & ~pagesize_m1);
	  char *oldguard = mem + (((size - pd->guardsize) / 2) & ~pagesize_m1);

#ifdef __ARCH_USE_MMU__
	  if (oldguard < guard
	      && mprotect (oldguard, guard - oldguard, prot) != 0)
	    goto mprot_error;

	  if (mprotect (guard + guardsize,
			oldguard + pd->guardsize - guard - guardsize,
			prot) != 0)
	    goto mprot_error;
#elif defined _STACK_GROWS_DOWN
	  if (mprotect ((char *) mem + guardsize, pd->guardsize - guardsize,
			prot) != 0)
	    goto mprot_error;
#elif defined _STACK_GROWS_UP
	  if (mprotect ((char *) (((uintptr_t) pd - pd->guardsize) & ~pagesize_m1),
			pd->guardsize - guardsize, prot) != 0)
	    goto mprot_error;
#endif
#endif

	  pd->guardsize = guardsize;
	}
      /* The pthread_getattr_np() calls need to get passed the size
	 requested in the attribute, regardless of how large the
	 actually used guardsize is.  */
      pd->reported_guardsize = guardsize;
    }

  /* Initialize the lock.  We have to do this unconditionally since the
     stillborn thread could be canceled while the lock is taken.  */
  pd->lock = LLL_LOCK_INITIALIZER;

  /* The robust mutex lists also need to be initialized
     unconditionally because the cleanup for the previous stack owner
     might have happened in the kernel.  */
  pd->robust_head.futex_offset = (offsetof (pthread_mutex_t, __data.__lock)
				  - offsetof (pthread_mutex_t,
					      __data.__list.__next));
  pd->robust_head.list_op_pending = NULL;
#ifdef __PTHREAD_MUTEX_HAVE_PREV
  pd->robust_prev = &pd->robust_head;
#endif
  pd->robust_head.list = &pd->robust_head;

  /* We place the thread descriptor at the end of the stack.  */
  *pdp = pd;

#if defined(TLS_TCB_AT_TP)
  /* The stack begins before the TCB and the static TLS block.  */
  stacktop = ((char *) (pd + 1) - __static_tls_size);
#elif defined(TLS_DTV_AT_TP)
  stacktop = (char *) (pd - 1);
#endif

#ifdef NEED_SEPARATE_REGISTER_STACK
  *stack = pd->stackblock;
  *stacksize = stacktop - *stack;
#elif defined _STACK_GROWS_DOWN
  *stack = stacktop;
#elif defined _STACK_GROWS_UP
  *stack = pd->stackblock;
  assert (*stack > 0);
#endif

  return 0;
}


void
internal_function
__deallocate_stack (struct pthread *pd)
{
  lll_lock (stack_cache_lock, LLL_PRIVATE);

  /* Remove the thread from the list of threads with user defined
     stacks.  */
  stack_list_del (&pd->list);

  /* Not much to do.  Just free the mmap()ed memory.  Note that we do
     not reset the 'used' flag in the 'tid' field.  This is done by
     the kernel.  If no thread has been created yet this field is
     still zero.  */
  if (__builtin_expect (! pd->user_stack, 1))
    (void) queue_stack (pd);
  else
    /* Free the memory associated with the ELF TLS.  */
    _dl_deallocate_tls (TLS_TPADJ (pd), false);

  lll_unlock (stack_cache_lock, LLL_PRIVATE);
}


int
internal_function
__make_stacks_executable (void **stack_endp)
{
  /* First the main thread's stack.  */
  int err = EPERM;
  if (err != 0)
    return err;

#ifdef NEED_SEPARATE_REGISTER_STACK
  const size_t pagemask = ~(__getpagesize () - 1);
#endif

  lll_lock (stack_cache_lock, LLL_PRIVATE);

  list_t *runp;
  list_for_each (runp, &stack_used)
    {
      err = change_stack_perm (list_entry (runp, struct pthread, list)
#ifdef NEED_SEPARATE_REGISTER_STACK
			       , pagemask
#endif
			       );
      if (err != 0)
	break;
    }

  /* Also change the permission for the currently unused stacks.  This
     might be wasted time but better spend it here than adding a check
     in the fast path.  */
  if (err == 0)
    list_for_each (runp, &stack_cache)
      {
	err = change_stack_perm (list_entry (runp, struct pthread, list)
#ifdef NEED_SEPARATE_REGISTER_STACK
				 , pagemask
#endif
				 );
	if (err != 0)
	  break;
      }

  lll_unlock (stack_cache_lock, LLL_PRIVATE);

  return err;
}


/* In case of a fork() call the memory allocation in the child will be
   the same but only one thread is running.  All stacks except that of
   the one running thread are not used anymore.  We have to recycle
   them.  */
void
__reclaim_stacks (void)
{
  struct pthread *self = (struct pthread *) THREAD_SELF;

  /* No locking necessary.  The caller is the only stack in use.  But
     we have to be aware that we might have interrupted a list
     operation.  */

  if (in_flight_stack != 0)
    {
      bool add_p = in_flight_stack & 1;
      list_t *elem = (list_t *)(uintptr_t)(in_flight_stack & ~UINTMAX_C (1));

      if (add_p)
	{
	  /* We always add at the beginning of the list.  So in this
	     case we only need to check the beginning of these lists.  */
	  int check_list (list_t *l)
	  {
	    if (l->next->prev != l)
	      {
		assert (l->next->prev == elem);

		elem->next = l->next;
		elem->prev = l;
		l->next = elem;

		return 1;
	      }

	    return 0;
	  }

	  if (check_list (&stack_used) == 0)
	    (void) check_list (&stack_cache);
	}
      else
	{
	  /* We can simply always replay the delete operation.  */
	  elem->next->prev = elem->prev;
	  elem->prev->next = elem->next;
	}
    }

  /* Mark all stacks except the still running one as free.  */
  list_t *runp;
  list_for_each (runp, &stack_used)
    {
      struct pthread *curp = list_entry (runp, struct pthread, list);
      if (curp != self)
	{
	  /* This marks the stack as free.  */
	  curp->tid = 0;

	  /* Account for the size of the stack.  */
	  stack_cache_actsize += curp->stackblock_size;

	  if (curp->specific_used)
	    {
	      /* Clear the thread-specific data.  */
	      memset (curp->specific_1stblock, '\0',
		      sizeof (curp->specific_1stblock));

	      curp->specific_used = false;

	      size_t cnt;
	      for (cnt = 1; cnt < PTHREAD_KEY_1STLEVEL_SIZE; ++cnt)
		if (curp->specific[cnt] != NULL)
		  {
		    memset (curp->specific[cnt], '\0',
			    sizeof (curp->specific_1stblock));

		    /* We have allocated the block which we do not
		       free here so re-set the bit.  */
		    curp->specific_used = true;
		  }
	    }
	}
    }

  /* Add the stack of all running threads to the cache.  */
  list_splice (&stack_used, &stack_cache);

  /* Remove the entry for the current thread to from the cache list
     and add it to the list of running threads.  Which of the two
     lists is decided by the user_stack flag.  */
  stack_list_del (&self->list);

  /* Re-initialize the lists for all the threads.  */
  INIT_LIST_HEAD (&stack_used);
  INIT_LIST_HEAD (&__stack_user);

  if (__builtin_expect (THREAD_GETMEM (self, user_stack), 0))
    list_add (&self->list, &__stack_user);
  else
    list_add (&self->list, &stack_used);

  /* There is one thread running.  */
  __nptl_nthreads = 1;

  in_flight_stack = 0;

  /* Initialize the lock.  */
  stack_cache_lock = LLL_LOCK_INITIALIZER;
}


static void
internal_function
setxid_mark_thread (struct xid_command *cmdp, struct pthread *t)
{
  int ch;

  /* Don't let the thread exit before the setxid handler runs.  */
  t->setxid_futex = 0;

  do
    {
      ch = t->cancelhandling;

      /* If the thread is exiting right now, ignore it.  */
      if ((ch & EXITING_BITMASK) != 0)
	return;
    }
  while (atomic_compare_and_exchange_bool_acq (&t->cancelhandling,
					       ch | SETXID_BITMASK, ch));
}


static void
internal_function
setxid_unmark_thread (struct xid_command *cmdp, struct pthread *t)
{
  int ch;

  do
    {
      ch = t->cancelhandling;
      if ((ch & SETXID_BITMASK) == 0)
	return;
    }
  while (atomic_compare_and_exchange_bool_acq (&t->cancelhandling,
					       ch & ~SETXID_BITMASK, ch));

  /* Release the futex just in case.  */
  t->setxid_futex = 1;
  lll_futex_wake (&t->setxid_futex, 1, LLL_PRIVATE);
}


static int
internal_function
setxid_signal_thread (struct xid_command *cmdp, struct pthread *t)
{
  if ((t->cancelhandling & SETXID_BITMASK) == 0)
    return 0;

  int val;
  pid_t pid = getpid ();
  INTERNAL_SYSCALL_DECL (err);
  val = INTERNAL_SYSCALL (tgkill, err, 3, pid, t->tid, SIGSETXID);

  /* If this failed, it must have had not started yet or else exited.  */
  if (!INTERNAL_SYSCALL_ERROR_P (val, err))
    {
      atomic_increment (&cmdp->cntr);
      return 1;
    }
  else
    return 0;
}


int
attribute_hidden
__nptl_setxid (struct xid_command *cmdp)
{
  int signalled;
  int result;
  lll_lock (stack_cache_lock, LLL_PRIVATE);

  __xidcmd = cmdp;
  cmdp->cntr = 0;

  struct pthread *self = THREAD_SELF;

  /* Iterate over the list with system-allocated threads first.  */
  list_t *runp;
  list_for_each (runp, &stack_used)
    {
      struct pthread *t = list_entry (runp, struct pthread, list);
      if (t == self)
	continue;

      setxid_mark_thread (cmdp, t);
    }

  /* Now the list with threads using user-allocated stacks.  */
  list_for_each (runp, &__stack_user)
    {
      struct pthread *t = list_entry (runp, struct pthread, list);
      if (t == self)
	continue;

      setxid_mark_thread (cmdp, t);
    }

  /* Iterate until we don't succeed in signalling anyone.  That means
     we have gotten all running threads, and their children will be
     automatically correct once started.  */
  do
    {
      signalled = 0;

      list_for_each (runp, &stack_used)
	{
	  struct pthread *t = list_entry (runp, struct pthread, list);
	  if (t == self)
	    continue;

	  signalled += setxid_signal_thread (cmdp, t);
	}

      list_for_each (runp, &__stack_user)
	{
	  struct pthread *t = list_entry (runp, struct pthread, list);
	  if (t == self)
	    continue;

	  signalled += setxid_signal_thread (cmdp, t);
	}

      int cur = cmdp->cntr;
      while (cur != 0)
	{
	  lll_futex_wait (&cmdp->cntr, cur, LLL_PRIVATE);
	  cur = cmdp->cntr;
	}
    }
  while (signalled != 0);

  /* Clean up flags, so that no thread blocks during exit waiting
     for a signal which will never come.  */
  list_for_each (runp, &stack_used)
    {
      struct pthread *t = list_entry (runp, struct pthread, list);
      if (t == self)
	continue;

      setxid_unmark_thread (cmdp, t);
    }

  list_for_each (runp, &__stack_user)
    {
      struct pthread *t = list_entry (runp, struct pthread, list);
      if (t == self)
	continue;

      setxid_unmark_thread (cmdp, t);
    }

  /* This must be last, otherwise the current thread might not have
     permissions to send SIGSETXID syscall to the other threads.  */
  INTERNAL_SYSCALL_DECL (err);
  result = INTERNAL_SYSCALL_NCS (cmdp->syscall_no, err, 3,
				 cmdp->id[0], cmdp->id[1], cmdp->id[2]);
  if (INTERNAL_SYSCALL_ERROR_P (result, err))
    {
      __set_errno (INTERNAL_SYSCALL_ERRNO (result, err));
      result = -1;
    }

  lll_unlock (stack_cache_lock, LLL_PRIVATE);
  return result;
}

static inline void __attribute__((always_inline))
init_one_static_tls (struct pthread *curp, struct link_map *map)
{
  dtv_t *dtv = GET_DTV (TLS_TPADJ (curp));
# if defined(TLS_TCB_AT_TP)
  void *dest = (char *) curp - map->l_tls_offset;
# elif defined(TLS_DTV_AT_TP)
  void *dest = (char *) curp + map->l_tls_offset + TLS_PRE_TCB_SIZE;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* Fill in the DTV slot so that a later LD/GD access will find it.  */
  dtv[map->l_tls_modid].pointer.val = dest;
  dtv[map->l_tls_modid].pointer.is_static = true;

  /* Initialize the memory.  */
  memset (mempcpy (dest, map->l_tls_initimage, map->l_tls_initimage_size),
	  '\0', map->l_tls_blocksize - map->l_tls_initimage_size);
}

void
attribute_hidden
__pthread_init_static_tls (struct link_map *map)
{
  lll_lock (stack_cache_lock, LLL_PRIVATE);

  /* Iterate over the list with system-allocated threads first.  */
  list_t *runp;
  list_for_each (runp, &stack_used)
    init_one_static_tls (list_entry (runp, struct pthread, list), map);

  /* Now the list with threads using user-allocated stacks.  */
  list_for_each (runp, &__stack_user)
    init_one_static_tls (list_entry (runp, struct pthread, list), map);

  lll_unlock (stack_cache_lock, LLL_PRIVATE);
}


void
attribute_hidden
__wait_lookup_done (void)
{
  lll_lock (stack_cache_lock, LLL_PRIVATE);

  struct pthread *self = THREAD_SELF;

  /* Iterate over the list with system-allocated threads first.  */
  list_t *runp;
  list_for_each (runp, &stack_used)
    {
      struct pthread *t = list_entry (runp, struct pthread, list);
      if (t == self || t->header.gscope_flag == THREAD_GSCOPE_FLAG_UNUSED)
	continue;

      int *const gscope_flagp = &t->header.gscope_flag;

      /* We have to wait until this thread is done with the global
	 scope.  First tell the thread that we are waiting and
	 possibly have to be woken.  */
      if (atomic_compare_and_exchange_bool_acq (gscope_flagp,
						THREAD_GSCOPE_FLAG_WAIT,
						THREAD_GSCOPE_FLAG_USED))
	continue;

      do
	lll_futex_wait (gscope_flagp, THREAD_GSCOPE_FLAG_WAIT, LLL_PRIVATE);
      while (*gscope_flagp == THREAD_GSCOPE_FLAG_WAIT);
    }

  /* Now the list with threads using user-allocated stacks.  */
  list_for_each (runp, &__stack_user)
    {
      struct pthread *t = list_entry (runp, struct pthread, list);
      if (t == self || t->header.gscope_flag == THREAD_GSCOPE_FLAG_UNUSED)
	continue;

      int *const gscope_flagp = &t->header.gscope_flag;

      /* We have to wait until this thread is done with the global
	 scope.  First tell the thread that we are waiting and
	 possibly have to be woken.  */
      if (atomic_compare_and_exchange_bool_acq (gscope_flagp,
						THREAD_GSCOPE_FLAG_WAIT,
						THREAD_GSCOPE_FLAG_USED))
	continue;

      do
	lll_futex_wait (gscope_flagp, THREAD_GSCOPE_FLAG_WAIT, LLL_PRIVATE);
      while (*gscope_flagp == THREAD_GSCOPE_FLAG_WAIT);
    }

  lll_unlock (stack_cache_lock, LLL_PRIVATE);
}
