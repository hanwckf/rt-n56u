/*
 * kernel-posix-timers.h - kernel-dependent definitions for POSIX timers.
 */

#include <features.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#ifdef __UCLIBC_HAS_THREADS__
#include <pthread.h>
#endif

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
/* Nonzero if the system calls are not available.  */
extern int __no_posix_timers attribute_hidden;

/* Callback to start helper thread.  */
extern void __start_helper_thread (void) attribute_hidden;

/* Control variable for helper thread creation.  */
extern pthread_once_t __helper_once attribute_hidden;

/* TID of the helper thread.  */
extern pid_t __helper_tid attribute_hidden;

/* List of active SIGEV_THREAD timers.  */
extern struct timer *__active_timer_sigev_thread attribute_hidden;
/* Lock for the __active_timer_sigev_thread.  */
extern pthread_mutex_t __active_timer_sigev_thread_lock attribute_hidden;
#endif

/* Type of timers in the kernel */
typedef int kernel_timer_t;

/* Internal representation of timer */
struct timer {
    /* Notification mechanism */
    int sigev_notify;

    /* Timer ID returned by the kernel */
    kernel_timer_t ktimerid;

    /*
     * All new elements must be added after ktimerid. And if the thrfunc
     * element is not the third element anymore the memory allocation in
     * timer_create needs to be changed.
     */

    /* Parameters for the thread to be started for SIGEV_THREAD */
    void (*thrfunc) (sigval_t);
    sigval_t sival;
#ifdef __UCLIBC_HAS_THREADS__
    pthread_attr_t attr;
#endif

    /* Next element in list of active SIGEV_THREAD timers. */
    struct timer *next;
};
