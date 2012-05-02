/*
 * mq_receive.c - functions for receiving from message queue.
 */

#include <errno.h>
#include <stddef.h>
#include <sys/syscall.h>

#include <mqueue.h>

#ifdef __NR_mq_timedreceive

#define __NR___syscall_mq_timedreceive __NR_mq_timedreceive
static inline _syscall5(int, __syscall_mq_timedreceive, int, mqdes,
	char *, msg_ptr, size_t, msg_len, unsigned int *, msg_prio,
	const void *, abs_timeout);

/*
 * Receive the oldest from highest priority messages.
 * Stop waiting if abs_timeout expires.
 */
ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
			unsigned int *msg_prio,
			const struct timespec *abs_timeout)
{
    return __syscall_mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);
}

/* Receive the oldest from highest priority messages */
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
			unsigned int *msg_prio)
{
    return mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, NULL);
}

#endif
