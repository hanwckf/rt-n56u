
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

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#include "smp.h"
#include <not-cancel.h>

/* Sanity check.  */
#if !defined __SIGRTMIN || (__SIGRTMAX - __SIGRTMIN) < 3
# error "This must not happen"
#endif

/* mods for uClibc: __libc_sigaction is not in any standard headers */
extern __typeof(sigaction) __libc_sigaction;

#if !(USE_TLS && HAVE___THREAD)
/* These variables are used by the setup code.  */
extern int _errno;
extern int _h_errno;

# if defined __UCLIBC_HAS_RESOLVER_SUPPORT__
/* We need the global/static resolver state here.  */
# include <resolv.h>
# undef _res
extern struct __res_state *__resp;
# endif
#endif

#ifdef USE_TLS

/* We need only a few variables.  */
#define manager_thread __pthread_manager_threadp
pthread_descr __pthread_manager_threadp attribute_hidden;

#else

/* Descriptor of the initial thread */

struct _pthread_descr_struct __pthread_initial_thread = {
  .p_header.data.self = &__pthread_initial_thread,
  .p_nextlive = &__pthread_initial_thread,
  .p_prevlive = &__pthread_initial_thread,
  .p_tid = PTHREAD_THREADS_MAX,
  .p_lock = &__pthread_handles[0].h_lock,
  .p_start_args = PTHREAD_START_ARGS_INITIALIZER(NULL),
#if !(USE_TLS && HAVE___THREAD)
  .p_errnop = &_errno,
  .p_h_errnop = &_h_errno,
#endif
  .p_userstack = 1,
  .p_resume_count = __ATOMIC_INITIALIZER,
  .p_alloca_cutoff = __MAX_ALLOCA_CUTOFF
};

/* Descriptor of the manager thread; none of this is used but the error
   variables, the p_pid and p_priority fields,
   and the address for identification.  */

#define manager_thread (&__pthread_manager_thread)
struct _pthread_descr_struct __pthread_manager_thread = {
  .p_header.data.self = &__pthread_manager_thread,
  .p_header.data.multiple_threads = 1,
  .p_lock = &__pthread_handles[1].h_lock,
  .p_start_args = PTHREAD_START_ARGS_INITIALIZER(__pthread_manager),
#if !(USE_TLS && HAVE___THREAD)
  .p_errnop = &__pthread_manager_thread.p_errno,
#endif
  .p_nr = 1,
  .p_resume_count = __ATOMIC_INITIALIZER,
  .p_alloca_cutoff = PTHREAD_STACK_MIN / 4
};
#endif

/* Pointer to the main thread (the father of the thread manager thread) */
/* Originally, this is the initial thread, but this changes after fork() */

#ifdef USE_TLS
pthread_descr __pthread_main_thread;
#else
pthread_descr __pthread_main_thread = &__pthread_initial_thread;
#endif

/* Limit between the stack of the initial thread (above) and the
   stacks of other threads (below). Aligned on a STACK_SIZE boundary. */

char *__pthread_initial_thread_bos;

/* File descriptor for sending requests to the thread manager. */
/* Initially -1, meaning that the thread manager is not running. */

int __pthread_manager_request = -1;

int __pthread_multiple_threads attribute_hidden;

/* Other end of the pipe for sending requests to the thread manager. */

int __pthread_manager_reader;

/* Limits of the thread manager stack */

char *__pthread_manager_thread_bos;
char *__pthread_manager_thread_tos;

/* For process-wide exit() */

int __pthread_exit_requested;
int __pthread_exit_code;

/* Maximum stack size.  */
size_t __pthread_max_stacksize;

/* Nozero if the machine has more than one processor.  */
int __pthread_smp_kernel;


#if !__ASSUME_REALTIME_SIGNALS
/* Pointers that select new or old suspend/resume functions
   based on availability of rt signals. */

void (*__pthread_restart)(pthread_descr) = __pthread_restart_old;
void (*__pthread_suspend)(pthread_descr) = __pthread_suspend_old;
int (*__pthread_timedsuspend)(pthread_descr, const struct timespec *) = __pthread_timedsuspend_old;
#endif	/* __ASSUME_REALTIME_SIGNALS */

/* Communicate relevant LinuxThreads constants to gdb */

const int __pthread_threads_max = PTHREAD_THREADS_MAX;
const int __pthread_sizeof_handle = sizeof(struct pthread_handle_struct);
const int __pthread_offsetof_descr = offsetof(struct pthread_handle_struct,
                                              h_descr);
const int __pthread_offsetof_pid = offsetof(struct _pthread_descr_struct,
                                            p_pid);
const int __linuxthreads_pthread_sizeof_descr
  = sizeof(struct _pthread_descr_struct);

const int __linuxthreads_initial_report_events;

const char __linuxthreads_version[] = VERSION;

/* Forward declarations */

static void pthread_onexit_process(int retcode, void *arg);
#ifndef HAVE_Z_NODELETE
static void pthread_atexit_process(void *arg, int retcode);
static void pthread_atexit_retcode(void *arg, int retcode);
#endif
static void pthread_handle_sigcancel(int sig);
static void pthread_handle_sigrestart(int sig);
static void pthread_handle_sigdebug(int sig);

/* Signal numbers used for the communication.
   In these variables we keep track of the used variables.  If the
   platform does not support any real-time signals we will define the
   values to some unreasonable value which will signal failing of all
   the functions below.  */
int __pthread_sig_restart = __SIGRTMIN;
int __pthread_sig_cancel = __SIGRTMIN + 1;
int __pthread_sig_debug = __SIGRTMIN + 2;

extern int __libc_current_sigrtmin_private (void);

