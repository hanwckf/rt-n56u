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

/* Thread creation, initialization, and basic low-level routines */

#define __FORCE_GLIBC
#include <features.h>
#define __USE_GNU
#include <errno.h>
#include <netdb.h>	/* for h_errno */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#include "debug.h"      /* added to linuxthreads -StS */


/* Mods for uClibc: Some includes */
#include <signal.h>
#include <sys/types.h>
#include <sys/syscall.h>

/* mods for uClibc: getpwd and getpagesize are the syscalls */
#define __getpid getpid
#define __getpagesize getpagesize
/* mods for uClibc: __libc_sigaction is not in any standard headers */
extern int __libc_sigaction (int sig, const struct sigaction *act, struct sigaction *oact);


/* These variables are used by the setup code.  */
extern int _errno;
extern int _h_errno;


/* Descriptor of the initial thread */

struct _pthread_descr_struct __pthread_initial_thread = {
  &__pthread_initial_thread,  /* pthread_descr p_nextlive */
  &__pthread_initial_thread,  /* pthread_descr p_prevlive */
  NULL,                       /* pthread_descr p_nextwaiting */
  NULL,			      /* pthread_descr p_nextlock */
  PTHREAD_THREADS_MAX,        /* pthread_t p_tid */
  0,                          /* int p_pid */
  0,                          /* int p_priority */
  &__pthread_handles[0].h_lock, /* struct _pthread_fastlock * p_lock */
  0,                          /* int p_signal */
  NULL,                       /* sigjmp_buf * p_signal_buf */
  NULL,                       /* sigjmp_buf * p_cancel_buf */
  0,                          /* char p_terminated */
  0,                          /* char p_detached */
  0,                          /* char p_exited */
  NULL,                       /* void * p_retval */
  0,                          /* int p_retval */
  NULL,                       /* pthread_descr p_joining */
  NULL,                       /* struct _pthread_cleanup_buffer * p_cleanup */
  0,                          /* char p_cancelstate */
  0,                          /* char p_canceltype */
  0,                          /* char p_canceled */
  &_errno,                       /* int *p_errnop */
  0,                          /* int p_errno */
  &_h_errno,                       /* int *p_h_errnop */
  0,                          /* int p_h_errno */
  NULL,                       /* char * p_in_sighandler */
  0,                          /* char p_sigwaiting */
  PTHREAD_START_ARGS_INITIALIZER, /* struct pthread_start_args p_start_args */
  {NULL},                     /* void ** p_specific[PTHREAD_KEY_1STLEVEL_SIZE] */
  {NULL},                     /* void * p_libc_specific[_LIBC_TSD_KEY_N] */
  0,                          /* int p_userstack */
  NULL,                       /* void * p_guardaddr */
  0,                          /* size_t p_guardsize */
  &__pthread_initial_thread,  /* pthread_descr p_self */
  0,                          /* Always index 0 */
  0,                          /* int p_report_events */
  {{{0, }}, 0, NULL},         /* td_eventbuf_t p_eventbuf */
  __ATOMIC_INITIALIZER,         /* struct pthread_atomic p_resume_count */
  0,                          /* char p_woken_by_cancel */
  0,                          /* char p_condvar_avail */
  0,                          /* char p_sem_avail */
  NULL,                       /* struct pthread_extricate_if *p_extricate */
  NULL,	                      /* pthread_readlock_info *p_readlock_list; */
  NULL,                       /* pthread_readlock_info *p_readlock_free; */
  0                           /* int p_untracked_readlock_count; */
#ifdef __UCLIBC_HAS_XLOCALE__
  ,
  &__global_locale_data,      /* __locale_t locale; */
#endif /* __UCLIBC_HAS_XLOCALE__ */
};

/* Descriptor of the manager thread; none of this is used but the error
   variables, the p_pid and p_priority fields,
   and the address for identification.  */
