/*
 * timer_delete.c - delete a per-process timer.
 */

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/syscall.h>

#include "kernel-posix-timers.h"

#ifdef __NR_timer_delete

#define __NR___syscall_timer_delete __NR_timer_delete
static inline _syscall1(int, __syscall_timer_delete, kernel_timer_t, ktimerid);

/* Delete a per-process timer */
int timer_delete(timer_t timerid)
{
    int res;
    struct timer *kt = (struct timer *) timerid;

    /* Delete the kernel timer object */
    res = __syscall_timer_delete(kt->ktimerid);
    if (res == 0) {
	free(kt);	/* Free the memory */
	return 0;
    }

    return -1;
}

#endif
