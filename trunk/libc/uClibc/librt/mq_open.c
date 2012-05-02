/*
 * mq_open.c - open a message queue.
 */

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/syscall.h>

#include <mqueue.h>

#ifdef __NR_mq_open

#define __NR___syscall_mq_open __NR_mq_open
static inline _syscall4(int, __syscall_mq_open, const char *, name,
	int, oflag, __kernel_mode_t, mode, void *, attr);
/*
 * Establish connection between a process and a message queue and
 * return message queue descriptor or (mqd_t) -1 on error.
 * oflag determines the type of access used. If O_CREAT is on oflag, the
 * third argument is taken as a `mode_t', the mode of the created
 * message queue, and the fourth argument is taken as `struct mq_attr *',
 * pointer to message queue attributes.
 * If the fourth argument is NULL, default attributes are used.
 */
mqd_t mq_open(const char *name, int oflag, ...)
{
    mode_t mode;
    struct mq_attr *attr;

    if (name[0] != '/') {
	__set_errno(EINVAL);
	return -1;
    }

    mode = 0;
    attr = NULL;

    if (oflag & O_CREAT) {
	va_list ap;

	va_start(ap, oflag);
	mode = va_arg(ap, mode_t);
	attr = va_arg(ap, struct mq_attr *);

	va_end(ap);
    }

    return __syscall_mq_open(name + 1, oflag, mode, attr);
}

#endif
