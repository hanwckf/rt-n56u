/*
 * timer-getoverr.c - get the timer overrun count.
 */

#include <errno.h>
#include <time.h>
#include <sys/syscall.h>

#include "kernel-posix-timers.h"

#ifdef __NR_timer_getoverrun

#define __NR___syscall_timer_getoverrun __NR_timer_getoverrun
static inline _syscall1(int, __syscall_timer_getoverrun, kernel_timer_t, ktimerid);

/* Get the timer overrun count */
int timer_getoverrun(timer_t timerid)
{
    struct timer *kt = (struct timer *) timerid;

    /* Get the information from the kernel */
    return __syscall_timer_getoverrun(kt->ktimerid);
}

#endif
