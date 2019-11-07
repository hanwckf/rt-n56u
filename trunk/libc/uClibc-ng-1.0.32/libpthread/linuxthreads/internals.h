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

#ifndef _INTERNALS_H
#define _INTERNALS_H   1

/* Internal data structures */

/* Includes */

#include <bits/libc-tsd.h> /* for _LIBC_TSD_KEY_N */
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <bits/stackinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "pt-machine.h"
#include "semaphore.h"
#include "../linuxthreads_db/thread_dbP.h"
#ifdef __UCLIBC_HAS_XLOCALE__
#include <bits/uClibc_locale.h>
#endif /* __UCLIBC_HAS_XLOCALE__ */

/* Use a funky version in a probably vein attempt at preventing gdb
 * from dlopen()'ing glibc's libthread_db library... */
#define VERSION __stringify(__UCLIBC_MAJOR__) "." __stringify(__UCLIBC_MINOR__) "." __stringify(__UCLIBC_SUBLEVEL__)

#ifndef THREAD_GETMEM
# define THREAD_GETMEM(descr, member) descr->member
#endif
#ifndef THREAD_GETMEM_NC
# define THREAD_GETMEM_NC(descr, member) descr->member
#endif
#ifndef THREAD_SETMEM
# define THREAD_SETMEM(descr, member, value) descr->member = (value)
#endif
#ifndef THREAD_SETMEM_NC
# define THREAD_SETMEM_NC(descr, member, value) descr->member = (value)
#endif

/* Conditional variable attribute data structure.  */
struct pthread_condattr
{
  /* Combination of values:

     Bit 0  : flag whether coditional variable will be shareable between
	      processes.

     Bit 1-7: clock ID.  */
  int value;
};

/* The __NWAITERS field is used as a counter and to house the number
   of bits for other purposes.  COND_CLOCK_BITS is the number
   of bits needed to represent the ID of the clock.  COND_NWAITERS_SHIFT
   is the number of bits reserved for other purposes like the clock.  */
#define COND_CLOCK_BITS		1
#define COND_NWAITERS_SHIFT	1

/* Arguments passed to thread creation routine */

struct pthread_start_args {
  void * (*start_routine)(void *); /* function to run */
  void * arg;                   /* its argument */
  sigset_t mask;                /* initial signal mask for thread */
  int schedpolicy;              /* initial scheduling policy (if any) */
  struct sched_param schedparam; /* initial scheduling parameters (if any) */
};


/* We keep thread specific data in a special data structure, a two-level
   array.  The top-level array contains pointers to dynamically allocated
   arrays of a certain number of data pointers.  So we can implement a
   sparse array.  Each dynamic second-level array has
	PTHREAD_KEY_2NDLEVEL_SIZE
   entries.  This value shouldn't be too large.  */
#define PTHREAD_KEY_2NDLEVEL_SIZE	32

/* We need to address PTHREAD_KEYS_MAX key with PTHREAD_KEY_2NDLEVEL_SIZE
   keys in each subarray.  */
#define PTHREAD_KEY_1STLEVEL_SIZE \
  ((PTHREAD_KEYS_MAX + PTHREAD_KEY_2NDLEVEL_SIZE - 1) \
   / PTHREAD_KEY_2NDLEVEL_SIZE)

typedef void (*destr_function)(void *);

struct pthread_key_struct {
  int in_use;                   /* already allocated? */
  destr_function destr;         /* destruction routine */
};


#define PTHREAD_START_ARGS_INITIALIZER { NULL, NULL, {{0, }}, 0, { 0 } }

/* The type of thread descriptors */

typedef struct _pthread_descr_struct * pthread_descr;

/* Callback interface for removing the thread from waiting on an
   object if it is cancelled while waiting or about to wait.
   This hold a pointer to the object, and a pointer to a function
   which ``extricates'' the thread from its enqueued state.
   The function takes two arguments: pointer to the wait object,
   and a pointer to the thread. It returns 1 if an extrication
   actually occured, and hence the thread must also be signalled.
   It returns 0 if the thread had already been extricated. */

typedef struct _pthread_extricate_struct {
    void *pu_object;
    int (*pu_extricate_func)(void *, pthread_descr);
} pthread_extricate_if;

/* Atomic counter made possible by compare_and_swap */

struct pthread_atomic {
  long p_count;
  int p_spinlock;
};