#define manager_thread (&__pthread_manager_thread)
struct _pthread_descr_struct __pthread_manager_thread = {
  NULL,                       /* pthread_descr p_nextlive */
  NULL,                       /* pthread_descr p_prevlive */
  NULL,                       /* pthread_descr p_nextwaiting */
  NULL,			      /* pthread_descr p_nextlock */
  0,                          /* int p_tid */
  0,                          /* int p_pid */
  0,                          /* int p_priority */
  &__pthread_handles[1].h_lock, /* struct _pthread_fastlock * p_lock */
  0,                          /* int p_signal */
  NULL,                       /* sigjmp_buf * p_signal_buf */
  NULL,                       /* sigjmp_buf * p_cancel_buf */
  0,                          /* char p_terminated */
  0,                          /* char p_detached */
  0,                          /* char p_exited */
  NULL,                       /* void * p_retval */
  0,                          /* int p_retval */
  NULL,                       /* pthread_descr p_joining */
  NULL,                       /* struct _pthread_cleanup_buffer * p_cleanup */
  0,                          /* char p_cancelstate */
  0,                          /* char p_canceltype */
  0,                          /* char p_canceled */
  &__pthread_manager_thread.p_errno, /* int *p_errnop */
  0,                          /* int p_errno */
  NULL,                       /* int *p_h_errnop */
  0,                          /* int p_h_errno */
  NULL,                       /* char * p_in_sighandler */
  0,                          /* char p_sigwaiting */
  PTHREAD_START_ARGS_INITIALIZER, /* struct pthread_start_args p_start_args */
  {NULL},                     /* void ** p_specific[PTHREAD_KEY_1STLEVEL_SIZE] */
  {NULL},                     /* void * p_libc_specific[_LIBC_TSD_KEY_N] */
  0,                          /* int p_userstack */
  NULL,                       /* void * p_guardaddr */
  0,                          /* size_t p_guardsize */
  &__pthread_manager_thread,  /* pthread_descr p_self */
  1,                          /* Always index 1 */
  0,                          /* int p_report_events */
  {{{0, }}, 0, NULL},         /* td_eventbuf_t p_eventbuf */
  __ATOMIC_INITIALIZER,         /* struct pthread_atomic p_resume_count */
  0,                          /* char p_woken_by_cancel */
  0,                          /* char p_condvar_avail */
  0,                          /* char p_sem_avail */
  NULL,                       /* struct pthread_extricate_if *p_extricate */
  NULL,	                      /* pthread_readlock_info *p_readlock_list; */
  NULL,                       /* pthread_readlock_info *p_readlock_free; */
  0                           /* int p_untracked_readlock_count; */
#ifdef __UCLIBC_HAS_XLOCALE__
  ,
  &__global_locale_data,      /* __locale_t locale; */
#endif /* __UCLIBC_HAS_XLOCALE__ */
};

/* Pointer to the main thread (the father of the thread manager thread) */
/* Originally, this is the initial thread, but this changes after fork() */

pthread_descr __pthread_main_thread = &__pthread_initial_thread;

/* Limit between the stack of the initial thread (above) and the
   stacks of other threads (below). Aligned on a STACK_SIZE boundary. */

char *__pthread_initial_thread_bos = NULL;

/* For non-MMU systems also remember to stack top of the initial thread.
 * This is adapted when other stacks are malloc'ed since we don't know
 * the bounds a-priori. -StS */

#ifndef __ARCH_HAS_MMU__
char *__pthread_initial_thread_tos = NULL;
#endif /* __ARCH_HAS_MMU__ */

/* File descriptor for sending requests to the thread manager. */
/* Initially -1, meaning that the thread manager is not running. */

int __pthread_manager_request = -1;

/* Other end of the pipe for sending requests to the thread manager. */

int __pthread_manager_reader;

/* Limits of the thread manager stack */

char *__pthread_manager_thread_bos = NULL;
char *__pthread_manager_thread_tos = NULL;

/* For process-wide exit() */

int __pthread_exit_requested = 0;
int __pthread_exit_code = 0;

/* Communicate relevant LinuxThreads constants to gdb */

const int __pthread_threads_max = PTHREAD_THREADS_MAX;
const int __pthread_sizeof_handle = sizeof(struct pthread_handle_struct);
const int __pthread_offsetof_descr = offsetof(struct pthread_handle_struct, h_descr);
const int __pthread_offsetof_pid = offsetof(struct _pthread_descr_struct,
                                            p_pid);
const int __linuxthreads_pthread_sizeof_descr
  = sizeof(struct _pthread_descr_struct);

const int __linuxthreads_initial_report_events;

const char __linuxthreads_version[] = VERSION;

/* Forward declarations */
static void pthread_onexit_process(int retcode, void *arg);
static void pthread_handle_sigcancel(int sig);
static void pthread_handle_sigrestart(int sig);
static void pthread_handle_sigdebug(int sig);
int __pthread_timedsuspend_new(pthread_descr self, const struct timespec *abstime);

/* Signal numbers used for the communication.
   In these variables we keep track of the used variables.  If the
   platform does not support any real-time signals we will define the
   values to some unreasonable value which will signal failing of all
   the functions below.  */
#ifndef __NR_rt_sigaction
static int current_rtmin = -1;
static int current_rtmax = -1;
int __pthread_sig_restart = SIGUSR1;
int __pthread_sig_cancel = SIGUSR2;
int __pthread_sig_debug;
#else

#if __SIGRTMAX - __SIGRTMIN >= 3
static int current_rtmin = __SIGRTMIN + 3;
static int current_rtmax = __SIGRTMAX;
int __pthread_sig_restart = __SIGRTMIN;
int __pthread_sig_cancel = __SIGRTMIN + 1;
int __pthread_sig_debug = __SIGRTMIN + 2;
void (*__pthread_restart)(pthread_descr) = __pthread_restart_new;
void (*__pthread_suspend)(pthread_descr) = __pthread_wait_for_restart_signal;
int (*__pthread_timedsuspend)(pthread_descr, const struct timespec *) = __pthread_timedsuspend_new;
#else
static int current_rtmin = __SIGRTMIN;
static int current_rtmax = __SIGRTMAX;
int __pthread_sig_restart = SIGUSR1;
int __pthread_sig_cancel = SIGUSR2;
int __pthread_sig_debug;
void (*__pthread_restart)(pthread_descr) = __pthread_restart_old;
void (*__pthread_suspend)(pthread_descr) = __pthread_suspend_old;
int (*__pthread_timedsuspend)(pthread_descr, const struct timespec *) = __pthread_timedsuspend_old;

