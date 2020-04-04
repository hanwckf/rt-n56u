/*
 * timer_create.c - create a per-process timer.
 */

#include <stddef.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/syscall.h>

#include "kernel-posix-timers.h"

#ifdef __NR_timer_create

#define __NR___syscall_timer_create __NR_timer_create
static __inline__ _syscall3(int, __syscall_timer_create, clockid_t, clock_id,
			struct sigevent *, evp, kernel_timer_t *, ktimerid);

/* Create a per-process timer */
int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t * timerid)
{
	int retval;
	kernel_timer_t ktimerid;
	struct sigevent default_evp;
	struct timer *newp;

	if (evp == NULL) {
		/*
		 * The kernel has to pass up the timer ID which is a userlevel object.
		 * Therefore we cannot leave it up to the kernel to determine it.
		 */
		default_evp.sigev_notify = SIGEV_SIGNAL;
		default_evp.sigev_signo = SIGALRM;
		evp = &default_evp;
	}

	/* Notification via a thread is not supported yet */
	if (__builtin_expect(evp->sigev_notify == SIGEV_THREAD, 1))
		return -1;

	/*
	 * We avoid allocating too much memory by basically using
	 * struct timer as a derived class with the first two elements
	 * being in the superclass. We only need these two elements here.
	 */
	newp = malloc(offsetof(struct timer, thrfunc));
	if (newp == NULL)
		return -1;	/* No memory */
	default_evp.sigev_value.sival_ptr = newp;

	retval = __syscall_timer_create(clock_id, evp, &ktimerid);
	if (retval != -1) {
		newp->sigev_notify = evp->sigev_notify;
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
