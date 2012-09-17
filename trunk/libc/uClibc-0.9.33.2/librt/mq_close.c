/*
 * mq_close.c - close a message queue.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <mqueue.h>

#ifdef __NR_mq_open

/*
 * Remove the association between message queue descriptor and its
 * message queue.
 */
int mq_close(mqd_t mqdes)
{
	return close(mqdes);
}

#endif
