/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* The "thread manager" thread: manages creation and termination of threads */

#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>		/* for poll */
#include <sys/mman.h>           /* for mmap */
#include <sys/param.h>
#include <sys/time.h>
#include <sys/wait.h>           /* for waitpid macros */
#include <locale.h>		/* for __uselocale */
#include <resolv.h>		/* for __resp */

#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#include "semaphore.h"
#include <not-cancel.h>

/* For debugging purposes put the maximum number of threads in a variable.  */
const int __linuxthreads_pthread_threads_max = PTHREAD_THREADS_MAX;

#ifndef THREAD_SELF
/* Indicate whether at least one thread has a user-defined stack (if 1),
   or if all threads have stacks supplied by LinuxThreads (if 0). */
int __pthread_nonstandard_stacks;
#endif

/* Number of active entries in __pthread_handles (used by gdb) */
__volatile__ int __pthread_handles_num = 2;

/* Whether to use debugger additional actions for thread creation
   (set to 1 by gdb) */
__volatile__ int __pthread_threads_debug;

/* Globally enabled events.  */
__volatile__ td_thr_events_t __pthread_threads_events;

/* Pointer to thread descriptor with last event.  */
__volatile__ pthread_descr __pthread_last_event;

static pthread_descr manager_thread;

/* Mapping from stack segment to thread descriptor. */
/* Stack segment numbers are also indices into the __pthread_handles array. */
/* Stack segment number 0 is reserved for the initial thread. */

#if FLOATING_STACKS
# define thread_segment(seq) NULL
#else
static __inline__ pthread_descr thread_segment(int seg)
{
# ifdef _STACK_GROWS_UP
  return (pthread_descr)(THREAD_STACK_START_ADDRESS + (seg - 1) * STACK_SIZE)
         + 1;
# else
  return (pthread_descr)(THREAD_STACK_START_ADDRESS - (seg - 1) * STACK_SIZE)
         - 1;
# endif
}
#endif

/* Flag set in signal handler to record child termination */

static __volatile__ int terminated_children;

/* Flag set when the initial thread is blocked on pthread_exit waiting
   for all other threads to terminate */

static int main_thread_exiting;

/* Counter used to generate unique thread identifier.
   Thread identifier is pthread_threads_counter + segment. */

static pthread_t pthread_threads_counter;

/* Forward declarations */

static int pthread_handle_create(pthread_t *thread, const pthread_attr_t *attr,
                                 void * (*start_routine)(void *), void *arg,
                                 sigset_t *mask, int father_pid,
				 int report_events,
				 td_thr_events_t *event_maskp);
static void pthread_handle_free(pthread_t th_id);
static void pthread_handle_exit(pthread_descr issuing_thread, int exitcode)
     __attribute__ ((noreturn));
static void pthread_reap_children(void);
static void pthread_kill_all_threads(int sig, int main_thread_also);
static void pthread_for_each_thread(void *arg,
    void (*fn)(void *, pthread_descr));

/* The server thread managing requests for thread creation and termination */