/* Context info for read write locks. The pthread_rwlock_info structure
   is information about a lock that has been read-locked by the thread
   in whose list this structure appears. The pthread_rwlock_context
   is embedded in the thread context and contains a pointer to the
   head of the list of lock info structures, as well as a count of
   read locks that are untracked, because no info structure could be
   allocated for them. */

struct _pthread_rwlock_t;

typedef struct _pthread_rwlock_info {
  struct _pthread_rwlock_info *pr_next;
  struct _pthread_rwlock_t *pr_lock;
  int pr_lock_count;
} pthread_readlock_info;

struct _pthread_descr_struct {
  pthread_descr p_nextlive, p_prevlive;
                                /* Double chaining of active threads */
  pthread_descr p_nextwaiting;  /* Next element in the queue holding the thr */
  pthread_descr p_nextlock;	/* can be on a queue and waiting on a lock */
  pthread_t p_tid;              /* Thread identifier */
  int p_pid;                    /* PID of Unix process */
  int p_priority;               /* Thread priority (== 0 if not realtime) */
  struct _pthread_fastlock * p_lock; /* Spinlock for synchronized accesses */
  int p_signal;                 /* last signal received */
  sigjmp_buf * p_signal_jmp;    /* where to siglongjmp on a signal or NULL */
  sigjmp_buf * p_cancel_jmp;    /* where to siglongjmp on a cancel or NULL */
  char p_terminated;            /* true if terminated e.g. by pthread_exit */
  char p_detached;              /* true if detached */
  char p_exited;                /* true if the assoc. process terminated */
  void * p_retval;              /* placeholder for return value */
  int p_retcode;                /* placeholder for return code */
  pthread_descr p_joining;      /* thread joining on that thread or NULL */
  struct _pthread_cleanup_buffer * p_cleanup; /* cleanup functions */
  char p_cancelstate;           /* cancellation state */
  char p_canceltype;            /* cancellation type (deferred/async) */
  char p_canceled;              /* cancellation request pending */
  int * p_errnop;               /* pointer to used errno variable */
  int p_errno;                  /* error returned by last system call */
  int * p_h_errnop;             /* pointer to used h_errno variable */
  int p_h_errno;                /* error returned by last netdb function */
  char * p_in_sighandler;       /* stack address of sighandler, or NULL */
  char p_sigwaiting;            /* true if a sigwait() is in progress */
  struct pthread_start_args p_start_args; /* arguments for thread creation */
  void ** p_specific[PTHREAD_KEY_1STLEVEL_SIZE]; /* thread-specific data */
  void * p_libc_specific[_LIBC_TSD_KEY_N]; /* thread-specific data for libc */
  int p_userstack;		/* nonzero if the user provided the stack */
  void *p_guardaddr;		/* address of guard area or NULL */
  size_t p_guardsize;		/* size of guard area */
  pthread_descr p_self;		/* Pointer to this structure */
  int p_nr;                     /* Index of descriptor in __pthread_handles */
  int p_report_events;         /* Nonzero if events must be reported.  */
  td_eventbuf_t p_eventbuf;     /* Data for event.  */
  struct pthread_atomic p_resume_count; /* number of times restart() was
					   called on thread */
  char p_woken_by_cancel;       /* cancellation performed wakeup */
  char p_condvar_avail;         /* flag if conditional variable became avail */
  char p_sem_avail;             /* flag if semaphore became available */
  pthread_extricate_if *p_extricate; /* See above */
  pthread_readlock_info *p_readlock_list;  /* List of readlock info structs */
  pthread_readlock_info *p_readlock_free;  /* Free list of structs */
  int p_untracked_readlock_count;	/* Readlocks not tracked by list */
  /* New elements must be added at the end.  */
#ifdef __UCLIBC_HAS_XLOCALE__
  __locale_t locale; /* thread-specific locale from uselocale() only! */
#endif /* __UCLIBC_HAS_XLOCALE__ */
} __attribute__ ((aligned(32))); /* We need to align the structure so that
				    doubles are aligned properly.  This is 8
				    bytes on MIPS and 16 bytes on MIPS64.
				    32 bytes might give better cache
				    utilization.  */

/* The type of thread handles. */

typedef struct pthread_handle_struct * pthread_handle;

struct pthread_handle_struct {
  struct _pthread_fastlock h_lock; /* Fast lock for sychronized access */
  pthread_descr h_descr;        /* Thread descriptor or NULL if invalid */
  char * h_bottom;              /* Lowest address in the stack thread */
};

/* The type of messages sent to the thread manager thread */

