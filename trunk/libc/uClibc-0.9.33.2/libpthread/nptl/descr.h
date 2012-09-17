/* Copyright (C) 2002-2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _DESCR_H
#define _DESCR_H	1

#include <limits.h>
#include <sched.h>
#include <setjmp.h>
#include <stdbool.h>
#include <sys/types.h>
#include <hp-timing.h>
#include <list.h>
#include <lowlevellock.h>
#include <pthreaddef.h>
#include "../nptl_db/thread_db.h"
#include <tls.h>
#ifdef HAVE_FORCED_UNWIND
# include <unwind.h>
#endif
#define __need_res_state
#include <resolv.h>
#include <bits/kernel-features.h>
#include "uClibc-glue.h"

#ifndef TCB_ALIGNMENT
# define TCB_ALIGNMENT	sizeof (double)
#endif


/* We keep thread specific data in a special data structure, a two-level
   array.  The top-level array contains pointers to dynamically allocated
   arrays of a certain number of data pointers.  So we can implement a
   sparse array.  Each dynamic second-level array has
        PTHREAD_KEY_2NDLEVEL_SIZE
   entries.  This value shouldn't be too large.  */
#define PTHREAD_KEY_2NDLEVEL_SIZE       32

/* We need to address PTHREAD_KEYS_MAX key with PTHREAD_KEY_2NDLEVEL_SIZE
   keys in each subarray.  */
#define PTHREAD_KEY_1STLEVEL_SIZE \
  ((PTHREAD_KEYS_MAX + PTHREAD_KEY_2NDLEVEL_SIZE - 1) \
   / PTHREAD_KEY_2NDLEVEL_SIZE)




/* Internal version of the buffer to store cancellation handler
   information.  */
struct pthread_unwind_buf
{
  struct
  {
    __jmp_buf jmp_buf;
    int mask_was_saved;
  } cancel_jmp_buf[1];

  union
  {
    /* This is the placeholder of the public version.  */
    void *pad[4];

    struct
    {
      /* Pointer to the previous cleanup buffer.  */
      struct pthread_unwind_buf *prev;

      /* Backward compatibility: state of the old-style cleanup
	 handler at the time of the previous new-style cleanup handler
	 installment.  */
      struct _pthread_cleanup_buffer *cleanup;

      /* Cancellation type before the push call.  */
      int canceltype;
    } data;
  } priv;
};


/* Opcodes and data types for communication with the signal handler to
   change user/group IDs.  */
struct xid_command
{
  int syscall_no;
  long int id[3];
  volatile int cntr;
};


/* Data structure used by the kernel to find robust futexes.  */
struct robust_list_head
{
  void *list;
  long int futex_offset;
  void *list_op_pending;
};


/* Data strcture used to handle thread priority protection.  */
struct priority_protection_data
{
  int priomax;
  unsigned int priomap[];
};


/* Thread descriptor data structure.  */
struct pthread
{
  union
  {
#if !defined(TLS_DTV_AT_TP)
    /* This overlaps the TCB as used for TLS without threads (see tls.h).  */
    tcbhead_t header;
#else
    struct
    {
      int multiple_threads;
      int gscope_flag;
# ifndef __ASSUME_PRIVATE_FUTEX
      int private_futex;
# endif
    } header;
#endif

    /* This extra padding has no special purpose, and this structure layout
       is private and subject to change without affecting the official ABI.
       We just have it here in case it might be convenient for some
       implementation-specific instrumentation hack or suchlike.  */
    void *__padding[24];
  };

  /* This descriptor's link on the `stack_used' or `__stack_user' list.  */
  list_t list;

  /* Thread ID - which is also a 'is this thread descriptor (and
     therefore stack) used' flag.  */
  pid_t tid;

  /* Process ID - thread group ID in kernel speak.  */
  pid_t pid;

  /* List of robust mutexes the thread is holding.  */
#ifdef __PTHREAD_MUTEX_HAVE_PREV
  void *robust_prev;
  struct robust_list_head robust_head;

