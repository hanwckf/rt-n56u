/*
 * kernel-posix-timers.h - kernel-dependent definitions for POSIX timers.
 */

#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <pthread.h>

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
    pthread_attr_t attr;
};