#endif

/* Return number of available real-time signal with highest priority.  */
int __libc_current_sigrtmin (void)
{
    return current_rtmin;
}

/* Return number of available real-time signal with lowest priority.  */
int __libc_current_sigrtmax (void)
{
    return current_rtmax;
}

/* Allocate real-time signal with highest/lowest available
   priority.  Please note that we don't use a lock since we assume
   this function to be called at program start.  */
int __libc_allocate_rtsig (int high)
{
    if (current_rtmin == -1 || current_rtmin > current_rtmax)
	/* We don't have anymore signal available.  */
	return -1;
    return high ? current_rtmin++ : current_rtmax--;
}
#endif

/* Initialize the pthread library.
   Initialization is split in two functions:
   - a constructor function that blocks the __pthread_sig_restart signal
     (must do this very early, since the program could capture the signal
      mask with e.g. sigsetjmp before creating the first thread);
   - a regular function called from pthread_create when needed. */

static void pthread_initialize(void) __attribute__((constructor));

 /* Do some minimal initialization which has to be done during the
    startup of the C library.  */
void __pthread_initialize_minimal(void)
{
    /* If we have special thread_self processing, initialize 
     * that for the main thread now.  */
#ifdef INIT_THREAD_SELF
    INIT_THREAD_SELF(&__pthread_initial_thread, 0);
#endif
}


static void pthread_initialize(void)
{
  struct sigaction sa;
  sigset_t mask;
  struct rlimit limit;
  int max_stack;

  /* If already done (e.g. by a constructor called earlier!), bail out */
  if (__pthread_initial_thread_bos != NULL) return;
#ifdef TEST_FOR_COMPARE_AND_SWAP
  /* Test if compare-and-swap is available */
  __pthread_has_cas = compare_and_swap_is_available();
#endif
  /* For the initial stack, reserve at least STACK_SIZE bytes of stack
     below the current stack address, and align that on a
     STACK_SIZE boundary. */
  __pthread_initial_thread_bos =
    (char *)(((long)CURRENT_STACK_FRAME - 2 * STACK_SIZE) & ~(STACK_SIZE - 1));
  /* Update the descriptor for the initial thread. */
  __pthread_initial_thread.p_pid = __getpid();
  /* If we have special thread_self processing, initialize that for the
     main thread now.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(&__pthread_initial_thread, 0);
#endif
  /* The errno/h_errno variable of the main thread are the global ones.  */
  __pthread_initial_thread.p_errnop = &_errno;
  __pthread_initial_thread.p_h_errnop = &_h_errno;

#ifdef __UCLIBC_HAS_XLOCALE__
  /* The locale of the main thread is the current locale in use. */
  __pthread_initial_thread.locale = __curlocale_var;
#endif /* __UCLIBC_HAS_XLOCALE__ */

 {			   /* uClibc-specific stdio initialization for threads. */
	 FILE *fp;
	 
	 _stdio_user_locking = 0;	/* 2 if threading not initialized */
	 for (fp = _stdio_openlist; fp != NULL; fp = fp->__nextopen) {
		 if (fp->__user_locking != 1) {
			 fp->__user_locking = 0;
		 }
	 }
 }

  /* Play with the stack size limit to make sure that no stack ever grows
     beyond STACK_SIZE minus two pages (one page for the thread descriptor
     immediately beyond, and one page to act as a guard page). */

#ifdef __ARCH_HAS_MMU__
  /* We cannot allocate a huge chunk of memory to mmap all thread stacks later
   * on a non-MMU system. Thus, we don't need the rlimit either. -StS */
  getrlimit(RLIMIT_STACK, &limit);
  max_stack = STACK_SIZE - 2 * __getpagesize();
  if (limit.rlim_cur > max_stack) {
    limit.rlim_cur = max_stack;
    setrlimit(RLIMIT_STACK, &limit);
  }
#else
  /* For non-MMU assume __pthread_initial_thread_tos at upper page boundary, and
   * __pthread_initial_thread_bos at address 0. These bounds are refined as we 
   * malloc other stack frames such that they don't overlap. -StS
   */
  __pthread_initial_thread_tos =
    (char *)(((long)CURRENT_STACK_FRAME + __getpagesize()) & ~(__getpagesize() - 1));
  __pthread_initial_thread_bos = (char *) 1; /* set it non-zero so we know we have been here */
  PDEBUG("initial thread stack bounds: bos=%p, tos=%p\n",
	 __pthread_initial_thread_bos, __pthread_initial_thread_tos);
#endif /* __ARCH_HAS_MMU__ */

  /* Setup signal handlers for the initial thread.
     Since signal handlers are shared between threads, these settings
     will be inherited by all other threads. */
  sa.sa_handler = pthread_handle_sigrestart;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  __libc_sigaction(__pthread_sig_restart, &sa, NULL);
  sa.sa_handler = pthread_handle_sigcancel;
  sigaddset(&sa.sa_mask, __pthread_sig_restart);
  // sa.sa_flags = 0;
  __libc_sigaction(__pthread_sig_cancel, &sa, NULL);
  if (__pthread_sig_debug > 0) {
      sa.sa_handler = pthread_handle_sigdebug;
      sigemptyset(&sa.sa_mask);
      // sa.sa_flags = 0;
      __libc_sigaction(__pthread_sig_debug, &sa, NULL);
  }
  /* Initially, block __pthread_sig_restart. Will be unblocked on demand. */
  sigemptyset(&mask);
  sigaddset(&mask, __pthread_sig_restart);
  sigprocmask(SIG_BLOCK, &mask, NULL);
  /* And unblock __pthread_sig_cancel if it has been blocked. */
  sigdelset(&mask, __pthread_sig_restart);
  sigaddset(&mask, __pthread_sig_cancel);
  sigprocmask(SIG_UNBLOCK, &mask, NULL);
  /* Register an exit function to kill all other threads. */
  /* Do it early so that user-registered atexit functions are called
     before pthread_onexit_process. */
  on_exit(pthread_onexit_process, NULL);
}