  /* The list above is strange.  It is basically a double linked list
     but the pointer to the next/previous element of the list points
     in the middle of the object, the __next element.  Whenever
     casting to __pthread_list_t we need to adjust the pointer
     first.  */
# define QUEUE_PTR_ADJUST (offsetof (__pthread_list_t, __next))

# define ENQUEUE_MUTEX_BOTH(mutex, val)					      \
  do {									      \
    __pthread_list_t *next = (__pthread_list_t *)			      \
      ((((uintptr_t) THREAD_GETMEM (THREAD_SELF, robust_head.list)) & ~1ul)   \
       - QUEUE_PTR_ADJUST);						      \
    next->__prev = (void *) &mutex->__data.__list.__next;		      \
    mutex->__data.__list.__next = THREAD_GETMEM (THREAD_SELF,		      \
						 robust_head.list);	      \
    mutex->__data.__list.__prev = (void *) &THREAD_SELF->robust_head;	      \
    THREAD_SETMEM (THREAD_SELF, robust_head.list,			      \
		   (void *) (((uintptr_t) &mutex->__data.__list.__next)	      \
			     | val));					      \
  } while (0)
# define DEQUEUE_MUTEX(mutex) \
  do {									      \
    __pthread_list_t *next = (__pthread_list_t *)			      \
      ((char *) (((uintptr_t) mutex->__data.__list.__next) & ~1ul)	      \
       - QUEUE_PTR_ADJUST);						      \
    next->__prev = mutex->__data.__list.__prev;				      \
    __pthread_list_t *prev = (__pthread_list_t *)			      \
      ((char *) (((uintptr_t) mutex->__data.__list.__prev) & ~1ul)	      \
       - QUEUE_PTR_ADJUST);						      \
    prev->__next = mutex->__data.__list.__next;				      \
    mutex->__data.__list.__prev = NULL;					      \
    mutex->__data.__list.__next = NULL;					      \
  } while (0)
#else
  union
  {
    __pthread_slist_t robust_list;
    struct robust_list_head robust_head;
  };

# define ENQUEUE_MUTEX_BOTH(mutex, val)					      \
  do {									      \
    mutex->__data.__list.__next						      \
      = THREAD_GETMEM (THREAD_SELF, robust_list.__next);		      \
    THREAD_SETMEM (THREAD_SELF, robust_list.__next,			      \
		   (void *) (((uintptr_t) &mutex->__data.__list) | val));     \
  } while (0)
# define DEQUEUE_MUTEX(mutex) \
  do {									      \
    __pthread_slist_t *runp = (__pthread_slist_t *)			      \
      (((uintptr_t) THREAD_GETMEM (THREAD_SELF, robust_list.__next)) & ~1ul); \
    if (runp == &mutex->__data.__list)					      \
      THREAD_SETMEM (THREAD_SELF, robust_list.__next, runp->__next);	      \
    else								      \
      {									      \
	__pthread_slist_t *next = (__pthread_slist_t *)		      \
	  (((uintptr_t) runp->__next) & ~1ul);				      \
	while (next != &mutex->__data.__list)				      \
	  {								      \
	    runp = next;						      \
	    next = (__pthread_slist_t *) (((uintptr_t) runp->__next) & ~1ul); \
	  }								      \
									      \
	runp->__next = next->__next;					      \
	mutex->__data.__list.__next = NULL;				      \
      }									      \
  } while (0)
#endif
#define ENQUEUE_MUTEX(mutex) ENQUEUE_MUTEX_BOTH (mutex, 0)
#define ENQUEUE_MUTEX_PI(mutex) ENQUEUE_MUTEX_BOTH (mutex, 1)

  /* List of cleanup buffers.  */
  struct _pthread_cleanup_buffer *cleanup;

  /* Unwind information.  */
  struct pthread_unwind_buf *cleanup_jmp_buf;
#define HAVE_CLEANUP_JMP_BUF

