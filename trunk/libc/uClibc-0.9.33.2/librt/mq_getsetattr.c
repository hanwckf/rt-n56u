/*
 * mq_getattr.c - get message queue attributes.
 */

#include <errno.h>
#include <stddef.h>
#include <sys/syscall.h>

#include <mqueue.h>

#ifdef __NR_mq_getsetattr

librt_hidden_proto(mq_setattr)
#define __NR___syscall_mq_getsetattr __NR_mq_getsetattr
static __inline__ _syscall3(int, __syscall_mq_getsetattr, int, mqdes,
			const void *, mqstat, void *, omqstat);

/*
 * Set attributes associated with message queue (and possibly also get
 * its old attributes)
 */
int mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat,
               struct mq_attr *omqstat)
{
	return __syscall_mq_getsetattr(mqdes, mqstat, omqstat);
}

librt_hidden_def(mq_setattr)

/* Query status and attributes of message queue */
int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
{
	return mq_setattr(mqdes, NULL, mqstat);
}

#endif