void __pthread_initialize(void)
{
  pthread_initialize();
}

int __pthread_initialize_manager(void)
{
  int manager_pipe[2];
  int pid;
  int report_events;
  struct pthread_request request;

  /* If basic initialization not done yet (e.g. we're called from a
     constructor run before our constructor), do it now */
  if (__pthread_initial_thread_bos == NULL) pthread_initialize();
  /* Setup stack for thread manager */
  __pthread_manager_thread_bos = malloc(THREAD_MANAGER_STACK_SIZE);
  if (__pthread_manager_thread_bos == NULL) return -1;
  __pthread_manager_thread_tos =
    __pthread_manager_thread_bos + THREAD_MANAGER_STACK_SIZE;

  /* On non-MMU systems we make sure that the initial thread bounds don't overlap
   * with the manager stack frame */
  NOMMU_INITIAL_THREAD_BOUNDS(__pthread_manager_thread_tos,__pthread_manager_thread_bos);
  PDEBUG("manager stack: size=%d, bos=%p, tos=%p\n", THREAD_MANAGER_STACK_SIZE,
	 __pthread_manager_thread_bos, __pthread_manager_thread_tos);
#if 0
  PDEBUG("initial stack: estimate bos=%p, tos=%p\n",
  	 __pthread_initial_thread_bos, __pthread_initial_thread_tos);
#endif

  /* Setup pipe to communicate with thread manager */
  if (pipe(manager_pipe) == -1) {
    free(__pthread_manager_thread_bos);
    return -1;
  }
  /* Start the thread manager */
  pid = 0;
#ifdef USE_TLS
  if (__linuxthreads_initial_report_events != 0)
    THREAD_SETMEM (((pthread_descr) NULL), p_report_events,
		   __linuxthreads_initial_report_events);
  report_events = THREAD_GETMEM (((pthread_descr) NULL), p_report_events);
#else
  if (__linuxthreads_initial_report_events != 0)
    __pthread_initial_thread.p_report_events
      = __linuxthreads_initial_report_events;
  report_events = __pthread_initial_thread.p_report_events;
#endif
  if (__builtin_expect (report_events, 0))
    {
      /* It's a bit more complicated.  We have to report the creation of
	 the manager thread.  */
      int idx = __td_eventword (TD_CREATE);
      uint32_t mask = __td_eventmask (TD_CREATE);

      if ((mask & (__pthread_threads_events.event_bits[idx]
		   | __pthread_initial_thread.p_eventbuf.eventmask.event_bits[idx]))
	  != 0)
	{

	 __pthread_lock(__pthread_manager_thread.p_lock, NULL);

	  pid = clone(__pthread_manager_event,
			(void **) __pthread_manager_thread_tos,
			CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND,
			(void *)(long)manager_pipe[0]);

	  if (pid != -1)
	    {
	      /* Now fill in the information about the new thread in
	         the newly created thread's data structure.  We cannot let
	         the new thread do this since we don't know whether it was
	         already scheduled when we send the event.  */
	      __pthread_manager_thread.p_eventbuf.eventdata =
		  &__pthread_manager_thread;
	      __pthread_manager_thread.p_eventbuf.eventnum = TD_CREATE;
	      __pthread_last_event = &__pthread_manager_thread;
	      __pthread_manager_thread.p_tid = 2* PTHREAD_THREADS_MAX + 1;
	      __pthread_manager_thread.p_pid = pid;

	      /* Now call the function which signals the event.  */
	      __linuxthreads_create_event ();
	    }
	  /* Now restart the thread.  */
	  __pthread_unlock(__pthread_manager_thread.p_lock);
	}
    }

  if (pid == 0) {
    pid = clone(__pthread_manager, (void **) __pthread_manager_thread_tos,
		  CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND,
		  (void *)(long)manager_pipe[0]);
  }
  if (pid == -1) {
    free(__pthread_manager_thread_bos);
    __libc_close(manager_pipe[0]);
    __libc_close(manager_pipe[1]);
    return -1;
  }
  __pthread_manager_request = manager_pipe[1]; /* writing end */
  __pthread_manager_reader = manager_pipe[0]; /* reading end */
  __pthread_manager_thread.p_tid = 2* PTHREAD_THREADS_MAX + 1;
  __pthread_manager_thread.p_pid = pid;

  /* Make gdb aware of new thread manager */
  if (__pthread_threads_debug && __pthread_sig_debug > 0)
    {
      raise(__pthread_sig_debug);
      /* We suspend ourself and gdb will wake us up when it is
	 ready to handle us. */
      __pthread_wait_for_restart_signal(thread_self());
    }
  /* Synchronize debugging of the thread manager */
  PDEBUG("send REQ_DEBUG to manager thread\n");
  request.req_kind = REQ_DEBUG;
  TEMP_FAILURE_RETRY(__libc_write(__pthread_manager_request,
	      (char *) &request, sizeof(request)));
  return 0;
}