#if !__ASSUME_REALTIME_SIGNALS
static int rtsigs_initialized;

static void
init_rtsigs (void)
{
  if (rtsigs_initialized)
    return;

  if (__libc_current_sigrtmin_private () == -1)
    {
      __pthread_sig_restart = SIGUSR1;
      __pthread_sig_cancel = SIGUSR2;
      __pthread_sig_debug = 0;
    }
  else
    {
      __pthread_restart = __pthread_restart_new;
      __pthread_suspend = __pthread_wait_for_restart_signal;
      __pthread_timedsuspend = __pthread_timedsuspend_new;
    }

  rtsigs_initialized = 1;
}
#endif


/* Initialize the pthread library.
   Initialization is split in two functions:
   - a constructor function that blocks the __pthread_sig_restart signal
     (must do this very early, since the program could capture the signal
      mask with e.g. sigsetjmp before creating the first thread);
   - a regular function called from pthread_create when needed. */

static void pthread_initialize(void) __attribute__((constructor));

#ifndef HAVE_Z_NODELETE
extern void *__dso_handle __attribute__ ((weak));
#endif


#if defined USE_TLS && !defined SHARED
extern void __libc_setup_tls (size_t tcbsize, size_t tcbalign);
#endif

struct pthread_functions __pthread_functions =
  {
#if !(USE_TLS && HAVE___THREAD)
    .ptr_pthread_internal_tsd_set = __pthread_internal_tsd_set,
    .ptr_pthread_internal_tsd_get = __pthread_internal_tsd_get,
    .ptr_pthread_internal_tsd_address = __pthread_internal_tsd_address,
#endif
    .ptr_pthread_fork = __pthread_fork,
    .ptr_pthread_attr_destroy = __pthread_attr_destroy,
    .ptr_pthread_attr_init = __pthread_attr_init,
    .ptr_pthread_attr_getdetachstate = __pthread_attr_getdetachstate,
    .ptr_pthread_attr_setdetachstate = __pthread_attr_setdetachstate,
    .ptr_pthread_attr_getinheritsched = __pthread_attr_getinheritsched,
    .ptr_pthread_attr_setinheritsched = __pthread_attr_setinheritsched,
    .ptr_pthread_attr_getschedparam = __pthread_attr_getschedparam,
    .ptr_pthread_attr_setschedparam = __pthread_attr_setschedparam,
    .ptr_pthread_attr_getschedpolicy = __pthread_attr_getschedpolicy,
    .ptr_pthread_attr_setschedpolicy = __pthread_attr_setschedpolicy,
    .ptr_pthread_attr_getscope = __pthread_attr_getscope,
    .ptr_pthread_attr_setscope = __pthread_attr_setscope,
    .ptr_pthread_condattr_destroy = __pthread_condattr_destroy,
    .ptr_pthread_condattr_init = __pthread_condattr_init,
    .ptr_pthread_cond_broadcast = __pthread_cond_broadcast,
    .ptr_pthread_cond_destroy = __pthread_cond_destroy,
    .ptr_pthread_cond_init = __pthread_cond_init,
    .ptr_pthread_cond_signal = __pthread_cond_signal,
    .ptr_pthread_cond_wait = __pthread_cond_wait,
    .ptr_pthread_cond_timedwait = __pthread_cond_timedwait,
    .ptr_pthread_equal = __pthread_equal,
    .ptr___pthread_exit = __pthread_exit,
    .ptr_pthread_getschedparam = __pthread_getschedparam,
    .ptr_pthread_setschedparam = __pthread_setschedparam,
    .ptr_pthread_mutex_destroy = __pthread_mutex_destroy,
    .ptr_pthread_mutex_init = __pthread_mutex_init,
    .ptr_pthread_mutex_lock = __pthread_mutex_lock,
    .ptr_pthread_mutex_trylock = __pthread_mutex_trylock,
    .ptr_pthread_mutex_unlock = __pthread_mutex_unlock,
    .ptr_pthread_self = __pthread_self,
    .ptr_pthread_setcancelstate = __pthread_setcancelstate,
    .ptr_pthread_setcanceltype = __pthread_setcanceltype,
    .ptr_pthread_do_exit = __pthread_do_exit,
    .ptr_pthread_thread_self = __pthread_thread_self,
    .ptr_pthread_cleanup_upto = __pthread_cleanup_upto,
    .ptr_pthread_sigaction = __pthread_sigaction,
    .ptr_pthread_sigwait = __pthread_sigwait,
    .ptr_pthread_raise = __pthread_raise,
    .ptr__pthread_cleanup_push = _pthread_cleanup_push,
    .ptr__pthread_cleanup_push_defer = _pthread_cleanup_push_defer,
    .ptr__pthread_cleanup_pop = _pthread_cleanup_pop,
    .ptr__pthread_cleanup_pop_restore = _pthread_cleanup_pop_restore,
  };
#ifdef SHARED
# define ptr_pthread_functions &__pthread_functions
#else
# define ptr_pthread_functions NULL
#endif

static int *__libc_multiple_threads_ptr;

/* Do some minimal initialization which has to be done during the
   startup of the C library.  */
