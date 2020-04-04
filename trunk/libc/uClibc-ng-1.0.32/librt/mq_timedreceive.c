/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <mqueue.h>
#include <unistd.h>
#include <cancel.h>

#define __NR___mq_timedreceive_nocancel __NR_mq_timedreceive
_syscall5(ssize_t, __NC(mq_timedreceive), mqd_t, mqdes, char *__restrict, msg_ptr, size_t, msg_len, unsigned int *__restrict, msq_prio, const struct timespec *__restrict, abs_timeout)

CANCELLABLE_SYSCALL(ssize_t, mq_timedreceive, (mqd_t mqdes, char *__restrict msg_ptr, size_t msq_len, unsigned int *__restrict msq_prio, const struct timespec *__restrict abs_timeout),
		    (mqdes, msg_ptr, msq_len, msq_prio, abs_timeout))
lt_libc_hidden(mq_timedreceive)