/* Thread creation */

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
			 void * (*start_routine)(void *), void *arg)
{
  pthread_descr self = thread_self();
  struct pthread_request request;
  if (__pthread_manager_request < 0) {
    if (__pthread_initialize_manager() < 0) return EAGAIN;
  }
  request.req_thread = self;
  request.req_kind = REQ_CREATE;
  request.req_args.create.attr = attr;
  request.req_args.create.fn = start_routine;
  request.req_args.create.arg = arg;
  sigprocmask(SIG_SETMASK, (const sigset_t *) NULL,
              &request.req_args.create.mask);
  PDEBUG("write REQ_CREATE to manager thread\n");
  TEMP_FAILURE_RETRY(__libc_write(__pthread_manager_request,
	      (char *) &request, sizeof(request)));
  PDEBUG("before suspend(self)\n");
  suspend(self);
  PDEBUG("after suspend(self)\n");
  if (THREAD_GETMEM(self, p_retcode) == 0)
    *thread = (pthread_t) THREAD_GETMEM(self, p_retval);
  return THREAD_GETMEM(self, p_retcode);
}

/* Simple operations on thread identifiers */

pthread_t pthread_self(void)
{
  pthread_descr self = thread_self();
  return THREAD_GETMEM(self, p_tid);
}

int pthread_equal(pthread_t thread1, pthread_t thread2)
{
  return thread1 == thread2;
}

/* Helper function for thread_self in the case of user-provided stacks */

#ifndef THREAD_SELF

pthread_descr __pthread_find_self()
{
  char * sp = CURRENT_STACK_FRAME;
  pthread_handle h;

  /* __pthread_handles[0] is the initial thread, __pthread_handles[1] is
     the manager threads handled specially in thread_self(), so start at 2 */
  h = __pthread_handles + 2;
  while (! (sp <= (char *) h->h_descr && sp >= h->h_bottom)) h++;

#ifdef DEBUG_PT
  if (h->h_descr == NULL) {
      printf("*** %s ERROR descriptor is NULL!!!!! ***\n\n", __FUNCTION__);
      _exit(1);
  }
#endif

  return h->h_descr;
}
#else

static pthread_descr thread_self_stack(void)
{
    char *sp = CURRENT_STACK_FRAME;
    pthread_handle h;

    if (sp >= __pthread_manager_thread_bos && sp < __pthread_manager_thread_tos)
	return manager_thread;
    h = __pthread_handles + 2;
# ifdef USE_TLS
    while (h->h_descr == NULL
	    || ! (sp <= (char *) h->h_descr->p_stackaddr && sp >= h->h_bottom))
	h++;
# else
    while (! (sp <= (char *) h->h_descr && sp >= h->h_bottom))
	h++;
# endif
    return h->h_descr;
}

#endif

/* Thread scheduling */

int pthread_setschedparam(pthread_t thread, int policy,
                          const struct sched_param *param)
{
  pthread_handle handle = thread_handle(thread);
  pthread_descr th;

  __pthread_lock(&handle->h_lock, NULL);
  if (invalid_handle(handle, thread)) {
    __pthread_unlock(&handle->h_lock);
    return ESRCH;
  }
  th = handle->h_descr;
  if (sched_setscheduler(th->p_pid, policy, param) == -1) {
    __pthread_unlock(&handle->h_lock);
    return errno;
  }
  th->p_priority = policy == SCHED_OTHER ? 0 : param->sched_priority;
  __pthread_unlock(&handle->h_lock);
  if (__pthread_manager_request >= 0)
    __pthread_manager_adjust_prio(th->p_priority);
  return 0;
}