void
__pthread_initialize_minimal(void)
{
#ifdef USE_TLS
  pthread_descr self;

  /* First of all init __pthread_handles[0] and [1] if needed.  */
# if __LT_SPINLOCK_INIT != 0
  __pthread_handles[0].h_lock = __LOCK_INITIALIZER;
  __pthread_handles[1].h_lock = __LOCK_INITIALIZER;
# endif
# ifndef SHARED
  /* Unlike in the dynamically linked case the dynamic linker has not
     taken care of initializing the TLS data structures.  */
  __libc_setup_tls (TLS_TCB_SIZE, TLS_TCB_ALIGN);
# elif !USE___THREAD
  if (__builtin_expect (GL(dl_tls_dtv_slotinfo_list) == NULL, 0))
    {
      tcbhead_t *tcbp;

      /* There is no actual TLS being used, so the thread register
	 was not initialized in the dynamic linker.  */

      /* We need to install special hooks so that the malloc and memalign
	 calls in _dl_tls_setup and _dl_allocate_tls won't cause full
	 malloc initialization that will try to set up its thread state.  */

      extern void __libc_malloc_pthread_startup (bool first_time);
      __libc_malloc_pthread_startup (true);

      if (__builtin_expect (_dl_tls_setup (), 0)
	  || __builtin_expect ((tcbp = _dl_allocate_tls (NULL)) == NULL, 0))
	{
	  static const char msg[] = "\
cannot allocate TLS data structures for initial thread\n";
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO,
						msg, sizeof msg - 1));
	  abort ();
	}
      const char *lossage = TLS_INIT_TP (tcbp, 0);
      if (__builtin_expect (lossage != NULL, 0))
	{
	  static const char msg[] = "cannot set up thread-local storage: ";
	  const char nl = '\n';
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO,
						msg, sizeof msg - 1));
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO,
						lossage, strlen (lossage)));
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO, &nl, 1));
	}

      /* Though it was allocated with libc's malloc, that was done without
	 the user's __malloc_hook installed.  A later realloc that uses
	 the hooks might not work with that block from the plain malloc.
	 So we record this block as unfreeable just as the dynamic linker
	 does when it allocates the DTV before the libc malloc exists.  */
      GL(dl_initial_dtv) = GET_DTV (tcbp);

      __libc_malloc_pthread_startup (false);
    }
# endif

  self = THREAD_SELF;

  /* The memory for the thread descriptor was allocated elsewhere as
     part of the TLS allocation.  We have to initialize the data
     structure by hand.  This initialization must mirror the struct
     definition above.  */
  self->p_nextlive = self->p_prevlive = self;
  self->p_tid = PTHREAD_THREADS_MAX;
  self->p_lock = &__pthread_handles[0].h_lock;
# ifndef HAVE___THREAD
  self->p_errnop = &_errno;
  self->p_h_errnop = &_h_errno;
# endif
  /* self->p_start_args need not be initialized, it's all zero.  */
  self->p_userstack = 1;
# if __LT_SPINLOCK_INIT != 0
  self->p_resume_count = (struct pthread_atomic) __ATOMIC_INITIALIZER;
# endif
  self->p_alloca_cutoff = __MAX_ALLOCA_CUTOFF;

  /* Another variable which points to the thread descriptor.  */
  __pthread_main_thread = self;

  /* And fill in the pointer the the thread __pthread_handles array.  */
  __pthread_handles[0].h_descr = self;

#else  /* USE_TLS */

  /* First of all init __pthread_handles[0] and [1].  */
# if __LT_SPINLOCK_INIT != 0
  __pthread_handles[0].h_lock = __LOCK_INITIALIZER;
  __pthread_handles[1].h_lock = __LOCK_INITIALIZER;
# endif
  __pthread_handles[0].h_descr = &__pthread_initial_thread;
  __pthread_handles[1].h_descr = &__pthread_manager_thread;

  /* If we have special thread_self processing, initialize that for the
     main thread now.  */
# ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(&__pthread_initial_thread, 0);
# endif
#endif

#if HP_TIMING_AVAIL
# ifdef USE_TLS
  self->p_cpuclock_offset = GL(dl_cpuclock_offset);
# else
  __pthread_initial_thread.p_cpuclock_offset = GL(dl_cpuclock_offset);
# endif
#endif

  __libc_multiple_threads_ptr = __libc_pthread_init (ptr_pthread_functions);
}


void
__pthread_init_max_stacksize(void)
{
  struct rlimit limit;
  size_t max_stack;

  getrlimit(RLIMIT_STACK, &limit);
#ifdef FLOATING_STACKS
  if (limit.rlim_cur == RLIM_INFINITY)
    limit.rlim_cur = ARCH_STACK_MAX_SIZE;
# ifdef NEED_SEPARATE_REGISTER_STACK
  max_stack = limit.rlim_cur / 2;
# else
  max_stack = limit.rlim_cur;
# endif
#else
  /* Play with the stack size limit to make sure that no stack ever grows
     beyond STACK_SIZE minus one page (to act as a guard page). */
# ifdef NEED_SEPARATE_REGISTER_STACK
  /* STACK_SIZE bytes hold both the main stack and register backing
     store. The rlimit value applies to each individually.  */
  max_stack = STACK_SIZE/2 - __getpagesize ();
# else
  max_stack = STACK_SIZE - __getpagesize();
# endif
  if (limit.rlim_cur > max_stack) {
    limit.rlim_cur = max_stack;
    setrlimit(RLIMIT_STACK, &limit);
  }
#endif
  __pthread_max_stacksize = max_stack;
  if (max_stack / 4 < __MAX_ALLOCA_CUTOFF)
    {
#ifdef USE_TLS
      pthread_descr self = THREAD_SELF;
      self->p_alloca_cutoff = max_stack / 4;
#else
      __pthread_initial_thread.p_alloca_cutoff = max_stack / 4;
#endif
    }
}

/* psm: we do not have any ld.so support yet
 *	 remove the USE_TLS guard if nptl is added */
#if defined SHARED && defined USE_TLS
# if USE___THREAD
/* When using __thread for this, we do it in libc so as not
   to give libpthread its own TLS segment just for this.  */
