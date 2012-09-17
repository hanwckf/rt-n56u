/*
 * mq_send.c - functions for sending to message queue.
 */

#include <sys/syscall.h>

#ifdef __NR_mq_timedsend

#include <stddef.h>
#include <mqueue.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
# ifndef __UCLIBC_HAS_ADVANCED_REALTIME__
extern int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
			unsigned int msg_prio, const struct timespec *abs_timeout);
# endif
librt_hidden_proto(mq_timedsend)
#else

# define __NR___syscall_mq_timedsend __NR_mq_timedsend
static _syscall5(int, __syscall_mq_timedsend, int, mqdes,
		 const char *, msg_ptr, size_t, msg_len, unsigned int,
		 msg_prio, const void *, abs_timeout)

# ifdef __UCLIBC_HAS_ADVANCED_REALTIME__
/*
 * Add a message to queue. If O_NONBLOCK is set and queue is full, wait
 * for sufficient room in the queue until abs_timeout expires.
 */
int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
		 unsigned int msg_prio, const struct timespec *abs_timeout)
{
	return __syscall_mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio,
				      abs_timeout);
}
# endif
#endif

/* Add a message to queue */
int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
	    unsigned int msg_prio)
{
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	return mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, NULL);
#else
	return __syscall_mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, NULL);
#endif
}

#endif