int pthread_getschedparam(pthread_t thread, int *policy,
                          struct sched_param *param)
{
  pthread_handle handle = thread_handle(thread);
  int pid, pol;

  __pthread_lock(&handle->h_lock, NULL);
  if (invalid_handle(handle, thread)) {
    __pthread_unlock(&handle->h_lock);
    return ESRCH;
  }
  pid = handle->h_descr->p_pid;
  __pthread_unlock(&handle->h_lock);
  pol = sched_getscheduler(pid);
  if (pol == -1) return errno;
  if (sched_getparam(pid, param) == -1) return errno;
  *policy = pol;
  return 0;
}

/* Process-wide exit() request */

static void pthread_onexit_process(int retcode, void *arg)
{
    struct pthread_request request;
    pthread_descr self = thread_self();

    if (__pthread_manager_request >= 0) {
	request.req_thread = self;
	request.req_kind = REQ_PROCESS_EXIT;
	request.req_args.exit.code = retcode;
	TEMP_FAILURE_RETRY(__libc_write(__pthread_manager_request,
		    (char *) &request, sizeof(request)));
	suspend(self);
	/* Main thread should accumulate times for thread manager and its
	   children, so that timings for main thread account for all threads. */
	if (self == __pthread_main_thread) {
	    waitpid(__pthread_manager_thread.p_pid, NULL, __WCLONE);
	    /* Since all threads have been asynchronously terminated
	     * (possibly holding locks), free cannot be used any more.  */
	    __pthread_manager_thread_bos = __pthread_manager_thread_tos = NULL;
	}
    }
}

/* The handler for the RESTART signal just records the signal received
   in the thread descriptor, and optionally performs a siglongjmp
   (for pthread_cond_timedwait). */

static void pthread_handle_sigrestart(int sig)
{
    pthread_descr self = thread_self();
    THREAD_SETMEM(self, p_signal, sig);
    if (THREAD_GETMEM(self, p_signal_jmp) != NULL)
	siglongjmp(*THREAD_GETMEM(self, p_signal_jmp), 1);
}

/* The handler for the CANCEL signal checks for cancellation
   (in asynchronous mode), for process-wide exit and exec requests.
   For the thread manager thread, redirect the signal to
   __pthread_manager_sighandler. */

static void pthread_handle_sigcancel(int sig)
{
  pthread_descr self = thread_self();
  sigjmp_buf * jmpbuf;
  

  if (self == &__pthread_manager_thread)
    {
#ifdef THREAD_SELF
      /* A new thread might get a cancel signal before it is fully
	 initialized, so that the thread register might still point to the
	 manager thread.  Double check that this is really the manager
	 thread.  */
      pthread_descr real_self = thread_self_stack();
      if (real_self == &__pthread_manager_thread)
	{
	  __pthread_manager_sighandler(sig);
	  return;
	}
      /* Oops, thread_self() isn't working yet..  */
      self = real_self;
# ifdef INIT_THREAD_SELF
      INIT_THREAD_SELF(self, self->p_nr);
# endif
#else
      __pthread_manager_sighandler(sig);
      return;
#endif
    }
  if (__builtin_expect (__pthread_exit_requested, 0)) {
    /* Main thread should accumulate times for thread manager and its
       children, so that timings for main thread account for all threads. */
    if (self == __pthread_main_thread) {
#ifdef USE_TLS
      waitpid(__pthread_manager_thread->p_pid, NULL, __WCLONE);
#else
      waitpid(__pthread_manager_thread.p_pid, NULL, __WCLONE);
#endif
    }
    _exit(__pthread_exit_code);
  }
  if (__builtin_expect (THREAD_GETMEM(self, p_canceled), 0)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
    if (THREAD_GETMEM(self, p_canceltype) == PTHREAD_CANCEL_ASYNCHRONOUS)
      pthread_exit(PTHREAD_CANCELED);
    jmpbuf = THREAD_GETMEM(self, p_cancel_jmp);
    if (jmpbuf != NULL) {
      THREAD_SETMEM(self, p_cancel_jmp, NULL);
      siglongjmp(*jmpbuf, 1);
    }
  }
}

/* Handler for the DEBUG signal.
   The debugging strategy is as follows:
   On reception of a REQ_DEBUG request (sent by new threads created to
   the thread manager under debugging mode), the thread manager throws
   __pthread_sig_debug to itself. The debugger (if active) intercepts
   this signal, takes into account new threads and continue execution
   of the thread manager by propagating the signal because it doesn't
   know what it is specifically done for. In the current implementation,
   the thread manager simply discards it. */

static void pthread_handle_sigdebug(int sig)
{
  /* Nothing */
}

/* Reset the state of the thread machinery after a fork().
   Close the pipe used for requests and set the main thread to the forked
   thread.
   Notice that we can't free the stack segments, as the forked thread
   may hold pointers into them. */