  /* Flags determining processing of cancellation.  */
  int cancelhandling;
  /* Bit set if cancellation is disabled.  */
#define CANCELSTATE_BIT		0
#define CANCELSTATE_BITMASK	(0x01 << CANCELSTATE_BIT)
  /* Bit set if asynchronous cancellation mode is selected.  */
#define CANCELTYPE_BIT		1
#define CANCELTYPE_BITMASK	(0x01 << CANCELTYPE_BIT)
  /* Bit set if canceling has been initiated.  */
#define CANCELING_BIT		2
#define CANCELING_BITMASK	(0x01 << CANCELING_BIT)
  /* Bit set if canceled.  */
#define CANCELED_BIT		3
#define CANCELED_BITMASK	(0x01 << CANCELED_BIT)
  /* Bit set if thread is exiting.  */
#define EXITING_BIT		4
#define EXITING_BITMASK		(0x01 << EXITING_BIT)
  /* Bit set if thread terminated and TCB is freed.  */
#define TERMINATED_BIT		5
#define TERMINATED_BITMASK	(0x01 << TERMINATED_BIT)
  /* Bit set if thread is supposed to change XID.  */
#define SETXID_BIT		6
#define SETXID_BITMASK		(0x01 << SETXID_BIT)
  /* Mask for the rest.  Helps the compiler to optimize.  */
#define CANCEL_RESTMASK		0xffffff80

#define CANCEL_ENABLED_AND_CANCELED(value) \
  (((value) & (CANCELSTATE_BITMASK | CANCELED_BITMASK | EXITING_BITMASK	      \
	       | CANCEL_RESTMASK | TERMINATED_BITMASK)) == CANCELED_BITMASK)
#define CANCEL_ENABLED_AND_CANCELED_AND_ASYNCHRONOUS(value) \
  (((value) & (CANCELSTATE_BITMASK | CANCELTYPE_BITMASK | CANCELED_BITMASK    \
	       | EXITING_BITMASK | CANCEL_RESTMASK | TERMINATED_BITMASK))     \
   == (CANCELTYPE_BITMASK | CANCELED_BITMASK))

  /* Flags.  Including those copied from the thread attribute.  */
  int flags;

  /* We allocate one block of references here.  This should be enough
     to avoid allocating any memory dynamically for most applications.  */
  struct pthread_key_data
  {
    /* Sequence number.  We use uintptr_t to not require padding on
       32- and 64-bit machines.  On 64-bit machines it helps to avoid
       wrapping, too.  */
    uintptr_t seq;

    /* Data pointer.  */
    void *data;
  } specific_1stblock[PTHREAD_KEY_2NDLEVEL_SIZE];

  /* Two-level array for the thread-specific data.  */
  struct pthread_key_data *specific[PTHREAD_KEY_1STLEVEL_SIZE];

  /* Flag which is set when specific data is set.  */
  bool specific_used;

  /* True if events must be reported.  */
  bool report_events;

  /* True if the user provided the stack.  */
  bool user_stack;

  /* True if thread must stop at startup time.  */
  bool stopped_start;

  /* The parent's cancel handling at the time of the pthread_create
     call.  This might be needed to undo the effects of a cancellation.  */
  int parent_cancelhandling;

  /* Lock to synchronize access to the descriptor.  */
  int lock;

  /* Lock for synchronizing setxid calls.  */
  int setxid_futex;

#if HP_TIMING_AVAIL
  /* Offset of the CPU clock at start thread start time.  */
  hp_timing_t cpuclock_offset;
#endif

  /* If the thread waits to join another one the ID of the latter is
     stored here.

     In case a thread is detached this field contains a pointer of the
     TCB if the thread itself.  This is something which cannot happen
     in normal operation.  */
  struct pthread *joinid;
  /* Check whether a thread is detached.  */
#define IS_DETACHED(pd) ((pd)->joinid == (pd))

  /* The result of the thread function.  */
  void *result;

  /* Scheduling parameters for the new thread.  */
  struct sched_param schedparam;
  int schedpolicy;

  /* Start position of the code to be executed and the argument passed
     to the function.  */
  void *(*start_routine) (void *);
  void *arg;

  /* Debug state.  */
  td_eventbuf_t eventbuf;
  /* Next descriptor with a pending event.  */
  struct pthread *nextevent;

#ifdef HAVE_FORCED_UNWIND
  /* Machine-specific unwind info.  */
  struct _Unwind_Exception exc;
#endif

  /* If nonzero pointer to area allocated for the stack and its
     size.  */
  void *stackblock;
  size_t stackblock_size;
  /* Size of the included guard area.  */
  size_t guardsize;
  /* This is what the user specified and what we will report.  */
  size_t reported_guardsize;

  /* Thread Priority Protection data.  */
  struct priority_protection_data *tpp;

  /* Resolver state.  */
  struct __res_state res;

  /* This member must be last.  */
  char end_padding[];

#define PTHREAD_STRUCT_END_PADDING \
  (sizeof (struct pthread) - offsetof (struct pthread, end_padding))
} __attribute ((aligned (TCB_ALIGNMENT)));


#endif	/* descr.h */
