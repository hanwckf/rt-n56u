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

#include <features.h>
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

#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#include "semaphore.h"
#include "debug.h" /* PDEBUG, added by StS */

#ifndef THREAD_STACK_OFFSET
#define THREAD_STACK_OFFSET 0
#endif

/* poll() is not supported in kernel <= 2.0, therefore is __NR_poll is
 * not available, we assume an old Linux kernel is in use and we will
 * use select() instead. */
#include <sys/syscall.h>
#ifndef __NR_poll
# define USE_SELECT
#endif

/* Array of active threads. Entry 0 is reserved for the initial thread. */
struct pthread_handle_struct __pthread_handles[PTHREAD_THREADS_MAX] =
{ { __LOCK_INITIALIZER, &__pthread_initial_thread, 0},
  { __LOCK_INITIALIZER, &__pthread_manager_thread, 0}, /* All NULLs */ };

/* For debugging purposes put the maximum number of threads in a variable.  */
const int __linuxthreads_pthread_threads_max = PTHREAD_THREADS_MAX;

/* Indicate whether at least one thread has a user-defined stack (if 1),
   or if all threads have stacks supplied by LinuxThreads (if 0). */
int __pthread_nonstandard_stacks;

/* Number of active entries in __pthread_handles (used by gdb) */
volatile int __pthread_handles_num = 2;

/* Whether to use debugger additional actions for thread creation
   (set to 1 by gdb) */
volatile int __pthread_threads_debug;

/* Globally enabled events.  */
volatile td_thr_events_t __pthread_threads_events;

/* Pointer to thread descriptor with last event.  */
volatile pthread_descr __pthread_last_event;

/* Mapping from stack segment to thread descriptor. */
/* Stack segment numbers are also indices into the __pthread_handles array. */
/* Stack segment number 0 is reserved for the initial thread. */

static __inline__ pthread_descr thread_segment(int seg)
{
  return (pthread_descr)(THREAD_STACK_START_ADDRESS - (seg - 1) * STACK_SIZE)
         - 1;
}

/* Flag set in signal handler to record child termination */

static volatile int terminated_children = 0;

/* Flag set when the initial thread is blocked on pthread_exit waiting
   for all other threads to terminate */

static int main_thread_exiting = 0;

/* Counter used to generate unique thread identifier.
   Thread identifier is pthread_threads_counter + segment. */

static pthread_t pthread_threads_counter = 0;

/* Forward declarations */

static int pthread_handle_create(pthread_t *thread, const pthread_attr_t *attr,
                                 void * (*start_routine)(void *), void *arg,
                                 sigset_t *mask, int father_pid,
				 int report_events,
				 td_thr_events_t *event_maskp);
static void pthread_handle_free(pthread_t th_id);
static void pthread_handle_exit(pthread_descr issuing_thread, int exitcode) attribute_noreturn;
static void pthread_reap_children(void);
static void pthread_kill_all_threads(int sig, int main_thread_also);

/* The server thread managing requests for thread creation and termination */