void __pthread_reset_main_thread()
{
  pthread_descr self = thread_self();

  if (__pthread_manager_request != -1) {
    /* Free the thread manager stack */
    free(__pthread_manager_thread_bos);
    __pthread_manager_thread_bos = __pthread_manager_thread_tos = NULL;
    /* Close the two ends of the pipe */
    __libc_close(__pthread_manager_request);
    __libc_close(__pthread_manager_reader);
    __pthread_manager_request = __pthread_manager_reader = -1;
  }

  /* Update the pid of the main thread */
  THREAD_SETMEM(self, p_pid, __getpid());
  /* Make the forked thread the main thread */
  __pthread_main_thread = self;
  THREAD_SETMEM(self, p_nextlive, self);
  THREAD_SETMEM(self, p_prevlive, self);
  /* Now this thread modifies the global variables.  */
  THREAD_SETMEM(self, p_errnop, &_errno);
  THREAD_SETMEM(self, p_h_errnop, &_h_errno);
}

/* Process-wide exec() request */

void __pthread_kill_other_threads_np(void)
{
  struct sigaction sa;
  /* Terminate all other threads and thread manager */
  pthread_onexit_process(0, NULL);
  /* Make current thread the main thread in case the calling thread
     changes its mind, does not exec(), and creates new threads instead. */
  __pthread_reset_main_thread();
  /* Reset the signal handlers behaviour for the signals the
     implementation uses since this would be passed to the new
     process.  */
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = SIG_DFL;
  __libc_sigaction(__pthread_sig_restart, &sa, NULL);
  __libc_sigaction(__pthread_sig_cancel, &sa, NULL);
  if (__pthread_sig_debug > 0)
    __libc_sigaction(__pthread_sig_debug, &sa, NULL);
}
weak_alias (__pthread_kill_other_threads_np, pthread_kill_other_threads_np)

/* Concurrency symbol level.  */
static int current_level;

int __pthread_setconcurrency(int level)
{
  /* We don't do anything unless we have found a useful interpretation.  */
  current_level = level;
  return 0;
}
weak_alias (__pthread_setconcurrency, pthread_setconcurrency)

int __pthread_getconcurrency(void)
{
  return current_level;
}
weak_alias (__pthread_getconcurrency, pthread_getconcurrency)


/* Primitives for controlling thread execution */

void __pthread_wait_for_restart_signal(pthread_descr self)
{
    sigset_t mask;

    sigprocmask(SIG_SETMASK, NULL, &mask); /* Get current signal mask */
    sigdelset(&mask, __pthread_sig_restart); /* Unblock the restart signal */
    THREAD_SETMEM(self, p_signal, 0);
    do {
	sigsuspend(&mask);                   /* Wait for signal */
    } while (THREAD_GETMEM(self, p_signal) !=__pthread_sig_restart);

    READ_MEMORY_BARRIER(); /* See comment in __pthread_restart_new */
}

#ifndef __NR_rt_sigaction
/* The _old variants are for 2.0 and early 2.1 kernels which don't have RT
   signals.
   On these kernels, we use SIGUSR1 and SIGUSR2 for restart and cancellation.
   Since the restart signal does not queue, we use an atomic counter to create
   queuing semantics. This is needed to resolve a rare race condition in
   pthread_cond_timedwait_relative. */

void __pthread_restart_old(pthread_descr th)
{
    if (atomic_increment(&th->p_resume_count) == -1)
	kill(th->p_pid, __pthread_sig_restart);
}

void __pthread_suspend_old(pthread_descr self)
{
    if (atomic_decrement(&self->p_resume_count) <= 0)
	__pthread_wait_for_restart_signal(self);
}