int
__attribute__ ((noreturn))
__pthread_manager(void *arg)
{
  pthread_descr self = manager_thread = arg;
  int reqfd = __pthread_manager_reader;
  struct pollfd ufd;
  sigset_t manager_mask;
  int n;
  struct pthread_request request;

  /* If we have special thread_self processing, initialize it.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, 1);
#endif
#if !(USE_TLS && HAVE___THREAD)
  /* Set the error variable.  */
  self->p_errnop = &self->p_errno;
  self->p_h_errnop = &self->p_h_errno;
#endif
  /* Block all signals except __pthread_sig_cancel and SIGTRAP */
  __sigfillset(&manager_mask);
  sigdelset(&manager_mask, __pthread_sig_cancel); /* for thread termination */
  sigdelset(&manager_mask, SIGTRAP);            /* for debugging purposes */
  if (__pthread_threads_debug && __pthread_sig_debug > 0)
    sigdelset(&manager_mask, __pthread_sig_debug);
  sigprocmask(SIG_SETMASK, &manager_mask, NULL);
  /* Raise our priority to match that of main thread */
  __pthread_manager_adjust_prio(__pthread_main_thread->p_priority);
  /* Synchronize debugging of the thread manager */
  n = TEMP_FAILURE_RETRY(read_not_cancel(reqfd, (char *)&request,
					 sizeof(request)));
  ASSERT(n == sizeof(request) && request.req_kind == REQ_DEBUG);
  ufd.fd = reqfd;
  ufd.events = POLLIN;
  /* Enter server loop */
  while(1) {
    n = __poll(&ufd, 1, 2000);

    /* Check for termination of the main thread */
    if (getppid() == 1) {
      pthread_kill_all_threads(SIGKILL, 0);
      _exit(0);
    }
    /* Check for dead children */
    if (terminated_children) {
      terminated_children = 0;
      pthread_reap_children();
    }
    /* Read and execute request */
    if (n == 1 && (ufd.revents & POLLIN)) {
      n = TEMP_FAILURE_RETRY(read_not_cancel(reqfd, (char *)&request,
					     sizeof(request)));
#ifdef DEBUG
      if (n < 0) {
	char d[64];
	write(STDERR_FILENO, d, snprintf(d, sizeof(d), "*** read err %m\n"));
      } else if (n != sizeof(request)) {
	write(STDERR_FILENO, "*** short read in manager\n", 26);
      }
#endif

      switch(request.req_kind) {
      case REQ_CREATE:
        request.req_thread->p_retcode =
          pthread_handle_create((pthread_t *) &request.req_thread->p_retval,
                                request.req_args.create.attr,
                                request.req_args.create.fn,
                                request.req_args.create.arg,
                                &request.req_args.create.mask,
                                request.req_thread->p_pid,
				request.req_thread->p_report_events,
				&request.req_thread->p_eventbuf.eventmask);
        restart(request.req_thread);
        break;
      case REQ_FREE:
	pthread_handle_free(request.req_args.free.thread_id);
        break;
      case REQ_PROCESS_EXIT:
        pthread_handle_exit(request.req_thread,
                            request.req_args.exit.code);
	/* NOTREACHED */
        break;
      case REQ_MAIN_THREAD_EXIT:
        main_thread_exiting = 1;
	/* Reap children in case all other threads died and the signal handler
	   went off before we set main_thread_exiting to 1, and therefore did
	   not do REQ_KICK. */
	pthread_reap_children();

        if (__pthread_main_thread->p_nextlive == __pthread_main_thread) {
          restart(__pthread_main_thread);
	  /* The main thread will now call exit() which will trigger an
	     __on_exit handler, which in turn will send REQ_PROCESS_EXIT
	     to the thread manager. In case you are wondering how the
	     manager terminates from its loop here. */
	}
        break;
      case REQ_POST:
        sem_post(request.req_args.post);
        break;
      case REQ_DEBUG:
	/* Make gdb aware of new thread and gdb will restart the
	   new thread when it is ready to handle the new thread. */
	if (__pthread_threads_debug && __pthread_sig_debug > 0)
	  raise(__pthread_sig_debug);
        break;
      case REQ_KICK:
	/* This is just a prod to get the manager to reap some
	   threads right away, avoiding a potential delay at shutdown. */
	break;
      case REQ_FOR_EACH_THREAD:
	pthread_for_each_thread(request.req_args.for_each.arg,
	                        request.req_args.for_each.fn);
	restart(request.req_thread);
	break;
      }
    }
  }
}

int __pthread_manager_event(void *arg)
{
  pthread_descr self = arg;
  /* If we have special thread_self processing, initialize it.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, 1);
#endif

  /* Get the lock the manager will free once all is correctly set up.  */
  __pthread_lock (THREAD_GETMEM(self, p_lock), NULL);
  /* Free it immediately.  */
  __pthread_unlock (THREAD_GETMEM(self, p_lock));

  return __pthread_manager(arg);
}

/* Process creation */

static int
__attribute__ ((noreturn))
pthread_start_thread(void *arg)
{
  pthread_descr self = (pthread_descr) arg;
  struct pthread_request request;
  void * outcome;
#if HP_TIMING_AVAIL
  hp_timing_t tmpclock;
#endif
  /* Initialize special thread_self processing, if any.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, self->p_nr);
#endif
#if HP_TIMING_AVAIL
  HP_TIMING_NOW (tmpclock);
  THREAD_SETMEM (self, p_cpuclock_offset, tmpclock);
#endif
  /* Make sure our pid field is initialized, just in case we get there
     before our father has initialized it. */
  THREAD_SETMEM(self, p_pid, __getpid());
  /* Initial signal mask is that of the creating thread. (Otherwise,
     we'd just inherit the mask of the thread manager.) */
  sigprocmask(SIG_SETMASK, &self->p_start_args.mask, NULL);
  /* Set the scheduling policy and priority for the new thread, if needed */
  if (THREAD_GETMEM(self, p_start_args.schedpolicy) >= 0)
    /* Explicit scheduling attributes were provided: apply them */
    __sched_setscheduler(THREAD_GETMEM(self, p_pid),
			 THREAD_GETMEM(self, p_start_args.schedpolicy),
                         &self->p_start_args.schedparam);
  else if (manager_thread->p_priority > 0)
    /* Default scheduling required, but thread manager runs in realtime
       scheduling: switch new thread to SCHED_OTHER policy */
    {
      struct sched_param default_params;
      default_params.sched_priority = 0;
      __sched_setscheduler(THREAD_GETMEM(self, p_pid),
                           SCHED_OTHER, &default_params);
    }
#if !(USE_TLS && HAVE___THREAD)
  /* Initialize thread-locale current locale to point to the global one.
     With __thread support, the variable's initializer takes care of this.  */
  __uselocale (LC_GLOBAL_LOCALE);
#elif defined __UCLIBC_HAS_RESOLVER_SUPPORT__
  /* Initialize __resp.  */
  __resp = &self->p_res;
#endif
  /* Make gdb aware of new thread */
  if (__pthread_threads_debug && __pthread_sig_debug > 0) {
    request.req_thread = self;
    request.req_kind = REQ_DEBUG;
    TEMP_FAILURE_RETRY(write_not_cancel(__pthread_manager_request,
					(char *) &request, sizeof(request)));
    suspend(self);
  }
  /* Run the thread code */
  outcome = self->p_start_args.start_routine(THREAD_GETMEM(self,
							   p_start_args.arg));
  /* Exit with the given return value */
  __pthread_do_exit(outcome, CURRENT_STACK_FRAME);
}

