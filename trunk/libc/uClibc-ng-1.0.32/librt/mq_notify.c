/*
 * mq_notify.c - notify process that a message is available.
 */

#include <errno.h>
#include <stddef.h>
#include <sys/syscall.h>

#include <mqueue.h>

#ifdef __NR_mq_notify

#define __NR___syscall_mq_notify __NR_mq_notify
static __inline__ _syscall2(int, __syscall_mq_notify, int, mqdes,
			const void *, notification);

/* Register notification upon message arrival to an empty message queue */
int mq_notify(mqd_t mqdes, const struct sigevent *notification)
{
	/* We don't support SIGEV_THREAD notification yet */
	if (notification != NULL && notification->sigev_notify == SIGEV_THREAD) {
		__set_errno(ENOSYS);
		return -1;
	}
	return __syscall_mq_notify(mqdes, notification);
}

#endif