int
__pthread_timedsuspend_old(pthread_descr self, const struct timespec *abstime)
{
  sigset_t unblock, initial_mask;
  int was_signalled = 0;
  sigjmp_buf jmpbuf;

  if (atomic_decrement(&self->p_resume_count) == 0) {
    /* Set up a longjmp handler for the restart signal, unblock
       the signal and sleep. */

    if (sigsetjmp(jmpbuf, 1) == 0) {
      THREAD_SETMEM(self, p_signal_jmp, &jmpbuf);
      THREAD_SETMEM(self, p_signal, 0);
      /* Unblock the restart signal */
      sigemptyset(&unblock);
      sigaddset(&unblock, __pthread_sig_restart);
      sigprocmask(SIG_UNBLOCK, &unblock, &initial_mask);

      while (1) {
	struct timeval now;
	struct timespec reltime;

	/* Compute a time offset relative to now.  */
	__gettimeofday (&now, NULL);
	reltime.tv_nsec = abstime->tv_nsec - now.tv_usec * 1000;
	reltime.tv_sec = abstime->tv_sec - now.tv_sec;
	if (reltime.tv_nsec < 0) {
	  reltime.tv_nsec += 1000000000;
	  reltime.tv_sec -= 1;
	}

	/* Sleep for the required duration. If woken by a signal,
	   resume waiting as required by Single Unix Specification.  */
	if (reltime.tv_sec < 0 || __libc_nanosleep(&reltime, NULL) == 0)
	  break;
      }

      /* Block the restart signal again */
      sigprocmask(SIG_SETMASK, &initial_mask, NULL);
      was_signalled = 0;
    } else {
      was_signalled = 1;
    }
    THREAD_SETMEM(self, p_signal_jmp, NULL);
  }

  /* Now was_signalled is true if we exited the above code
     due to the delivery of a restart signal.  In that case,
     we know we have been dequeued and resumed and that the
     resume count is balanced.  Otherwise, there are some
     cases to consider. First, try to bump up the resume count
     back to zero. If it goes to 1, it means restart() was
     invoked on this thread. The signal must be consumed
     and the count bumped down and everything is cool. We
     can return a 1 to the caller.
     Otherwise, no restart was delivered yet, so a potential
     race exists; we return a 0 to the caller which must deal
     with this race in an appropriate way; for example by
     atomically removing the thread from consideration for a
     wakeup---if such a thing fails, it means a restart is
     being delivered. */

  if (!was_signalled) {
    if (atomic_increment(&self->p_resume_count) != -1) {
      __pthread_wait_for_restart_signal(self);
      atomic_decrement(&self->p_resume_count); /* should be zero now! */
      /* woke spontaneously and consumed restart signal */
      return 1;
    }
    /* woke spontaneously but did not consume restart---caller must resolve */
    return 0;
  }
  /* woken due to restart signal */
  return 1;
}
#endif /* __NR_rt_sigaction */


#ifdef __NR_rt_sigaction
void __pthread_restart_new(pthread_descr th)
{
    /* The barrier is proabably not needed, in which case it still documents
       our assumptions. The intent is to commit previous writes to shared
       memory so the woken thread will have a consistent view.  Complementary
       read barriers are present to the suspend functions. */
    WRITE_MEMORY_BARRIER();
    kill(th->p_pid, __pthread_sig_restart);
}

int __pthread_timedsuspend_new(pthread_descr self, const struct timespec *abstime)
{
    sigset_t unblock, initial_mask;
    int was_signalled = 0;
    sigjmp_buf jmpbuf;

    if (sigsetjmp(jmpbuf, 1) == 0) {
	THREAD_SETMEM(self, p_signal_jmp, &jmpbuf);
	THREAD_SETMEM(self, p_signal, 0);
	/* Unblock the restart signal */
	sigemptyset(&unblock);
	sigaddset(&unblock, __pthread_sig_restart);
	sigprocmask(SIG_UNBLOCK, &unblock, &initial_mask);

	while (1) {
	    struct timeval now;
	    struct timespec reltime;

	    /* Compute a time offset relative to now.  */
	    gettimeofday (&now, NULL);
	    reltime.tv_nsec = abstime->tv_nsec - now.tv_usec * 1000;
	    reltime.tv_sec = abstime->tv_sec - now.tv_sec;
	    if (reltime.tv_nsec < 0) {
		reltime.tv_nsec += 1000000000;
		reltime.tv_sec -= 1;
	    }

	    /* Sleep for the required duration. If woken by a signal,
	       resume waiting as required by Single Unix Specification.  */
	    if (reltime.tv_sec < 0 || __libc_nanosleep(&reltime, NULL) == 0)
		break;
	}

	/* Block the restart signal again */
	sigprocmask(SIG_SETMASK, &initial_mask, NULL);
	was_signalled = 0;
    } else {
	was_signalled = 1;
    }
    THREAD_SETMEM(self, p_signal_jmp, NULL);

    /* Now was_signalled is true if we exited the above code
       due to the delivery of a restart signal.  In that case,
       everything is cool. We have been removed from whatever
       we were waiting on by the other thread, and consumed its signal.

       Otherwise we this thread woke up spontaneously, or due to a signal other
       than restart. This is an ambiguous case  that must be resolved by
       the caller; the thread is still eligible for a restart wakeup
       so there is a race. */

    READ_MEMORY_BARRIER(); /* See comment in __pthread_restart_new */
    return was_signalled;
}
#endif

/* Debugging aid */

#ifdef DEBUG_PT
#include <stdarg.h>

void __pthread_message(char * fmt, ...)
{
  char buffer[1024];
  va_list args;
  sprintf(buffer, "%05d : ", __getpid());
  va_start(args, fmt);
  vsnprintf(buffer + 8, sizeof(buffer) - 8, fmt, args);
  va_end(args);
  TEMP_FAILURE_RETRY(__libc_write(2, buffer, strlen(buffer)));
}

#endif


#ifndef __PIC__
/* We need a hook to force the cancelation wrappers to be linked in when
   static libpthread is used.  */
extern const int __pthread_provide_wrappers;
static const int *const __pthread_require_wrappers =
  &__pthread_provide_wrappers;
#endif