static int
__attribute__ ((noreturn))
pthread_start_thread_event(void *arg)
{
  pthread_descr self = (pthread_descr) arg;

#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, self->p_nr);
#endif
  /* Make sure our pid field is initialized, just in case we get there
     before our father has initialized it. */
  THREAD_SETMEM(self, p_pid, __getpid());
  /* Get the lock the manager will free once all is correctly set up.  */
  __pthread_lock (THREAD_GETMEM(self, p_lock), NULL);
  /* Free it immediately.  */
  __pthread_unlock (THREAD_GETMEM(self, p_lock));

  /* Continue with the real function.  */
  pthread_start_thread (arg);
}

#if defined USE_TLS && !FLOATING_STACKS
# error "TLS can only work with floating stacks"
#endif

static int pthread_allocate_stack(const pthread_attr_t *attr,
                                  pthread_descr default_new_thread,
                                  int pagesize,
                                  char ** out_new_thread,
                                  char ** out_new_thread_bottom,
                                  char ** out_guardaddr,
                                  size_t * out_guardsize,
                                  size_t * out_stacksize)
{
  pthread_descr new_thread;
  char * new_thread_bottom;
  char * guardaddr;
  size_t stacksize, guardsize;

#ifdef USE_TLS
  /* TLS cannot work with fixed thread descriptor addresses.  */
  assert (default_new_thread == NULL);
#endif

  if (attr != NULL && attr->__stackaddr_set)
    {
#ifdef _STACK_GROWS_UP
      /* The user provided a stack. */
# ifdef USE_TLS
      /* This value is not needed.  */
      new_thread = (pthread_descr) attr->__stackaddr;
      new_thread_bottom = (char *) new_thread;
# else
      new_thread = (pthread_descr) attr->__stackaddr;
      new_thread_bottom = (char *) (new_thread + 1);
# endif
      guardaddr = attr->__stackaddr + attr->__stacksize;
      guardsize = 0;
#else
      /* The user provided a stack.  For now we interpret the supplied
	 address as 1 + the highest addr. in the stack segment.  If a
	 separate register stack is needed, we place it at the low end
	 of the segment, relying on the associated stacksize to
	 determine the low end of the segment.  This differs from many
	 (but not all) other pthreads implementations.  The intent is
	 that on machines with a single stack growing toward higher
	 addresses, stackaddr would be the lowest address in the stack
	 segment, so that it is consistently close to the initial sp
	 value. */
# ifdef USE_TLS
      new_thread = (pthread_descr) attr->__stackaddr;
# else
      new_thread =
        (pthread_descr) ((long)(attr->__stackaddr) & -sizeof(void *)) - 1;
# endif
      new_thread_bottom = (char *) attr->__stackaddr - attr->__stacksize;
      guardaddr = new_thread_bottom;
      guardsize = 0;
#endif
#ifndef THREAD_SELF
      __pthread_nonstandard_stacks = 1;
#endif
#ifndef USE_TLS
      /* Clear the thread data structure.  */
      memset (new_thread, '\0', sizeof (*new_thread));
#endif
      stacksize = attr->__stacksize;
    }
  else
    {
#ifdef NEED_SEPARATE_REGISTER_STACK
      const size_t granularity = 2 * pagesize;
      /* Try to make stacksize/2 a multiple of pagesize */
#else
      const size_t granularity = pagesize;
#endif
      void *map_addr;

      /* Allocate space for stack and thread descriptor at default address */
#if FLOATING_STACKS
      if (attr != NULL)
	{
	  guardsize = page_roundup (attr->__guardsize, granularity);
	  stacksize = __pthread_max_stacksize - guardsize;
	  stacksize = MIN (stacksize,
			   page_roundup (attr->__stacksize, granularity));
	}
      else
	{
	  guardsize = granularity;
	  stacksize = __pthread_max_stacksize - guardsize;
	}

      map_addr = mmap(NULL, stacksize + guardsize,
		      PROT_READ | PROT_WRITE | PROT_EXEC,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (map_addr == MAP_FAILED)
        /* No more memory available.  */
        return -1;

# ifdef NEED_SEPARATE_REGISTER_STACK
      guardaddr = map_addr + stacksize / 2;
      if (guardsize > 0)
	mprotect (guardaddr, guardsize, PROT_NONE);

      new_thread_bottom = (char *) map_addr;
#  ifdef USE_TLS
      new_thread = ((pthread_descr) (new_thread_bottom + stacksize
				     + guardsize));
#  else
      new_thread = ((pthread_descr) (new_thread_bottom + stacksize
				     + guardsize)) - 1;
#  endif
# elif defined _STACK_GROWS_DOWN
      guardaddr = map_addr;
      if (guardsize > 0)
	mprotect (guardaddr, guardsize, PROT_NONE);

      new_thread_bottom = (char *) map_addr + guardsize;
#  ifdef USE_TLS
      new_thread = ((pthread_descr) (new_thread_bottom + stacksize));
#  else
      new_thread = ((pthread_descr) (new_thread_bottom + stacksize)) - 1;
#  endif
# elif defined _STACK_GROWS_UP
      guardaddr = map_addr + stacksize;
      if (guardsize > 0)
	mprotect (guardaddr, guardsize, PROT_NONE);

      new_thread = (pthread_descr) map_addr;
#  ifdef USE_TLS
      new_thread_bottom = (char *) new_thread;
#  else
      new_thread_bottom = (char *) (new_thread + 1);
#  endif
# else
#  error You must define a stack direction
# endif /* Stack direction */
#else /* !FLOATING_STACKS */
# if !defined NEED_SEPARATE_REGISTER_STACK && defined _STACK_GROWS_DOWN
      void *res_addr;
# endif

      if (attr != NULL)
	{
	  guardsize = page_roundup (attr->__guardsize, granularity);
	  stacksize = STACK_SIZE - guardsize;
	  stacksize = MIN (stacksize,
			   page_roundup (attr->__stacksize, granularity));
	}
      else
	{
	  guardsize = granularity;
	  stacksize = STACK_SIZE - granularity;
	}

# ifdef NEED_SEPARATE_REGISTER_STACK
      new_thread = default_new_thread;
      new_thread_bottom = (char *) (new_thread + 1) - stacksize - guardsize;
      /* Includes guard area, unlike the normal case.  Use the bottom
       end of the segment as backing store for the register stack.
       Needed on IA64.  In this case, we also map the entire stack at
       once.  According to David Mosberger, that's cheaper.  It also
       avoids the risk of intermittent failures due to other mappings
       in the same region.  The cost is that we might be able to map
       slightly fewer stacks.  */

      /* First the main stack: */
      map_addr = (caddr_t)((char *)(new_thread + 1) - stacksize / 2);
      res_addr = mmap(map_addr, stacksize / 2,
		      PROT_READ | PROT_WRITE | PROT_EXEC,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (res_addr != map_addr)
	{
	  /* Bad luck, this segment is already mapped. */
	  if (res_addr != MAP_FAILED)
	    munmap(res_addr, stacksize / 2);
	  return -1;
	}
      /* Then the register stack:	*/
      map_addr = (caddr_t)new_thread_bottom;
      res_addr = mmap(map_addr, stacksize/2,
		      PROT_READ | PROT_WRITE | PROT_EXEC,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (res_addr != map_addr)
	{
	  if (res_addr != MAP_FAILED)
	    munmap(res_addr, stacksize / 2);
	  munmap((caddr_t)((char *)(new_thread + 1) - stacksize/2),
		 stacksize/2);
	  return -1;
	}

      guardaddr = new_thread_bottom + stacksize/2;
      /* We leave the guard area in the middle unmapped.	*/
# else  /* !NEED_SEPARATE_REGISTER_STACK */
#  ifdef _STACK_GROWS_DOWN
      new_thread = default_new_thread;
      new_thread_bottom = (char *) (new_thread + 1) - stacksize;
      map_addr = new_thread_bottom - guardsize;
      res_addr = mmap(map_addr, stacksize + guardsize,
		      PROT_READ | PROT_WRITE | PROT_EXEC,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (res_addr != map_addr)
	{
	  /* Bad luck, this segment is already mapped. */
	  if (res_addr != MAP_FAILED)
	    munmap (res_addr, stacksize + guardsize);
	  return -1;
	}

      /* We manage to get a stack.  Protect the guard area pages if
	 necessary.  */
      guardaddr = map_addr;
      if (guardsize > 0)
	mprotect (guardaddr, guardsize, PROT_NONE);
#  else
      /* The thread description goes at the bottom of this area, and
       * the stack starts directly above it.
       */
      new_thread = (pthread_descr)((unsigned long)default_new_thread &~ (STACK_SIZE - 1));
      map_addr = mmap(new_thread, stacksize + guardsize,
		      PROT_READ | PROT_WRITE | PROT_EXEC,
		      MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (map_addr == MAP_FAILED)
	  return -1;

      new_thread_bottom = map_addr + sizeof(*new_thread);
      guardaddr = map_addr + stacksize;
      if (guardsize > 0)
	  mprotect (guardaddr, guardsize, PROT_NONE);

#  endif /* stack direction */
# endif  /* !NEED_SEPARATE_REGISTER_STACK */
#endif   /* !FLOATING_STACKS */
    }
  *out_new_thread = (char *) new_thread;
  *out_new_thread_bottom = new_thread_bottom;
  *out_guardaddr = guardaddr;
  *out_guardsize = guardsize;
#ifdef NEED_SEPARATE_REGISTER_STACK
  *out_stacksize = stacksize / 2;
#else
  *out_stacksize = stacksize;
#endif
  return 0;
}

static int pthread_handle_create(pthread_t *thread, const pthread_attr_t *attr,
				 void * (*start_routine)(void *), void *arg,
				 sigset_t * mask, int father_pid,
				 int report_events,
				 td_thr_events_t *event_maskp)
{
  size_t sseg;
  int pid;
  pthread_descr new_thread;
  char *stack_addr;
  char * new_thread_bottom;
  pthread_t new_thread_id;
  char *guardaddr = NULL;
  size_t guardsize = 0, stksize = 0;
  int pagesize = __getpagesize();
  int saved_errno = 0;

#ifdef USE_TLS
  new_thread = _dl_allocate_tls (NULL);
  if (new_thread == NULL)
    return EAGAIN;
# if defined(TLS_DTV_AT_TP)
  /* pthread_descr is below TP.  */
  new_thread = (pthread_descr) ((char *) new_thread - TLS_PRE_TCB_SIZE);
# endif
#else
  /* Prevent warnings.  */
  new_thread = NULL;
#endif

  /* First check whether we have to change the policy and if yes, whether
     we can  do this.  Normally this should be done by examining the
     return value of the __sched_setscheduler call in pthread_start_thread
     but this is hard to implement.  FIXME  */
  if (attr != NULL && attr->__schedpolicy != SCHED_OTHER && geteuid () != 0)
    return EPERM;
  /* Find a free segment for the thread, and allocate a stack if needed */
  for (sseg = 2; ; sseg++)
    {
      if (sseg >= PTHREAD_THREADS_MAX)
	{
#ifdef USE_TLS
# if defined(TLS_DTV_AT_TP)
	  new_thread = (pthread_descr) ((char *) new_thread + TLS_PRE_TCB_SIZE);
# endif
	  _dl_deallocate_tls (new_thread, true);
#endif
	  return EAGAIN;
	}
      if (__pthread_handles[sseg].h_descr != NULL)
	continue;
      if (pthread_allocate_stack(attr, thread_segment(sseg),
				 pagesize, &stack_addr, &new_thread_bottom,
                                 &guardaddr, &guardsize, &stksize) == 0)
	{
#ifdef USE_TLS
	  new_thread->p_stackaddr = stack_addr;
#else
	  new_thread = (pthread_descr) stack_addr;
#endif
	  break;
#ifndef __ARCH_USE_MMU__
	} else {
	  /* When there is MMU, mmap () is used to allocate the stack. If one
	   * segment is already mapped, we should continue to see if we can
	   * use the next one. However, when there is no MMU, malloc () is used.
	   * It's waste of CPU cycles to continue to try if it fails.  */
	  return EAGAIN;
#endif
	}
    }
  __pthread_handles_num++;
  /* Allocate new thread identifier */
  pthread_threads_counter += PTHREAD_THREADS_MAX;
  new_thread_id = sseg + pthread_threads_counter;
  /* Initialize the thread descriptor.  Elements which have to be
     initialized to zero already have this value.  */
#if !defined USE_TLS || !TLS_DTV_AT_TP
  new_thread->p_header.data.tcb = new_thread;
  new_thread->p_header.data.self = new_thread;
#endif
#if TLS_MULTIPLE_THREADS_IN_TCB || !defined USE_TLS || !TLS_DTV_AT_TP
  new_thread->p_multiple_threads = 1;
#endif
  new_thread->p_tid = new_thread_id;
  new_thread->p_lock = &(__pthread_handles[sseg].h_lock);
  new_thread->p_cancelstate = PTHREAD_CANCEL_ENABLE;
  new_thread->p_canceltype = PTHREAD_CANCEL_DEFERRED;
#if !(USE_TLS && HAVE___THREAD)
  new_thread->p_errnop = &new_thread->p_errno;
  new_thread->p_h_errnop = &new_thread->p_h_errno;
  new_thread->p_resp = &new_thread->p_res;
#endif
  new_thread->p_guardaddr = guardaddr;
  new_thread->p_guardsize = guardsize;
  new_thread->p_nr = sseg;
  new_thread->p_inheritsched = attr ? attr->__inheritsched : 0;
  new_thread->p_alloca_cutoff = stksize / 4 > __MAX_ALLOCA_CUTOFF
				 ? __MAX_ALLOCA_CUTOFF : stksize / 4;
  /* Initialize the thread handle */
  __pthread_init_lock(&__pthread_handles[sseg].h_lock);
  __pthread_handles[sseg].h_descr = new_thread;
  __pthread_handles[sseg].h_bottom = new_thread_bottom;
  /* Determine scheduling parameters for the thread */
  new_thread->p_start_args.schedpolicy = -1;
  if (attr != NULL) {
    new_thread->p_detached = attr->__detachstate;
    new_thread->p_userstack = attr->__stackaddr_set;

    switch(attr->__inheritsched) {
    case PTHREAD_EXPLICIT_SCHED:
      new_thread->p_start_args.schedpolicy = attr->__schedpolicy;
      memcpy (&new_thread->p_start_args.schedparam, &attr->__schedparam,
	      sizeof (struct sched_param));
      break;
    case PTHREAD_INHERIT_SCHED:
      new_thread->p_start_args.schedpolicy = __sched_getscheduler(father_pid);
      __sched_getparam(father_pid, &new_thread->p_start_args.schedparam);
      break;
    }
    new_thread->p_priority =
      new_thread->p_start_args.schedparam.sched_priority;
  }
  /* Finish setting up arguments to pthread_start_thread */
  new_thread->p_start_args.start_routine = start_routine;
  new_thread->p_start_args.arg = arg;
  new_thread->p_start_args.mask = *mask;
  /* Make the new thread ID available already now.  If any of the later
     functions fail we return an error value and the caller must not use
     the stored thread ID.  */
  *thread = new_thread_id;
  /* Raise priority of thread manager if needed */
  __pthread_manager_adjust_prio(new_thread->p_priority);
  /* Do the cloning.  We have to use two different functions depending
     on whether we are debugging or not.  */
  pid = 0;	/* Note that the thread never can have PID zero.  */
  if (report_events)
    {
      /* See whether the TD_CREATE event bit is set in any of the
         masks.  */
      int idx = __td_eventword (TD_CREATE);
      uint32_t mask = __td_eventmask (TD_CREATE);

      if ((mask & (__pthread_threads_events.event_bits[idx]
		   | event_maskp->event_bits[idx])) != 0)
	{
	  /* Lock the mutex the child will use now so that it will stop.  */
	  __pthread_lock(new_thread->p_lock, NULL);

	  /* We have to report this event.  */
#ifdef NEED_SEPARATE_REGISTER_STACK
	  /* Perhaps this version should be used on all platforms. But
	   this requires that __clone2 be uniformly supported
	   everywhere.

	   And there is some argument for changing the __clone2
	   interface to pass sp and bsp instead, making it more IA64
	   specific, but allowing stacks to grow outward from each
	   other, to get less paging and fewer mmaps.  */
	  pid = __clone2(pthread_start_thread_event,
  		 (void **)new_thread_bottom,
			 (char *)stack_addr - new_thread_bottom,
			 CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM |
			 __pthread_sig_cancel, new_thread);
#elif defined _STACK_GROWS_UP
	  pid = __clone(pthread_start_thread_event, (void *) new_thread_bottom,
			CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM |
			__pthread_sig_cancel, new_thread);
#else
	  pid = __clone(pthread_start_thread_event, stack_addr,
			CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM |
			__pthread_sig_cancel, new_thread);
#endif
	  saved_errno = errno;
	  if (pid != -1)
	    {
	      /* Now fill in the information about the new thread in
		 the newly created thread's data structure.  We cannot let
		 the new thread do this since we don't know whether it was
		 already scheduled when we send the event.  */
	      new_thread->p_eventbuf.eventdata = new_thread;
	      new_thread->p_eventbuf.eventnum = TD_CREATE;
	      __pthread_last_event = new_thread;

	      /* We have to set the PID here since the callback function
		 in the debug library will need it and we cannot guarantee
		 the child got scheduled before the debugger.  */
	      new_thread->p_pid = pid;

	      /* Now call the function which signals the event.  */
	      __linuxthreads_create_event ();

	      /* Now restart the thread.  */
	      __pthread_unlock(new_thread->p_lock);
	    }
	}
    }
  if (pid == 0)
    {
#ifdef NEED_SEPARATE_REGISTER_STACK
      pid = __clone2(pthread_start_thread,
		     (void **)new_thread_bottom,
                     (char *)stack_addr - new_thread_bottom,
		     CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM |
		     __pthread_sig_cancel, new_thread);
#elif defined _STACK_GROWS_UP
      pid = __clone(pthread_start_thread, (void *) new_thread_bottom,
		    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM |
		    __pthread_sig_cancel, new_thread);
#else
      pid = __clone(pthread_start_thread, stack_addr,
		    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM |
		    __pthread_sig_cancel, new_thread);
#endif /* !NEED_SEPARATE_REGISTER_STACK */
      saved_errno = errno;
    }
  /* Check if cloning succeeded */
  if (pid == -1) {
    /* Free the stack if we allocated it */
    if (attr == NULL || !attr->__stackaddr_set)
      {
#ifdef NEED_SEPARATE_REGISTER_STACK
	size_t stacksize = ((char *)(new_thread->p_guardaddr)
			    - new_thread_bottom);
	munmap((caddr_t)new_thread_bottom,
	       2 * stacksize + new_thread->p_guardsize);
#elif defined _STACK_GROWS_UP
# ifdef USE_TLS
	size_t stacksize = guardaddr - stack_addr;
	munmap(stack_addr, stacksize + guardsize);
# else
	size_t stacksize = guardaddr - (char *)new_thread;
	munmap(new_thread, stacksize + guardsize);
# endif
#else
# ifdef USE_TLS
	size_t stacksize = stack_addr - new_thread_bottom;
# else
	size_t stacksize = (char *)(new_thread+1) - new_thread_bottom;
# endif
	munmap(new_thread_bottom - guardsize, guardsize + stacksize);
#endif
      }
#ifdef USE_TLS
# if defined(TLS_DTV_AT_TP)
    new_thread = (pthread_descr) ((char *) new_thread + TLS_PRE_TCB_SIZE);
# endif
    _dl_deallocate_tls (new_thread, true);
#endif
    __pthread_handles[sseg].h_descr = NULL;
    __pthread_handles[sseg].h_bottom = NULL;
    __pthread_handles_num--;
    return saved_errno;
  }
  /* Insert new thread in doubly linked list of active threads */
  new_thread->p_prevlive = __pthread_main_thread;
  new_thread->p_nextlive = __pthread_main_thread->p_nextlive;
  __pthread_main_thread->p_nextlive->p_prevlive = new_thread;
  __pthread_main_thread->p_nextlive = new_thread;
  /* Set pid field of the new thread, in case we get there before the
     child starts. */
  new_thread->p_pid = pid;
  return 0;
}


/* Try to free the resources of a thread when requested by pthread_join
   or pthread_detach on a terminated thread. */

static void pthread_free(pthread_descr th)
{
  pthread_handle handle;
  pthread_readlock_info *iter, *next;

  ASSERT(th->p_exited);
  /* Make the handle invalid */
  handle =  thread_handle(th->p_tid);
  __pthread_lock(&handle->h_lock, NULL);
  handle->h_descr = NULL;
  handle->h_bottom = (char *)(-1L);
  __pthread_unlock(&handle->h_lock);
#ifdef FREE_THREAD
  FREE_THREAD(th, th->p_nr);
#endif
  /* One fewer threads in __pthread_handles */
  __pthread_handles_num--;

  /* Destroy read lock list, and list of free read lock structures.
     If the former is not empty, it means the thread exited while
     holding read locks! */

  for (iter = th->p_readlock_list; iter != NULL; iter = next)
    {
      next = iter->pr_next;
      free(iter);
    }

  for (iter = th->p_readlock_free; iter != NULL; iter = next)
    {
      next = iter->pr_next;
      free(iter);
    }

  /* If initial thread, nothing to free */
  if (!th->p_userstack)
    {
      size_t guardsize = th->p_guardsize;
      /* Free the stack and thread descriptor area */
      char *guardaddr = th->p_guardaddr;
#ifdef _STACK_GROWS_UP
# ifdef USE_TLS
      size_t stacksize = guardaddr - th->p_stackaddr;
      guardaddr = th->p_stackaddr;
# else
      size_t stacksize = guardaddr - (char *)th;
      guardaddr = (char *)th;
# endif
#else
      /* Guardaddr is always set, even if guardsize is 0.  This allows
	 us to compute everything else.  */
# ifdef USE_TLS
      size_t stacksize = th->p_stackaddr - guardaddr - guardsize;
# else
      size_t stacksize = (char *)(th+1) - guardaddr - guardsize;
# endif
# ifdef NEED_SEPARATE_REGISTER_STACK
      /* Take account of the register stack, which is below guardaddr.  */
      guardaddr -= stacksize;
      stacksize *= 2;
# endif
#endif
      /* Unmap the stack.  */
      munmap(guardaddr, stacksize + guardsize);

    }

#ifdef USE_TLS
# if defined(TLS_DTV_AT_TP)
  th = (pthread_descr) ((char *) th + TLS_PRE_TCB_SIZE);
# endif
  _dl_deallocate_tls (th, true);
#endif
}

/* Handle threads that have exited */

static void pthread_exited(pid_t pid)
{
  pthread_descr th;
  int detached;
  /* Find thread with that pid */
  for (th = __pthread_main_thread->p_nextlive;
       th != __pthread_main_thread;
       th = th->p_nextlive) {
    if (th->p_pid == pid) {
      /* Remove thread from list of active threads */
      th->p_nextlive->p_prevlive = th->p_prevlive;
      th->p_prevlive->p_nextlive = th->p_nextlive;
      /* Mark thread as exited, and if detached, free its resources */
      __pthread_lock(th->p_lock, NULL);
      th->p_exited = 1;
      /* If we have to signal this event do it now.  */
      if (th->p_report_events)
	{
	  /* See whether TD_REAP is in any of the mask.  */
	  int idx = __td_eventword (TD_REAP);
	  uint32_t mask = __td_eventmask (TD_REAP);

	  if ((mask & (__pthread_threads_events.event_bits[idx]
		       | th->p_eventbuf.eventmask.event_bits[idx])) != 0)
	    {
	      /* Yep, we have to signal the reapage.  */
	      th->p_eventbuf.eventnum = TD_REAP;
	      th->p_eventbuf.eventdata = th;
	      __pthread_last_event = th;

	      /* Now call the function to signal the event.  */
	      __linuxthreads_reap_event();
	    }
	}
      detached = th->p_detached;
      __pthread_unlock(th->p_lock);
      if (detached)
	pthread_free(th);
      break;
    }
  }
  /* If all threads have exited and the main thread is pending on a
     pthread_exit, wake up the main thread and terminate ourselves. */
  if (main_thread_exiting &&
      __pthread_main_thread->p_nextlive == __pthread_main_thread) {
    restart(__pthread_main_thread);
    /* Same logic as REQ_MAIN_THREAD_EXIT. */
  }
}

static void pthread_reap_children(void)
{
  pid_t pid;
  int status;

  while ((pid = waitpid_not_cancel(-1, &status, WNOHANG | __WCLONE)) > 0) {
    pthread_exited(pid);
    if (WIFSIGNALED(status)) {
      /* If a thread died due to a signal, send the same signal to
         all other threads, including the main thread. */
      pthread_kill_all_threads(WTERMSIG(status), 1);
      _exit(0);
    }
  }
}

/* Try to free the resources of a thread when requested by pthread_join
   or pthread_detach on a terminated thread. */

static void pthread_handle_free(pthread_t th_id)
{
  pthread_handle handle = thread_handle(th_id);
  pthread_descr th;

  __pthread_lock(&handle->h_lock, NULL);
  if (nonexisting_handle(handle, th_id)) {
    /* pthread_reap_children has deallocated the thread already,
       nothing needs to be done */
    __pthread_unlock(&handle->h_lock);
    return;
  }
  th = handle->h_descr;
  if (th->p_exited) {
    __pthread_unlock(&handle->h_lock);
    pthread_free(th);
  } else {
    /* The Unix process of the thread is still running.
       Mark the thread as detached so that the thread manager will
       deallocate its resources when the Unix process exits. */
    th->p_detached = 1;
    __pthread_unlock(&handle->h_lock);
  }
}

/* Send a signal to all running threads */

static void pthread_kill_all_threads(int sig, int main_thread_also)
{
  pthread_descr th;
  for (th = __pthread_main_thread->p_nextlive;
       th != __pthread_main_thread;
       th = th->p_nextlive) {
    kill(th->p_pid, sig);
  }
  if (main_thread_also) {
    kill(__pthread_main_thread->p_pid, sig);
  }
}

static void pthread_for_each_thread(void *arg,
    void (*fn)(void *, pthread_descr))
{
  pthread_descr th;

  for (th = __pthread_main_thread->p_nextlive;
       th != __pthread_main_thread;
       th = th->p_nextlive) {
    fn(arg, th);
  }

  fn(arg, __pthread_main_thread);
}

/* Process-wide exit() */

static void pthread_handle_exit(pthread_descr issuing_thread, int exitcode)
{
  pthread_descr th;
  __pthread_exit_requested = 1;
  __pthread_exit_code = exitcode;
  /* A forced asynchronous cancellation follows.  Make sure we won't
     get stuck later in the main thread with a system lock being held
     by one of the cancelled threads.  Ideally one would use the same
     code as in pthread_atfork(), but we can't distinguish system and
     user handlers there.  */
  __flockfilelist();
  /* Send the CANCEL signal to all running threads, including the main
     thread, but excluding the thread from which the exit request originated
     (that thread must complete the exit, e.g. calling atexit functions
     and flushing stdio buffers). */
  for (th = issuing_thread->p_nextlive;
       th != issuing_thread;
       th = th->p_nextlive) {
    kill(th->p_pid, __pthread_sig_cancel);
  }
  /* Now, wait for all these threads, so that they don't become zombies
     and their times are properly added to the thread manager's times. */
  for (th = issuing_thread->p_nextlive;
       th != issuing_thread;
       th = th->p_nextlive) {
    waitpid(th->p_pid, NULL, __WCLONE);
  }
  __fresetlockfiles();
  restart(issuing_thread);
  _exit(0);
}

/* Handler for __pthread_sig_cancel in thread manager thread */

void __pthread_manager_sighandler(int sig)
{
  int kick_manager = terminated_children == 0 && main_thread_exiting;
  terminated_children = 1;

  /* If the main thread is terminating, kick the thread manager loop
     each time some threads terminate. This eliminates a two second
     shutdown delay caused by the thread manager sleeping in the
     call to __poll(). Instead, the thread manager is kicked into
     action, reaps the outstanding threads and resumes the main thread
     so that it can complete the shutdown. */

  if (kick_manager) {
    struct pthread_request request;
    request.req_thread = 0;
    request.req_kind = REQ_KICK;
    TEMP_FAILURE_RETRY(write_not_cancel(__pthread_manager_request,
					(char *) &request, sizeof(request)));
  }
}

/* Adjust priority of thread manager so that it always run at a priority
   higher than all threads */

void __pthread_manager_adjust_prio(int thread_prio)
{
  struct sched_param param;

  if (thread_prio <= manager_thread->p_priority) return;
  param.sched_priority =
    thread_prio < __sched_get_priority_max(SCHED_FIFO)
    ? thread_prio + 1 : thread_prio;
  __sched_setscheduler(manager_thread->p_pid, SCHED_FIFO, &param);
  manager_thread->p_priority = thread_prio;
}