extern void **__libc_dl_error_tsd (void) __attribute__ ((const));
# else
static void ** __attribute__ ((const))
__libc_dl_error_tsd (void)
{
  return &thread_self ()->p_libc_specific[_LIBC_TSD_KEY_DL_ERROR];
}
# endif
#endif

#ifdef USE_TLS
static __inline__ void __attribute__((always_inline))
init_one_static_tls (pthread_descr descr, struct link_map *map)
{
# if defined(TLS_TCB_AT_TP)
  dtv_t *dtv = GET_DTV (descr);
  void *dest = (char *) descr - map->l_tls_offset;
# elif defined(TLS_DTV_AT_TP)
  dtv_t *dtv = GET_DTV ((pthread_descr) ((char *) descr + TLS_PRE_TCB_SIZE));
  void *dest = (char *) descr + map->l_tls_offset + TLS_PRE_TCB_SIZE;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* Fill in the DTV slot so that a later LD/GD access will find it.  */
  dtv[map->l_tls_modid].pointer.val = dest;
  dtv[map->l_tls_modid].pointer.is_static = true;

  /* Initialize the memory.  */
  memset (__mempcpy (dest, map->l_tls_initimage, map->l_tls_initimage_size),
	  '\0', map->l_tls_blocksize - map->l_tls_initimage_size);
}

static void
__pthread_init_static_tls (struct link_map *map)
{
  size_t i;

  for (i = 0; i < PTHREAD_THREADS_MAX; ++i)
    if (__pthread_handles[i].h_descr != NULL && i != 1)
      {
        __pthread_lock (&__pthread_handles[i].h_lock, NULL);
	if (__pthread_handles[i].h_descr != NULL)
	  init_one_static_tls (__pthread_handles[i].h_descr, map);
        __pthread_unlock (&__pthread_handles[i].h_lock);
      }
}
#endif

static void pthread_initialize(void)
{
  struct sigaction sa;
  sigset_t mask;

  /* If already done (e.g. by a constructor called earlier!), bail out */
  if (__pthread_initial_thread_bos != NULL) return;
#ifdef TEST_FOR_COMPARE_AND_SWAP
  /* Test if compare-and-swap is available */
  __pthread_has_cas = compare_and_swap_is_available();
#endif
#ifdef FLOATING_STACKS
  /* We don't need to know the bottom of the stack.  Give the pointer some
     value to signal that initialization happened.  */
  __pthread_initial_thread_bos = (void *) -1l;
#else
  /* Determine stack size limits .  */
  __pthread_init_max_stacksize ();
# ifdef _STACK_GROWS_UP
  /* The initial thread already has all the stack it needs */
  __pthread_initial_thread_bos = (char *)
    ((long)CURRENT_STACK_FRAME &~ (STACK_SIZE - 1));
# else
  /* For the initial stack, reserve at least STACK_SIZE bytes of stack
     below the current stack address, and align that on a
     STACK_SIZE boundary. */
  __pthread_initial_thread_bos =
    (char *)(((long)CURRENT_STACK_FRAME - 2 * STACK_SIZE) & ~(STACK_SIZE - 1));
# endif
#endif
#ifdef USE_TLS
  /* Update the descriptor for the initial thread. */
  THREAD_SETMEM (((pthread_descr) NULL), p_pid, __getpid());
# if !defined HAVE___THREAD && defined __UCLIBC_HAS_RESOLVER_SUPPORT__
  /* Likewise for the resolver state _res.  */
  THREAD_SETMEM (((pthread_descr) NULL), p_resp, __resp);
# endif
#else
  /* Update the descriptor for the initial thread. */
  __pthread_initial_thread.p_pid = __getpid();
# if defined __UCLIBC_HAS_RESOLVER_SUPPORT__
  /* Likewise for the resolver state _res.  */
  __pthread_initial_thread.p_resp = __resp;
# endif
#endif
#if !__ASSUME_REALTIME_SIGNALS
  /* Initialize real-time signals. */
  init_rtsigs ();
#endif
  /* Setup signal handlers for the initial thread.
     Since signal handlers are shared between threads, these settings
     will be inherited by all other threads. */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = pthread_handle_sigrestart;
  __libc_sigaction(__pthread_sig_restart, &sa, NULL);
  sa.sa_handler = pthread_handle_sigcancel;
  sigaddset(&sa.sa_mask, __pthread_sig_restart);
  __libc_sigaction(__pthread_sig_cancel, &sa, NULL);
  if (__pthread_sig_debug > 0) {
    sa.sa_handler = pthread_handle_sigdebug;
    __sigemptyset(&sa.sa_mask);
    __libc_sigaction(__pthread_sig_debug, &sa, NULL);
  }
  /* Initially, block __pthread_sig_restart. Will be unblocked on demand. */
  __sigemptyset(&mask);
  sigaddset(&mask, __pthread_sig_restart);
  sigprocmask(SIG_BLOCK, &mask, NULL);
  /* And unblock __pthread_sig_cancel if it has been blocked. */
  sigdelset(&mask, __pthread_sig_restart);
  sigaddset(&mask, __pthread_sig_cancel);
  sigprocmask(SIG_UNBLOCK, &mask, NULL);
  /* Register an exit function to kill all other threads. */
  /* Do it early so that user-registered atexit functions are called
     before pthread_*exit_process. */
#ifndef HAVE_Z_NODELETE
  if (__builtin_expect (&__dso_handle != NULL, 1))
    __cxa_atexit ((void (*) (void *)) pthread_atexit_process, NULL,
		  __dso_handle);
  else
#endif
    __on_exit (pthread_onexit_process, NULL);
  /* How many processors.  */
  __pthread_smp_kernel = is_smp_system ();

/* psm: we do not have any ld.so support yet
 *	 remove the USE_TLS guard if nptl is added */
#if defined SHARED && defined USE_TLS
  /* Transfer the old value from the dynamic linker's internal location.  */
  *__libc_dl_error_tsd () = *(*GL(dl_error_catch_tsd)) ();
  GL(dl_error_catch_tsd) = &__libc_dl_error_tsd;

  /* Make __rtld_lock_{,un}lock_recursive use pthread_mutex_{,un}lock,
     keep the lock count from the ld.so implementation.  */
  GL(dl_rtld_lock_recursive) = (void *) __pthread_mutex_lock;
  GL(dl_rtld_unlock_recursive) = (void *) __pthread_mutex_unlock;
  unsigned int rtld_lock_count = GL(dl_load_lock).mutex.__m_count;
  GL(dl_load_lock).mutex.__m_count = 0;
  while (rtld_lock_count-- > 0)
    __pthread_mutex_lock (&GL(dl_load_lock).mutex);
#endif

#ifdef USE_TLS
  GL(dl_init_static_tls) = &__pthread_init_static_tls;
#endif

  /* uClibc-specific stdio initialization for threads. */
  {
    FILE *fp;
    _stdio_user_locking = 0;       /* 2 if threading not initialized */
    for (fp = _stdio_openlist; fp != NULL; fp = fp->__nextopen) {
      if (fp->__user_locking != 1) {
        fp->__user_locking = 0;
      }
    }
  }
}