int attribute_noreturn __pthread_manager(void *arg)
{
  int reqfd = (int) (long int) arg;
#ifdef USE_SELECT
  struct timeval tv;
  fd_set fd;
#else
  struct pollfd ufd;
#endif
  sigset_t manager_mask;
  int n;
  struct pthread_request request;

  /* If we have special thread_self processing, initialize it.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(&__pthread_manager_thread, 1);
#endif
  /* Set the error variable.  */
  __pthread_manager_thread.p_errnop = &__pthread_manager_thread.p_errno;
  __pthread_manager_thread.p_h_errnop = &__pthread_manager_thread.p_h_errno;

#ifdef __UCLIBC_HAS_XLOCALE__
  /* Initialize thread's locale to the global locale. */
  __pthread_manager_thread.locale = __global_locale;
#endif /* __UCLIBC_HAS_XLOCALE__ */

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
  n = TEMP_FAILURE_RETRY(read(reqfd, (char *)&request,
				     sizeof(request)));
#ifndef USE_SELECT
  ufd.fd = reqfd;
  ufd.events = POLLIN;
#endif
  /* Enter server loop */
  while(1) {
#ifdef USE_SELECT
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    FD_ZERO (&fd);
    FD_SET (reqfd, &fd);
    n = select (reqfd + 1, &fd, NULL, NULL, &tv);
#else
    PDEBUG("before poll\n");
    n = poll(&ufd, 1, 2000);
    PDEBUG("after poll\n");
#endif
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
#ifdef USE_SELECT
    if (n == 1)
#else
    if (n == 1 && (ufd.revents & POLLIN))
#endif
    {

      PDEBUG("before read\n");
      n = read(reqfd, (char *)&request, sizeof(request));
      PDEBUG("after read, n=%d\n", n);
      switch(request.req_kind) {
      case REQ_CREATE:
        PDEBUG("got REQ_CREATE\n");
        request.req_thread->p_retcode =
          pthread_handle_create((pthread_t *) &request.req_thread->p_retval,
                                request.req_args.create.attr,
                                request.req_args.create.fn,
                                request.req_args.create.arg,
                                &request.req_args.create.mask,
                                request.req_thread->p_pid,
                                request.req_thread->p_report_events,
                                &request.req_thread->p_eventbuf.eventmask);
        PDEBUG("restarting %p\n", request.req_thread);
        restart(request.req_thread);
        break;
      case REQ_FREE:
        PDEBUG("got REQ_FREE\n");
        pthread_handle_free(request.req_args.free.thread_id);
        break;
      case REQ_PROCESS_EXIT:
        PDEBUG("got REQ_PROCESS_EXIT from %p, exit code = %d\n",
        request.req_thread, request.req_args.exit.code);
        pthread_handle_exit(request.req_thread,
                            request.req_args.exit.code);
        break;
      case REQ_MAIN_THREAD_EXIT:
        PDEBUG("got REQ_MAIN_THREAD_EXIT\n");
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
        PDEBUG("got REQ_POST\n");
        sem_post(request.req_args.post);
        break;
      case REQ_DEBUG:
        PDEBUG("got REQ_DEBUG\n");
	/* Make gdb aware of new thread and gdb will restart the
	   new thread when it is ready to handle the new thread. */
	if (__pthread_threads_debug && __pthread_sig_debug > 0) {
	  PDEBUG("about to call raise(__pthread_sig_debug)\n");
	  raise(__pthread_sig_debug);
	}
      case REQ_KICK:
	/* This is just a prod to get the manager to reap some
	   threads right away, avoiding a potential delay at shutdown. */
	break;
      }
    }
  }
}

int attribute_noreturn __pthread_manager_event(void *arg)
{
  /* If we have special thread_self processing, initialize it.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(&__pthread_manager_thread, 1);
#endif

  /* Get the lock the manager will free once all is correctly set up.  */
  __pthread_lock (THREAD_GETMEM((&__pthread_manager_thread), p_lock), NULL);
  /* Free it immediately.  */
  __pthread_unlock (THREAD_GETMEM((&__pthread_manager_thread), p_lock));

  __pthread_manager(arg);
}

