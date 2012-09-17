/*
 * mq_unlink.c - remove a message queue.
 */

#include <errno.h>
#include <sys/syscall.h>

#include <mqueue.h>

#ifdef __NR_mq_unlink

#define __NR___syscall_mq_unlink __NR_mq_unlink
static inline _syscall1(int, __syscall_mq_unlink, const char *, name);

/* Remove message queue */
int mq_unlink(const char *name)
{
    int ret;
    if (name[0] != '/') {
	__set_errno(EINVAL);
	return -1;
    }

    ret = __syscall_mq_unlink(name + 1);

    /* While unlink can return either EPERM or EACCES, mq_unlink should return just EACCES.  */
    if (ret < 0) {
	ret = errno;
	if (ret == EPERM)
	    ret = EACCES;
	__set_errno(ret);
	ret = -1;
    }

    return ret;
}

#endif
