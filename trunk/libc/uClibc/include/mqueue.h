/*
 * mqueue.h - definitions and function prototypes for POSIX mqueue support.
 */

#ifndef _MQUEUE_H
#define _MQUEUE_H

#include <features.h>
#include <sys/types.h>
#include <fcntl.h>
#define __need_sigevent_t
#include <bits/siginfo.h>
#define __need_timespec
#include <time.h>

typedef int mqd_t;

struct mq_attr {
    long int mq_flags;		/* Message queue flags */
    long int mq_maxmsg;		/* Maximum number of messages */
    long int mq_msgsize;	/* Maximum message size */
    long int mq_curmsgs;	/* Number of messages currently queued */
    long int __pad[4];
};

__BEGIN_DECLS

/*
 * Establish connection between a process and a message queue __name and
 * return message queue descriptor or (mqd_t) -1 on error. __oflag determines
 * the type of access used. If O_CREAT is on __oflag, the third argument is
 * taken as a `mode_t', the mode of the created message queue, and the fourth
 * argument is taken as `struct mq_attr *', pointer to message queue
 * attributes. If the fourth argument is NULL, default attributes are used.
 */
extern mqd_t mq_open(const char *__name, int __oflag, ...) __THROW;

/*
 * Remove the association between message queue descriptor __mqdes and its
 * message queue.
 */
extern int mq_close(mqd_t __mqdes) __THROW;

/* Query status and attributes of message queue __mqdes */
extern int mq_getattr(mqd_t __mqdes, struct mq_attr *__mqstat) __THROW;

/*
 * Set attributes associated with message queue __mqdes and if __omqstat is
 * not NULL also query its old attributes.
 */
extern int mq_setattr(mqd_t __mqdes,
		      const struct mq_attr *__restrict __mqstat,
		      struct mq_attr *__restrict __omqstat) __THROW;

/* Remove message queue named __name */
extern int mq_unlink(const char *__name) __THROW;

/*
 * Register notification upon message arrival to an empty message queue
 * __mqdes
 */
extern int mq_notify(mqd_t __mqdes, const struct sigevent *__notification)
     __THROW;

/*
 * Receive the oldest from highest priority messages in message queue
 * __mqdes
 */
extern ssize_t mq_receive(mqd_t __mqdes, char *__msg_ptr, size_t __msg_len,
			  unsigned int *__msg_prio);

/* Add message pointed by __msg_ptr to message queue __mqdes */
extern int mq_send(mqd_t __mqdes, const char *__msg_ptr, size_t __msg_len,
		   unsigned int __msg_prio);

#ifdef __USE_XOPEN2K
/*
 * Receive the oldest from highest priority messages in message queue
 * __mqdes, stop waiting if __abs_timeout expires.
 */
extern ssize_t mq_timedreceive(mqd_t __mqdes, char *__restrict __msg_ptr,
			       size_t __msg_len,
			       unsigned int *__restrict __msg_prio,
			       const struct timespec *__restrict __abs_timeout);

/*
 * Add message pointed by __msg_ptr to message queue __mqdes, stop blocking
 * on full message queue if __abs_timeout expires.
 */
extern int mq_timedsend(mqd_t __mqdes, const char *__msg_ptr,
			size_t __msg_len, unsigned int __msg_prio,
			const struct timespec *__abs_timeout);
#endif

__END_DECLS

#endif