/* Process creation */
static int
attribute_noreturn
pthread_start_thread(void *arg)
{
  pthread_descr self = (pthread_descr) arg;
  struct pthread_request request;
  void * outcome;
  /* Initialize special thread_self processing, if any.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, self->p_nr);
#endif
  PDEBUG("\n");
  /* Make sure our pid field is initialized, just in case we get there
     before our father has initialized it. */
  THREAD_SETMEM(self, p_pid, getpid());
  /* Initial signal mask is that of the creating thread. (Otherwise,
     we'd just inherit the mask of the thread manager.) */
  sigprocmask(SIG_SETMASK, &self->p_start_args.mask, NULL);
  /* Set the scheduling policy and priority for the new thread, if needed */
  if (THREAD_GETMEM(self, p_start_args.schedpolicy) >= 0)
    /* Explicit scheduling attributes were provided: apply them */
    sched_setscheduler(THREAD_GETMEM(self, p_pid),
			 THREAD_GETMEM(self, p_start_args.schedpolicy),
                         &self->p_start_args.schedparam);
  else if (__pthread_manager_thread.p_priority > 0)
    /* Default scheduling required, but thread manager runs in realtime
       scheduling: switch new thread to SCHED_OTHER policy */
    {
      struct sched_param default_params;
      default_params.sched_priority = 0;
      sched_setscheduler(THREAD_GETMEM(self, p_pid),
                           SCHED_OTHER, &default_params);
    }
  /* Make gdb aware of new thread */
  if (__pthread_threads_debug && __pthread_sig_debug > 0) {
    request.req_thread = self;
    request.req_kind = REQ_DEBUG;
    TEMP_FAILURE_RETRY(write(__pthread_manager_request,
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
attribute_noreturn
pthread_start_thread_event(void *arg)
{
  pthread_descr self = (pthread_descr) arg;

#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, self->p_nr);
#endif
  /* Make sure our pid field is initialized, just in case we get there
     before our father has initialized it. */
  THREAD_SETMEM(self, p_pid, getpid());
  /* Get the lock the manager will free once all is correctly set up.  */
  __pthread_lock (THREAD_GETMEM(self, p_lock), NULL);
  /* Free it immediately.  */
  __pthread_unlock (THREAD_GETMEM(self, p_lock));

  /* Continue with the real function.  */
  pthread_start_thread (arg);
}

static int pthread_allocate_stack(const pthread_attr_t *attr,
                                  pthread_descr default_new_thread,
                                  int pagesize,
                                  pthread_descr * out_new_thread,
                                  char ** out_new_thread_bottom,
                                  char ** out_guardaddr,
                                  size_t * out_guardsize)
{
  pthread_descr new_thread;
  char * new_thread_bottom;
  char * guardaddr;
  size_t stacksize, guardsize;

  if (attr != NULL && attr->__stackaddr_set)
    {
      /* The user provided a stack. */
      new_thread = (pthread_descr) ((long)(attr->__stackaddr) & -sizeof(void *)) - 1;
      new_thread_bottom = (char *) attr->__stackaddr - attr->__stacksize;
      guardaddr = NULL;
      guardsize = 0;
      __pthread_nonstandard_stacks = 1;
#ifndef __ARCH_USE_MMU__
      /* check the initial thread stack boundaries so they don't overlap */
      NOMMU_INITIAL_THREAD_BOUNDS((char *) new_thread, (char *) new_thread_bottom);

      PDEBUG("initial stack: bos=%p, tos=%p\n", __pthread_initial_thread_bos,
            __pthread_initial_thread_tos);
#endif
    }
  else
    {
#ifdef __ARCH_USE_MMU__
      stacksize = STACK_SIZE - pagesize;
      if (attr != NULL)
        stacksize = MIN(stacksize, roundup(attr->__stacksize, pagesize));
      /* Allocate space for stack and thread descriptor at default address */
      new_thread = default_new_thread;
      new_thread_bottom = (char *) (new_thread + 1) - stacksize;
      if (mmap((caddr_t)((char *)(new_thread + 1) - INITIAL_STACK_SIZE),
               INITIAL_STACK_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
               MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED | MAP_GROWSDOWN,
               -1, 0) == MAP_FAILED)
        /* Bad luck, this segment is already mapped. */
        return -1;
      /* We manage to get a stack.  Now see whether we need a guard
         and allocate it if necessary.  Notice that the default
         attributes (stack_size = STACK_SIZE - pagesize) do not need
	 a guard page, since the RLIMIT_STACK soft limit prevents stacks
	 from running into one another. */
      if (stacksize == (size_t) (STACK_SIZE - pagesize))
        {
          /* We don't need a guard page. */
          guardaddr = NULL;
          guardsize = 0;
        }
      else
        {
          /* Put a bad page at the bottom of the stack */
          guardsize = attr->__guardsize;
          guardaddr = (void *)new_thread_bottom - guardsize;
          if (mmap((caddr_t) guardaddr, guardsize, 0, MAP_FIXED, -1, 0)
              == MAP_FAILED)
            {
              /* We don't make this an error.  */
              guardaddr = NULL;
              guardsize = 0;
            }
        }
#else
      /* We cannot mmap to this huge chunk of stack space when we don't have
       * an MMU. Pretend we are using a user provided stack even if there was
       * none provided by the user. Thus, we get around the mmap and reservation
       * of a huge stack segment. -StS */

      stacksize = INITIAL_STACK_SIZE;
      /* The user may want to use a non-default stacksize */
      if (attr != NULL)
	{
	  stacksize = attr->__stacksize;
	}

      /* malloc a stack - memory from the bottom up */
      if ((new_thread_bottom = malloc(stacksize)) == NULL)
	{
	  /* bad luck, we cannot malloc any more */
	  return -1 ;
	}
      PDEBUG("malloced chunk: base=%p, size=0x%04x\n", new_thread_bottom, stacksize);

      /* Set up the pointers. new_thread marks the TOP of the stack frame and
       * the address of the pthread_descr struct at the same time. Therefore we
       * must account for its size and fit it in the malloc()'ed block. The
       * value of `new_thread' is then passed to clone() as the stack argument.
       *
       *               ^ +------------------------+
       *               | |  pthread_descr struct  |
       *               | +------------------------+  <- new_thread
       * malloc block  | |                        |
       *               | |  thread stack          |
       *               | |                        |
       *               v +------------------------+  <- new_thread_bottom
       *
       * Note: The calculated value of new_thread must be word aligned otherwise
       * the kernel chokes on a non-aligned stack frame. Choose the lower
       * available word boundary.
       */
      new_thread = ((pthread_descr) ((int)(new_thread_bottom + stacksize) & -sizeof(void*))) - 1;
      guardaddr = NULL;
      guardsize = 0;

      PDEBUG("thread stack: bos=%p, tos=%p\n", new_thread_bottom, new_thread);

      /* check the initial thread stack boundaries so they don't overlap */
      NOMMU_INITIAL_THREAD_BOUNDS((char *) new_thread, (char *) new_thread_bottom);

      PDEBUG("initial stack: bos=%p, tos=%p\n", __pthread_initial_thread_bos,
	     __pthread_initial_thread_tos);

      /* on non-MMU systems we always have non-standard stack frames */
      __pthread_nonstandard_stacks = 1;

#endif /* __ARCH_USE_MMU__ */
    }

  /* Clear the thread data structure.  */
  memset (new_thread, '\0', sizeof (*new_thread));
  *out_new_thread = new_thread;
  *out_new_thread_bottom = new_thread_bottom;
  *out_guardaddr = guardaddr;
  *out_guardsize = guardsize;
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
  char * new_thread_bottom;
  char * new_thread_top;
  pthread_t new_thread_id;
  char *guardaddr = NULL;
  size_t guardsize = 0;
  int pagesize = getpagesize();
  int saved_errno = 0;

  /* First check whether we have to change the policy and if yes, whether
     we can  do this.  Normally this should be done by examining the
     return value of the sched_setscheduler call in pthread_start_thread
     but this is hard to implement.  FIXME  */
  if (attr != NULL && attr->__schedpolicy != SCHED_OTHER && geteuid () != 0)
    return EPERM;
  /* Find a free segment for the thread, and allocate a stack if needed */
  for (sseg = 2; ; sseg++)
    {
      if (sseg >= PTHREAD_THREADS_MAX)
	return EAGAIN;
      if (__pthread_handles[sseg].h_descr != NULL)
	continue;
      if (pthread_allocate_stack(attr, thread_segment(sseg), pagesize,
                                 &new_thread, &new_thread_bottom,
                                 &guardaddr, &guardsize) == 0)
        break;
#ifndef __ARCH_USE_MMU__
      else
        /* When there is MMU, mmap () is used to allocate the stack. If one
         * segment is already mapped, we should continue to see if we can
         * use the next one. However, when there is no MMU, malloc () is used.
         * It's waste of CPU cycles to continue to try if it fails.  */
        return EAGAIN;
#endif
    }
  __pthread_handles_num++;
  /* Allocate new thread identifier */
  pthread_threads_counter += PTHREAD_THREADS_MAX;
  new_thread_id = sseg + pthread_threads_counter;
  /* Initialize the thread descriptor.  Elements which have to be
     initialized to zero already have this value.  */
  new_thread->p_tid = new_thread_id;
  new_thread->p_lock = &(__pthread_handles[sseg].h_lock);
  new_thread->p_cancelstate = PTHREAD_CANCEL_ENABLE;
  new_thread->p_canceltype = PTHREAD_CANCEL_DEFERRED;
  new_thread->p_errnop = &new_thread->p_errno;
  new_thread->p_h_errnop = &new_thread->p_h_errno;
#ifdef __UCLIBC_HAS_XLOCALE__
  /* Initialize thread's locale to the global locale. */
  new_thread->locale = __global_locale;
#endif /* __UCLIBC_HAS_XLOCALE__ */
  new_thread->p_guardaddr = guardaddr;
  new_thread->p_guardsize = guardsize;
  new_thread->p_self = new_thread;
  new_thread->p_nr = sseg;
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
      new_thread->p_start_args.schedpolicy = sched_getscheduler(father_pid);
      sched_getparam(father_pid, &new_thread->p_start_args.schedparam);
      break;
    }
    new_thread->p_priority =
      new_thread->p_start_args.schedparam.sched_priority;
  }
  /* Finish setting up arguments to pthread_start_thread */
  new_thread->p_start_args.start_routine = start_routine;
  new_thread->p_start_args.arg = arg;
  new_thread->p_start_args.mask = *mask;
  /* Raise priority of thread manager if needed */
  __pthread_manager_adjust_prio(new_thread->p_priority);
  /* Do the cloning.  We have to use two different functions depending
     on whether we are debugging or not.  */
  pid = 0;     /* Note that the thread never can have PID zero.  */
  new_thread_top = ((char *)new_thread - THREAD_STACK_OFFSET);

  /* ******************************************************** */
  /*  This code was moved from below to cope with running threads
   *  on uClinux systems.  See comment below...
   * Insert new thread in doubly linked list of active threads */
  new_thread->p_prevlive = __pthread_main_thread;
  new_thread->p_nextlive = __pthread_main_thread->p_nextlive;
  __pthread_main_thread->p_nextlive->p_prevlive = new_thread;
  __pthread_main_thread->p_nextlive = new_thread;
  /* ********************************************************* */

  if (report_events)
    {
      /* See whether the TD_CREATE event bit is set in any of the
         masks.  */
      int idx = __td_eventword (TD_CREATE);
      uint32_t m = __td_eventmask (TD_CREATE);

      if ((m & (__pthread_threads_events.event_bits[idx]
		   | event_maskp->event_bits[idx])) != 0)
	{
	  /* Lock the mutex the child will use now so that it will stop.  */
	  __pthread_lock(new_thread->p_lock, NULL);

	  /* We have to report this event.  */
#ifdef __ia64__
	  pid = __clone2(pthread_start_thread_event, new_thread_top,
			new_thread_top - new_thread_bottom,
			CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
			__pthread_sig_cancel, new_thread);
#else
	  pid = clone(pthread_start_thread_event, new_thread_top,
			CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
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
      PDEBUG("cloning new_thread = %p\n", new_thread);
#ifdef __ia64__
      pid = __clone2(pthread_start_thread, new_thread_top,
		    new_thread_top - new_thread_bottom,
		    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
		    __pthread_sig_cancel, new_thread);
#else
      pid = clone(pthread_start_thread, new_thread_top,
		    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
		    __pthread_sig_cancel, new_thread);
#endif
      saved_errno = errno;
    }
  /* Check if cloning succeeded */
  if (pid == -1) {
    /********************************************************
     * Code inserted to remove the thread from our list of active
     * threads in case of failure (needed to cope with uClinux),
     * See comment below. */
    new_thread->p_nextlive->p_prevlive = new_thread->p_prevlive;
    new_thread->p_prevlive->p_nextlive = new_thread->p_nextlive;
    /********************************************************/

    /* Free the stack if we allocated it */
    if (attr == NULL || !attr->__stackaddr_set)
      {
#ifdef __ARCH_USE_MMU__
	if (new_thread->p_guardsize != 0)
	  munmap(new_thread->p_guardaddr, new_thread->p_guardsize);
	munmap((caddr_t)((char *)(new_thread+1) - INITIAL_STACK_SIZE),
	       INITIAL_STACK_SIZE);
#else
	free(new_thread_bottom);
#endif /* __ARCH_USE_MMU__ */
      }
    __pthread_handles[sseg].h_descr = NULL;
    __pthread_handles[sseg].h_bottom = NULL;
    __pthread_handles_num--;
    return saved_errno;
  }
  PDEBUG("new thread pid = %d\n", pid);

#if 0
  /* ***********************************************************
   This code has been moved before the call to clone().  In uClinux,
   the use of wait on a semaphore is dependant upon that the child so
   the child must be in the active threads list. This list is used in
   pthread_find_self() to get the pthread_descr of self. So, if the
   child calls sem_wait before this code is executed , it will hang
   forever and initial_thread will instead be posted by a sem_post
   call. */

  /* Insert new thread in doubly linked list of active threads */
  new_thread->p_prevlive = __pthread_main_thread;
  new_thread->p_nextlive = __pthread_main_thread->p_nextlive;
  __pthread_main_thread->p_nextlive->p_prevlive = new_thread;
  __pthread_main_thread->p_nextlive = new_thread;
  /************************************************************/
#endif

  /* Set pid field of the new thread, in case we get there before the
     child starts. */
  new_thread->p_pid = pid;
  /* We're all set */
  *thread = new_thread_id;
  return 0;
}


/* Try to free the resources of a thread when requested by pthread_join
   or pthread_detach on a terminated thread. */

static void pthread_free(pthread_descr th)
{
  pthread_handle handle;
  pthread_readlock_info *iter, *next;
#ifndef __ARCH_USE_MMU__
  char *h_bottom_save;
#endif

  /* Make the handle invalid */
  handle =  thread_handle(th->p_tid);
  __pthread_lock(&handle->h_lock, NULL);
#ifndef __ARCH_USE_MMU__
  h_bottom_save = handle->h_bottom;
#endif
  handle->h_descr = NULL;
  handle->h_bottom = (char *)(-1L);
  __pthread_unlock(&handle->h_lock);
#ifdef FREE_THREAD_SELF
  FREE_THREAD_SELF(th, th->p_nr);
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
  if (th == &__pthread_initial_thread) return;
  if (!th->p_userstack)
    {
#ifdef __ARCH_USE_MMU__
      /* Free the stack and thread descriptor area */
      if (th->p_guardsize != 0)
	munmap(th->p_guardaddr, th->p_guardsize);
      munmap((caddr_t) ((char *)(th+1) - STACK_SIZE), STACK_SIZE);
#else
      /* For non-MMU systems we always malloc the stack, so free it here. -StS */
      free(h_bottom_save);
#endif /* __ARCH_USE_MMU__ */
    }
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
  PDEBUG("\n");

  while ((pid = waitpid(-1, &status, WNOHANG | __WCLONE)) > 0) {
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
  if (invalid_handle(handle, th_id)) {
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

/* Process-wide exit() */

static void pthread_handle_exit(pthread_descr issuing_thread, int exitcode)
{
  pthread_descr th;
  __pthread_exit_requested = 1;
  __pthread_exit_code = exitcode;
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
  restart(issuing_thread);
  _exit(0);
}

/* Handler for __pthread_sig_cancel in thread manager thread */

void __pthread_manager_sighandler(int sig attribute_unused)
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
	TEMP_FAILURE_RETRY(write(__pthread_manager_request,
		    (char *) &request, sizeof(request)));
    }
}

/* Adjust priority of thread manager so that it always run at a priority
   higher than all threads */

void __pthread_manager_adjust_prio(int thread_prio)
{
  struct sched_param param;

  if (thread_prio <= __pthread_manager_thread.p_priority) return;
  param.sched_priority =
    thread_prio < sched_get_priority_max(SCHED_FIFO)
    ? thread_prio + 1 : thread_prio;
  sched_setscheduler(__pthread_manager_thread.p_pid, SCHED_FIFO, &param);
  __pthread_manager_thread.p_priority = thread_prio;
}
