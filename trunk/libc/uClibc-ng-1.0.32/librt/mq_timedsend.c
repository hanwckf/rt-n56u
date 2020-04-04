/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <mqueue.h>
#include <unistd.h>
#include <cancel.h>

#define __NR___mq_timedsend_nocancel __NR_mq_timedsend
_syscall5(int, __NC(mq_timedsend), mqd_t, mqdes, const char *, msg_ptr, size_t, msg_len, unsigned int, msq_prio, const struct timespec *, abs_timeout)

CANCELLABLE_SYSCALL(int, mq_timedsend, (mqd_t mqdes, const char *msg_ptr, size_t msq_len, unsigned int msq_prio, const struct timespec *abs_timeout),
		    (mqdes, msg_ptr, msq_len, msq_prio, abs_timeout))
lt_libc_hidden(mq_timedsend)
