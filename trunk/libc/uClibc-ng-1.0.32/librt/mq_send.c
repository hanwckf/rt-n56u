/*
 * mq_send.c - functions for sending to message queue.
 */

#include <errno.h>
#include <stddef.h>
#include <sys/syscall.h>

#include <mqueue.h>

#ifdef __NR_mq_timedsend
int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio)
{
	return mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, NULL);
}
#endif