struct pthread_request {
  pthread_descr req_thread;     /* Thread doing the request */
  enum {                        /* Request kind */
    REQ_CREATE, REQ_FREE, REQ_PROCESS_EXIT, REQ_MAIN_THREAD_EXIT,
    REQ_POST, REQ_DEBUG, REQ_KICK
  } req_kind;
  union {                       /* Arguments for request */
    struct {                    /* For REQ_CREATE: */
      const pthread_attr_t * attr; /* thread attributes */
      void * (*fn)(void *);     /*   start function */
      void * arg;               /*   argument to start function */
      sigset_t mask;            /*   signal mask */
    } create;
    struct {                    /* For REQ_FREE: */
      pthread_t thread_id;      /*   identifier of thread to free */
    } free;
    struct {                    /* For REQ_PROCESS_EXIT: */
      int code;                 /*   exit status */
    } exit;
    void * post;                /* For REQ_POST: the semaphore */
  } req_args;
};


/* Signals used for suspend/restart and for cancellation notification.  */

extern int __pthread_sig_restart;
extern int __pthread_sig_cancel;

/* Signal used for interfacing with gdb */

extern int __pthread_sig_debug;

/* Global array of thread handles, used for validating a thread id
   and retrieving the corresponding thread descriptor. Also used for
   mapping the available stack segments. */

extern struct pthread_handle_struct __pthread_handles[PTHREAD_THREADS_MAX];

/* Descriptor of the initial thread */

extern struct _pthread_descr_struct __pthread_initial_thread;

/* Descriptor of the manager thread */

extern struct _pthread_descr_struct __pthread_manager_thread;

/* Descriptor of the main thread */

extern pthread_descr __pthread_main_thread;

/* Limit between the stack of the initial thread (above) and the
   stacks of other threads (below). Aligned on a STACK_SIZE boundary.
   Initially 0, meaning that the current thread is (by definition)
   the initial thread. */

extern char *__pthread_initial_thread_bos;
#ifndef __ARCH_USE_MMU__
/* For non-MMU systems, we have no idea the bounds of the initial thread
 * stack, so we have to track it on the fly relative to other stacks.  Do
 * so by scaling back our assumptions on the limits of the bos/tos relative
 * to the known mid point.  See also the comments in pthread_initialize(). */
extern char *__pthread_initial_thread_tos, *__pthread_initial_thread_mid;
#define NOMMU_INITIAL_THREAD_BOUNDS(tos,bos) \
	do { \
		char *__tos = (tos); \
		char *__bos = (bos); \
		if (__tos >= __pthread_initial_thread_bos && \
		    __bos < __pthread_initial_thread_tos) { \
			if (__bos < __pthread_initial_thread_mid) \
				__pthread_initial_thread_bos = __tos; \
			else \
				__pthread_initial_thread_tos = __bos; \
		} \
	} while (0)
#else
#define NOMMU_INITIAL_THREAD_BOUNDS(tos,bos) /* empty */
#endif /* __ARCH_USE_MMU__ */


/* Indicate whether at least one thread has a user-defined stack (if 1),
   or all threads have stacks supplied by LinuxThreads (if 0). */

extern int __pthread_nonstandard_stacks;

/* File descriptor for sending requests to the thread manager.
   Initially -1, meaning that __pthread_initialize_manager must be called. */

extern int __pthread_manager_request;

/* Other end of the pipe for sending requests to the thread manager. */

extern int __pthread_manager_reader;

/* Limits of the thread manager stack. */

extern char *__pthread_manager_thread_bos;
extern char *__pthread_manager_thread_tos;

/* Pending request for a process-wide exit */

extern int __pthread_exit_requested, __pthread_exit_code;

/* Set to 1 by gdb if we're debugging */

extern volatile int __pthread_threads_debug;

/* Globally enabled events.  */
extern volatile td_thr_events_t __pthread_threads_events;

/* Pointer to descriptor of thread with last event.  */
extern volatile pthread_descr __pthread_last_event;

/* Return the handle corresponding to a thread id */

static __inline__ pthread_handle thread_handle(pthread_t id)
{
  return &__pthread_handles[id % PTHREAD_THREADS_MAX];
}

/* Validate a thread handle. Must have acquired h->h_spinlock before. */

static __inline__ int invalid_handle(pthread_handle h, pthread_t id)
{
  return h->h_descr == NULL || h->h_descr->p_tid != id;
}

/* Fill in defaults left unspecified by pt-machine.h.  */

