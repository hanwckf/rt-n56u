/*
 * mq_receive.c - functions for receiving from message queue.
 */

#include <errno.h>
#include <stddef.h>
#include <sys/syscall.h>

#include <mqueue.h>

#ifdef __NR_mq_timedreceive
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio)
{
	return mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, NULL);
}
#endif