void __pthread_initialize(void)
{
  pthread_initialize();
}

int __pthread_initialize_manager(void)
{
  int manager_pipe[2];
  int pid;
  struct pthread_request request;
  int report_events;
  pthread_descr mgr;
#ifdef USE_TLS
  tcbhead_t *tcbp;
#endif

  __pthread_multiple_threads = 1;
#if TLS_MULTIPLE_THREADS_IN_TCB || !defined USE_TLS || !TLS_DTV_AT_TP
  __pthread_main_thread->p_multiple_threads = 1;
#endif
  *__libc_multiple_threads_ptr = 1;

#ifndef HAVE_Z_NODELETE
  if (__builtin_expect (&__dso_handle != NULL, 1))
    __cxa_atexit ((void (*) (void *)) pthread_atexit_retcode, NULL,
		  __dso_handle);
#endif

  if (__pthread_max_stacksize == 0)
    __pthread_init_max_stacksize ();
  /* If basic initialization not done yet (e.g. we're called from a
     constructor run before our constructor), do it now */
  if (__pthread_initial_thread_bos == NULL) pthread_initialize();
  /* Setup stack for thread manager */
  __pthread_manager_thread_bos = malloc(THREAD_MANAGER_STACK_SIZE);
  if (__pthread_manager_thread_bos == NULL) return -1;
  __pthread_manager_thread_tos =
    __pthread_manager_thread_bos + THREAD_MANAGER_STACK_SIZE;
  /* Setup pipe to communicate with thread manager */
  if (pipe(manager_pipe) == -1) {
    free(__pthread_manager_thread_bos);
    return -1;
  }

#ifdef USE_TLS
  /* Allocate memory for the thread descriptor and the dtv.  */
  tcbp = _dl_allocate_tls (NULL);
  if (tcbp == NULL) {
    free(__pthread_manager_thread_bos);
    close_not_cancel(manager_pipe[0]);
    close_not_cancel(manager_pipe[1]);
    return -1;
  }

# if defined(TLS_TCB_AT_TP)
  mgr = (pthread_descr) tcbp;
# elif defined(TLS_DTV_AT_TP)
  /* pthread_descr is located right below tcbhead_t which _dl_allocate_tls
     returns.  */
  mgr = (pthread_descr) ((char *) tcbp - TLS_PRE_TCB_SIZE);
# endif
  __pthread_handles[1].h_descr = manager_thread = mgr;

  /* Initialize the descriptor.  */
#if !defined USE_TLS || !TLS_DTV_AT_TP
  mgr->p_header.data.tcb = tcbp;
  mgr->p_header.data.self = mgr;
  mgr->p_header.data.multiple_threads = 1;
#elif TLS_MULTIPLE_THREADS_IN_TCB
  mgr->p_multiple_threads = 1;
#endif
  mgr->p_lock = &__pthread_handles[1].h_lock;
# ifndef HAVE___THREAD
  mgr->p_errnop = &mgr->p_errno;
# endif
  mgr->p_start_args = (struct pthread_start_args) PTHREAD_START_ARGS_INITIALIZER(__pthread_manager);
  mgr->p_nr = 1;
# if __LT_SPINLOCK_INIT != 0
  self->p_resume_count = (struct pthread_atomic) __ATOMIC_INITIALIZER;
# endif
  mgr->p_alloca_cutoff = PTHREAD_STACK_MIN / 4;
#else
  mgr = &__pthread_manager_thread;
#endif

  __pthread_manager_request = manager_pipe[1]; /* writing end */
  __pthread_manager_reader = manager_pipe[0]; /* reading end */

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
      uint32_t event_bits;

#ifdef USE_TLS
      event_bits = THREAD_GETMEM_NC (((pthread_descr) NULL),
				     p_eventbuf.eventmask.event_bits[idx]);
#else
      event_bits = __pthread_initial_thread.p_eventbuf.eventmask.event_bits[idx];
#endif

      if ((mask & (__pthread_threads_events.event_bits[idx] | event_bits))
	  != 0)
	{
	  __pthread_lock(mgr->p_lock, NULL);

#ifdef NEED_SEPARATE_REGISTER_STACK
	  pid = __clone2(__pthread_manager_event,
			 (void **) __pthread_manager_thread_bos,
			 THREAD_MANAGER_STACK_SIZE,
			 CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM,
			 mgr);
#elif defined _STACK_GROWS_UP
	  pid = __clone(__pthread_manager_event,
			(void **) __pthread_manager_thread_bos,
			CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM,
			mgr);
#else
	  pid = __clone(__pthread_manager_event,
			(void **) __pthread_manager_thread_tos,
			CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM,
			mgr);
#endif

	  if (pid != -1)
	    {
	      /* Now fill in the information about the new thread in
	         the newly created thread's data structure.  We cannot let
	         the new thread do this since we don't know whether it was
	         already scheduled when we send the event.  */
	      mgr->p_eventbuf.eventdata = mgr;
	      mgr->p_eventbuf.eventnum = TD_CREATE;
	      __pthread_last_event = mgr;
	      mgr->p_tid = 2* PTHREAD_THREADS_MAX + 1;
	      mgr->p_pid = pid;

	      /* Now call the function which signals the event.  */
	      __linuxthreads_create_event ();
	    }

	  /* Now restart the thread.  */
	  __pthread_unlock(mgr->p_lock);
	}
    }

  if (__builtin_expect (pid, 0) == 0)
    {
#ifdef NEED_SEPARATE_REGISTER_STACK
      pid = __clone2(__pthread_manager, (void **) __pthread_manager_thread_bos,
		     THREAD_MANAGER_STACK_SIZE,
		     CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM, mgr);
#elif defined _STACK_GROWS_UP
      pid = __clone(__pthread_manager, (void **) __pthread_manager_thread_bos,
		    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM, mgr);
#else
      pid = __clone(__pthread_manager, (void **) __pthread_manager_thread_tos,
		    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM, mgr);
#endif
    }
  if (__builtin_expect (pid, 0) == -1) {
#ifdef USE_TLS
    _dl_deallocate_tls (tcbp, true);
#endif
    free(__pthread_manager_thread_bos);
    close_not_cancel(manager_pipe[0]);
    close_not_cancel(manager_pipe[1]);
    return -1;
  }
  mgr->p_tid = 2* PTHREAD_THREADS_MAX + 1;
  mgr->p_pid = pid;
  /* Make gdb aware of new thread manager */
  if (__builtin_expect (__pthread_threads_debug, 0) && __pthread_sig_debug > 0)
    {
      raise(__pthread_sig_debug);
      /* We suspend ourself and gdb will wake us up when it is
	 ready to handle us. */
      __pthread_wait_for_restart_signal(thread_self());
    }
  /* Synchronize debugging of the thread manager */
  request.req_kind = REQ_DEBUG;
  TEMP_FAILURE_RETRY(write_not_cancel(__pthread_manager_request,
				      (char *) &request, sizeof(request)));
  return 0;
}

