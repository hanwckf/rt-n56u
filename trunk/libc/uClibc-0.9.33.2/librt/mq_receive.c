/*
 * mq_receive.c - functions for receiving from message queue.
 */

#include <sys/syscall.h>

#ifdef __NR_mq_timedreceive

#include <stddef.h>
#include <mqueue.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
# ifndef __UCLIBC_HAS_ADVANCED_REALTIME__
extern ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
			       unsigned int *msg_prio,
			       const struct timespec *abs_timeout);
# endif
librt_hidden_proto(mq_timedreceive)
#else

# define __NR___syscall_mq_timedreceive __NR_mq_timedreceive
static _syscall5(int, __syscall_mq_timedreceive, int, mqdes,
		 char *, msg_ptr, size_t, msg_len, unsigned int *,
		 msg_prio, const void *, abs_timeout)

# ifdef __UCLIBC_HAS_ADVANCED_REALTIME__
/*
 * Receive the oldest from highest priority messages.
 * Stop waiting if abs_timeout expires.
 */
ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
			unsigned int *msg_prio,
			const struct timespec *abs_timeout)
{
	return __syscall_mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio,
					 abs_timeout);
}
# endif

#endif

/* Receive the oldest from highest priority messages */
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
		   unsigned int *msg_prio)
{
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	return mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, NULL);
#else
	return __syscall_mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, NULL);
#endif
}

#endif
