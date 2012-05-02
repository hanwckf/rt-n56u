/*
 * timer_create.c - create a per-process timer.
 */

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/syscall.h>

#include "kernel-posix-timers.h"

#ifdef __NR_timer_create

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define __NR___syscall_timer_create __NR_timer_create
static inline _syscall3(int, __syscall_timer_create, clockid_t, clock_id,
	struct sigevent *, evp, kernel_timer_t *, ktimerid);

/* Create a per-process timer */
int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *timerid)
{
    int retval;
    kernel_timer_t ktimerid;
    struct sigevent local_evp;
    struct timer *newp;

    /* Notification via a thread is not supported yet */
    if (__builtin_expect(evp->sigev_notify == SIGEV_THREAD, 1))
	return -1;

    /*
     * We avoid allocating too much memory by basically using
     * struct timer as a derived class with the first two elements
     * being in the superclass. We only need these two elements here.
     */
    newp = (struct timer *) malloc(offsetof(struct timer, thrfunc));
    if (newp == NULL)
	return -1;	/* No memory */

    if (evp == NULL) {
	/*
	 * The kernel has to pass up the timer ID which is a userlevel object.
	 * Therefore we cannot leave it up to the kernel to determine it.
	 */
	local_evp.sigev_notify = SIGEV_SIGNAL;
	local_evp.sigev_signo = SIGALRM;
	local_evp.sigev_value.sival_ptr = newp;

	evp = &local_evp;
    }

    retval = __syscall_timer_create(clock_id, evp, &ktimerid);
    if (retval != -1) {
	newp->sigev_notify = (evp != NULL ? evp->sigev_notify : SIGEV_SIGNAL);
	newp->ktimerid = ktimerid;

	*timerid = (timer_t) newp;
    } else {
	/* Cannot allocate the timer, fail */
	free(newp);
	retval = -1;
    }

    return retval;
}

#endif