/* Thread creation */

int __pthread_create(pthread_t *thread, const pthread_attr_t *attr,
			 void * (*start_routine)(void *), void *arg)
{
  pthread_descr self = thread_self();
  struct pthread_request request;
  int retval;
  if (__builtin_expect (__pthread_manager_request, 0) < 0) {
    if (__pthread_initialize_manager() < 0) return EAGAIN;
  }
  request.req_thread = self;
  request.req_kind = REQ_CREATE;
  request.req_args.create.attr = attr;
  request.req_args.create.fn = start_routine;
  request.req_args.create.arg = arg;
  sigprocmask(SIG_SETMASK, NULL, &request.req_args.create.mask);
  TEMP_FAILURE_RETRY(write_not_cancel(__pthread_manager_request,
				      (char *) &request, sizeof(request)));
  suspend(self);
  retval = THREAD_GETMEM(self, p_retcode);
  if (__builtin_expect (retval, 0) == 0)
    *thread = (pthread_t) THREAD_GETMEM(self, p_retval);
  return retval;
}
strong_alias (__pthread_create, pthread_create)

/* Simple operations on thread identifiers */

pthread_descr __pthread_thread_self(void)
{
  return thread_self();
}

pthread_t __pthread_self(void)
{
  pthread_descr self = thread_self();
  return THREAD_GETMEM(self, p_tid);
}
strong_alias (__pthread_self, pthread_self)

int __pthread_equal(pthread_t thread1, pthread_t thread2)
{
  return thread1 == thread2;
}
strong_alias (__pthread_equal, pthread_equal)

/* Helper function for thread_self in the case of user-provided stacks */

#ifndef THREAD_SELF

pthread_descr __pthread_find_self(void)
{
  char * sp = CURRENT_STACK_FRAME;
  pthread_handle h;

  /* __pthread_handles[0] is the initial thread, __pthread_handles[1] is
     the manager threads handled specially in thread_self(), so start at 2 */
  h = __pthread_handles + 2;
# ifdef _STACK_GROWS_UP
  while (! (sp >= (char *) h->h_descr && sp < (char *) h->h_descr->p_guardaddr)) h++;
# else
  while (! (sp <= (char *) h->h_descr && sp >= h->h_bottom)) h++;
# endif
  return h->h_descr;
}

#else