/* The page size we can get from the system.  This should likely not be
   changed by the machine file but, you never know.  */
#define __PAGE_SIZE  (sysconf (_SC_PAGESIZE))

/* The max size of the thread stack segments.  If the default
   THREAD_SELF implementation is used, this must be a power of two and
   a multiple of __PAGE_SIZE.  */
#ifndef STACK_SIZE
#ifdef __ARCH_USE_MMU__
#define STACK_SIZE  (2 * 1024 * 1024)
#else
#define STACK_SIZE  (4 * __PAGE_SIZE)
#endif
#endif

/* The initial size of the thread stack.  Must be a multiple of __PAGE_SIZE.  */
#ifndef INITIAL_STACK_SIZE
#define INITIAL_STACK_SIZE  (4 * __PAGE_SIZE)
#endif

/* Size of the thread manager stack. The "- 32" avoids wasting space
   with some malloc() implementations. */
#ifndef THREAD_MANAGER_STACK_SIZE
#define THREAD_MANAGER_STACK_SIZE  (2 * __PAGE_SIZE - 32)
#endif

/* The base of the "array" of thread stacks.  The array will grow down from
   here.  Defaults to the calculated bottom of the initial application
   stack.  */
#ifndef THREAD_STACK_START_ADDRESS
#define THREAD_STACK_START_ADDRESS  __pthread_initial_thread_bos
#endif

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#ifndef CURRENT_STACK_FRAME
#define CURRENT_STACK_FRAME  ({ char __csf; &__csf; })
#endif

/* If MEMORY_BARRIER isn't defined in pt-machine.h, assume the
   architecture doesn't need a memory barrier instruction (e.g. Intel
   x86).  Still we need the compiler to respect the barrier and emit
   all outstanding operations which modify memory.  Some architectures
   distinguish between full, read and write barriers.  */
#ifndef MEMORY_BARRIER
#define MEMORY_BARRIER() __asm__ ("" : : : "memory")
#endif
#ifndef READ_MEMORY_BARRIER
#define READ_MEMORY_BARRIER() MEMORY_BARRIER()
#endif
#ifndef WRITE_MEMORY_BARRIER
#define WRITE_MEMORY_BARRIER() MEMORY_BARRIER()
#endif

/* Recover thread descriptor for the current thread */

extern pthread_descr __pthread_find_self (void) __attribute__ ((const)) attribute_hidden;

static __inline__ pthread_descr thread_self (void) __attribute__ ((const));
static __inline__ pthread_descr thread_self (void)
{
#ifdef THREAD_SELF
  return THREAD_SELF;
#else
  char *sp = CURRENT_STACK_FRAME;
#ifdef __ARCH_USE_MMU__
  if (sp >= __pthread_initial_thread_bos)
    return &__pthread_initial_thread;
  else if (sp >= __pthread_manager_thread_bos
	   && sp < __pthread_manager_thread_tos)
    return &__pthread_manager_thread;
  else if (__pthread_nonstandard_stacks)
    return __pthread_find_self();
  else
    return (pthread_descr)(((unsigned long)sp | (STACK_SIZE-1))+1) - 1;
#else
  /* For non-MMU we need to be more careful about the initial thread stack.
   * We refine the initial thread stack bounds dynamically as we allocate
   * the other stack frame such that it doesn't overlap with them. Then
   * we can be sure to pick the right thread according to the current SP */

  /* Since we allow other stack frames to be above or below, we need to
   * treat this case special. When pthread_initialize() wasn't called yet,
   * only the initial thread is there. */
  if (__pthread_initial_thread_bos == NULL) {
      return &__pthread_initial_thread;
  }
  else if (sp >= __pthread_initial_thread_bos
	   && sp < __pthread_initial_thread_tos) {
      return &__pthread_initial_thread;
  }
  else if (sp >= __pthread_manager_thread_bos
	   && sp < __pthread_manager_thread_tos) {
      return &__pthread_manager_thread;
  }
  else {
      return __pthread_find_self();
  }
#endif /* __ARCH_USE_MMU__ */
#endif
}

/* Max number of times we must spin on a spinlock calling sched_yield().
   After MAX_SPIN_COUNT iterations, we put the calling thread to sleep. */

#ifndef MAX_SPIN_COUNT
#define MAX_SPIN_COUNT 50
#endif

/* Duration of sleep (in nanoseconds) when we can't acquire a spinlock
   after MAX_SPIN_COUNT iterations of sched_yield().
   With the 2.0 and 2.1 kernels, this MUST BE > 2ms.
   (Otherwise the kernel does busy-waiting for realtime threads,
    giving other threads no chance to run.) */