pthread_descr __pthread_self_stack(void)
{
  char *sp = CURRENT_STACK_FRAME;
  pthread_handle h;

  if (sp >= __pthread_manager_thread_bos && sp < __pthread_manager_thread_tos)
    return manager_thread;
  h = __pthread_handles + 2;
# ifdef USE_TLS
#  ifdef _STACK_GROWS_UP
  while (h->h_descr == NULL
	 || ! (sp >= h->h_descr->p_stackaddr && sp < h->h_descr->p_guardaddr))
    h++;
#  else
  while (h->h_descr == NULL
	 || ! (sp <= (char *) h->h_descr->p_stackaddr && sp >= h->h_bottom))
    h++;
#  endif
# else
#  ifdef _STACK_GROWS_UP
  while (! (sp >= (char *) h->h_descr && sp < h->h_descr->p_guardaddr))
    h++;
#  else
  while (! (sp <= (char *) h->h_descr && sp >= h->h_bottom))
    h++;
#  endif
# endif
  return h->h_descr;
}

#endif

/* Thread scheduling */

int __pthread_setschedparam(pthread_t thread, int policy,
                            const struct sched_param *param)
{
  pthread_handle handle = thread_handle(thread);
  pthread_descr th;

  __pthread_lock(&handle->h_lock, NULL);
  if (__builtin_expect (invalid_handle(handle, thread), 0)) {
    __pthread_unlock(&handle->h_lock);
    return ESRCH;
  }
  th = handle->h_descr;
  if (__builtin_expect (__sched_setscheduler(th->p_pid, policy, param) == -1,
			0)) {
    __pthread_unlock(&handle->h_lock);
    return errno;
  }
  th->p_priority = policy == SCHED_OTHER ? 0 : param->sched_priority;
  __pthread_unlock(&handle->h_lock);
  if (__pthread_manager_request >= 0)
    __pthread_manager_adjust_prio(th->p_priority);
  return 0;
}
strong_alias (__pthread_setschedparam, pthread_setschedparam)

int __pthread_getschedparam(pthread_t thread, int *policy,
                            struct sched_param *param)
{
  pthread_handle handle = thread_handle(thread);
  int pid, pol;

  __pthread_lock(&handle->h_lock, NULL);
  if (__builtin_expect (invalid_handle(handle, thread), 0)) {
    __pthread_unlock(&handle->h_lock);
    return ESRCH;
  }
  pid = handle->h_descr->p_pid;
  __pthread_unlock(&handle->h_lock);
  pol = __sched_getscheduler(pid);
  if (__builtin_expect (pol, 0) == -1) return errno;
  if (__sched_getparam(pid, param) == -1) return errno;
  *policy = pol;
  return 0;
}
strong_alias (__pthread_getschedparam, pthread_getschedparam)

/* Process-wide exit() request */

static void pthread_onexit_process(int retcode, void *arg)
{
  if (__builtin_expect (__pthread_manager_request, 0) >= 0) {
    struct pthread_request request;
    pthread_descr self = thread_self();

    /* Make sure we come back here after suspend(), in case we entered
       from a signal handler.  */
    THREAD_SETMEM(self, p_signal_jmp, NULL);

    request.req_thread = self;
    request.req_kind = REQ_PROCESS_EXIT;
    request.req_args.exit.code = retcode;
    TEMP_FAILURE_RETRY(write_not_cancel(__pthread_manager_request,
					(char *) &request, sizeof(request)));
    suspend(self);
    /* Main thread should accumulate times for thread manager and its
       children, so that timings for main thread account for all threads. */
    if (self == __pthread_main_thread)
      {
#ifdef USE_TLS
	waitpid(manager_thread->p_pid, NULL, __WCLONE);
#else
	waitpid(__pthread_manager_thread.p_pid, NULL, __WCLONE);
#endif
	/* Since all threads have been asynchronously terminated
           (possibly holding locks), free cannot be used any more.
           For mtrace, we'd like to print something though.  */
	/* #ifdef USE_TLS
	   tcbhead_t *tcbp = (tcbhead_t *) manager_thread;
	   # if defined(TLS_DTV_AT_TP)
	   tcbp = (tcbhead_t) ((char *) tcbp + TLS_PRE_TCB_SIZE);
	   # endif
	   _dl_deallocate_tls (tcbp, true);
	   #endif
	   free (__pthread_manager_thread_bos); */
	__pthread_manager_thread_bos = __pthread_manager_thread_tos = NULL;
      }
  }
}

#ifndef HAVE_Z_NODELETE
static int __pthread_atexit_retcode;

static void pthread_atexit_process(void *arg, int retcode)
{
  pthread_onexit_process (retcode ?: __pthread_atexit_retcode, arg);
}

static void pthread_atexit_retcode(void *arg, int retcode)
{
  __pthread_atexit_retcode = retcode;
}
#endif

/* The handler for the RESTART signal just records the signal received
   in the thread descriptor, and optionally performs a siglongjmp
   (for pthread_cond_timedwait). */

static void pthread_handle_sigrestart(int sig)
{
  pthread_descr self = check_thread_self();
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
  pthread_descr self = check_thread_self();
  sigjmp_buf * jmpbuf;

  if (self == manager_thread)
    {
      __pthread_manager_sighandler(sig);
      return;
    }
  if (__builtin_expect (__pthread_exit_requested, 0)) {
    /* Main thread should accumulate times for thread manager and its
       children, so that timings for main thread account for all threads. */
    if (self == __pthread_main_thread) {
#ifdef USE_TLS
      waitpid(manager_thread->p_pid, NULL, __WCLONE);
#else
      waitpid(__pthread_manager_thread.p_pid, NULL, __WCLONE);
#endif
    }
    _exit(__pthread_exit_code);
  }
  if (__builtin_expect (THREAD_GETMEM(self, p_canceled), 0)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
    if (THREAD_GETMEM(self, p_canceltype) == PTHREAD_CANCEL_ASYNCHRONOUS)
      __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
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

void __pthread_reset_main_thread(void)
{
  pthread_descr self = thread_self();

  if (__pthread_manager_request != -1) {
    /* Free the thread manager stack */
    free(__pthread_manager_thread_bos);
    __pthread_manager_thread_bos = __pthread_manager_thread_tos = NULL;
    /* Close the two ends of the pipe */
    close_not_cancel(__pthread_manager_request);
    close_not_cancel(__pthread_manager_reader);
    __pthread_manager_request = __pthread_manager_reader = -1;
  }

  /* Update the pid of the main thread */
  THREAD_SETMEM(self, p_pid, __getpid());
  /* Make the forked thread the main thread */
  __pthread_main_thread = self;
  THREAD_SETMEM(self, p_nextlive, self);
  THREAD_SETMEM(self, p_prevlive, self);
#if !(USE_TLS && HAVE___THREAD)
  /* Now this thread modifies the global variables.  */
  THREAD_SETMEM(self, p_errnop, &_errno);
  THREAD_SETMEM(self, p_h_errnop, &_h_errno);
# if defined __UCLIBC_HAS_RESOLVER_SUPPORT__
  THREAD_SETMEM(self, p_resp, __resp);
# endif
#endif

#ifndef FLOATING_STACKS
  /* This is to undo the setrlimit call in __pthread_init_max_stacksize.
     XXX This can be wrong if the user set the limit during the run.  */
 {
   struct rlimit limit;
   if (getrlimit (RLIMIT_STACK, &limit) == 0
       && limit.rlim_cur != limit.rlim_max)
     {
       limit.rlim_cur = limit.rlim_max;
       setrlimit(RLIMIT_STACK, &limit);
     }
 }
#endif
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
  memset(&sa, 0, sizeof(sa));
  if (SIG_DFL) /* if it's constant zero, it's already done */
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
    __pthread_sigsuspend(&mask);	/* Wait for signal.  Must not be a
					   cancellation point. */
  } while (THREAD_GETMEM(self, p_signal) !=__pthread_sig_restart);

  READ_MEMORY_BARRIER(); /* See comment in __pthread_restart_new */
}

#if !__ASSUME_REALTIME_SIGNALS
/* The _old variants are for 2.0 and early 2.1 kernels which don't have RT
   signals.
   On these kernels, we use SIGUSR1 and SIGUSR2 for restart and cancellation.
   Since the restart signal does not queue, we use an atomic counter to create
   queuing semantics. This is needed to resolve a rare race condition in
   pthread_cond_timedwait_relative. */

void __pthread_restart_old(pthread_descr th)
{
  if (pthread_atomic_increment(&th->p_resume_count) == -1)
    kill(th->p_pid, __pthread_sig_restart);
}

void __pthread_suspend_old(pthread_descr self)
{
  if (pthread_atomic_decrement(&self->p_resume_count) <= 0)
    __pthread_wait_for_restart_signal(self);
}

int
__pthread_timedsuspend_old(pthread_descr self, const struct timespec *abstime)
{
  sigset_t unblock, initial_mask;
  int was_signalled = 0;
  sigjmp_buf jmpbuf;

  if (pthread_atomic_decrement(&self->p_resume_count) == 0) {
    /* Set up a longjmp handler for the restart signal, unblock
       the signal and sleep. */

    if (sigsetjmp(jmpbuf, 1) == 0) {
      THREAD_SETMEM(self, p_signal_jmp, &jmpbuf);
      THREAD_SETMEM(self, p_signal, 0);
      /* Unblock the restart signal */
      __sigemptyset(&unblock);
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
	if (reltime.tv_sec < 0 || nanosleep(&reltime, NULL) == 0)
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
    if (pthread_atomic_increment(&self->p_resume_count) != -1) {
      __pthread_wait_for_restart_signal(self);
      pthread_atomic_decrement(&self->p_resume_count); /* should be zero now! */
      /* woke spontaneously and consumed restart signal */
      return 1;
    }
    /* woke spontaneously but did not consume restart---caller must resolve */
    return 0;
  }
  /* woken due to restart signal */
  return 1;
}
#endif /* __ASSUME_REALTIME_SIGNALS */

void __pthread_restart_new(pthread_descr th)
{
  /* The barrier is proabably not needed, in which case it still documents
     our assumptions. The intent is to commit previous writes to shared
     memory so the woken thread will have a consistent view.  Complementary
     read barriers are present to the suspend functions. */
  WRITE_MEMORY_BARRIER();
  kill(th->p_pid, __pthread_sig_restart);
}

/* There is no __pthread_suspend_new because it would just
   be a wasteful wrapper for __pthread_wait_for_restart_signal */

int
__pthread_timedsuspend_new(pthread_descr self, const struct timespec *abstime)
{
  sigset_t unblock, initial_mask;
  int was_signalled = 0;
  sigjmp_buf jmpbuf;

  if (sigsetjmp(jmpbuf, 1) == 0) {
    THREAD_SETMEM(self, p_signal_jmp, &jmpbuf);
    THREAD_SETMEM(self, p_signal, 0);
    /* Unblock the restart signal */
    __sigemptyset(&unblock);
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
      if (reltime.tv_sec < 0 || nanosleep(&reltime, NULL) == 0)
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


/* Debugging aid */

#ifdef DEBUG
#include <stdarg.h>

void __pthread_message(const char * fmt, ...)
{
  char buffer[1024];
  va_list args;
  sprintf(buffer, "%05d : ", __getpid());
  va_start(args, fmt);
  vsnprintf(buffer + 8, sizeof(buffer) - 8, fmt, args);
  va_end(args);
  TEMP_FAILURE_RETRY(write_not_cancel(2, buffer, strlen(buffer)));
}

#endif