#ifndef SPIN_SLEEP_DURATION
#define SPIN_SLEEP_DURATION 2000001
#endif

/* Defined and used in libc.so.  */
extern int __libc_multiple_threads attribute_hidden;

/* Internal global functions */

void __pthread_do_exit (void *retval, char *currentframe)
     __attribute__ ((__noreturn__)) attribute_hidden;
void __pthread_destroy_specifics(void) attribute_hidden;
void __pthread_perform_cleanup(char *currentframe) attribute_hidden;
int __pthread_initialize_manager(void) attribute_hidden;
void __pthread_message(char * fmt, ...)
     __attribute__ ((__format__ (printf, 1, 2))) attribute_hidden;
int __pthread_manager(void *reqfd) attribute_hidden;
int __pthread_manager_event(void *reqfd) attribute_hidden;
void __pthread_manager_sighandler(int sig) attribute_hidden;
void __pthread_reset_main_thread(void) attribute_hidden;
void __fresetlockfiles(void) attribute_hidden;
void __pthread_manager_adjust_prio(int thread_prio) attribute_hidden;
void __pthread_initialize_minimal (void);

extern void __pthread_exit (void *retval)
#if defined NOT_IN_libc && defined IS_IN_libpthread
	attribute_noreturn
#endif
	;

extern int __pthread_attr_setguardsize(pthread_attr_t *__attr,
				       size_t __guardsize) attribute_hidden;
extern int __pthread_attr_getguardsize(const pthread_attr_t *__attr,
				       size_t *__guardsize) attribute_hidden;
extern int __pthread_attr_setstackaddr(pthread_attr_t *__attr,
				       void *__stackaddr) attribute_hidden;
extern int __pthread_attr_getstackaddr(const pthread_attr_t *__attr,
				       void **__stackaddr) attribute_hidden;
extern int __pthread_attr_setstacksize(pthread_attr_t *__attr,
				       size_t __stacksize) attribute_hidden;
extern int __pthread_attr_getstacksize(const pthread_attr_t *__attr,
				       size_t *__stacksize) attribute_hidden;
extern int __pthread_getconcurrency(void) attribute_hidden;
extern int __pthread_setconcurrency(int __level) attribute_hidden;
extern void __pthread_kill_other_threads_np(void) attribute_hidden;

extern void __pthread_restart_old(pthread_descr th) attribute_hidden;
extern void __pthread_suspend_old(pthread_descr self) attribute_hidden;
extern int __pthread_timedsuspend_old(pthread_descr self, const struct timespec *abstime) attribute_hidden;

extern void __pthread_restart_new(pthread_descr th) attribute_hidden;
extern void __pthread_suspend_new(pthread_descr self) attribute_hidden;
extern int __pthread_timedsuspend_new(pthread_descr self, const struct timespec *abstime) attribute_hidden;

extern void __pthread_wait_for_restart_signal(pthread_descr self) attribute_hidden;

/* Global pointers to old or new suspend functions */

extern void (*__pthread_restart)(pthread_descr) attribute_hidden;
extern void (*__pthread_suspend)(pthread_descr) attribute_hidden;

#if defined NOT_IN_libc && defined IS_IN_libpthread
extern __typeof(pthread_mutex_init) __pthread_mutex_init attribute_hidden;
extern __typeof(pthread_mutex_destroy) __pthread_mutex_destroy attribute_hidden;
extern __typeof(pthread_mutex_lock) __pthread_mutex_lock attribute_hidden;
extern __typeof(pthread_mutex_trylock) __pthread_mutex_trylock attribute_hidden;
extern __typeof(pthread_mutex_unlock) __pthread_mutex_unlock attribute_hidden;
#endif

/* Prototypes for some of the new semaphore functions.  */
/*extern int __new_sem_post (sem_t * sem);*/

/* TSD.  */
extern int __pthread_internal_tsd_set (int key, const void * pointer);
extern void * __pthread_internal_tsd_get (int key);
extern void ** __attribute__ ((__const__))
  __pthread_internal_tsd_address (int key);

/* The functions called the signal events.  */
extern void __linuxthreads_create_event (void) attribute_hidden;
extern void __linuxthreads_death_event (void) attribute_hidden;
extern void __linuxthreads_reap_event (void) attribute_hidden;

extern int * __libc_pthread_init (void);

#endif /* internals.h */
