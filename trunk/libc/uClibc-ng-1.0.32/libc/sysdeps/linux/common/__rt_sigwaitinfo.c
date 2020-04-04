/*
 * __rt_sigwaitinfo() for uClibc
 *
 * Copyright (C) 2006 by Steven Hill <sjhill@realitydiluted.com>
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include <sys/syscall.h>

#ifdef __NR_rt_sigtimedwait
# define __need_NULL
# include <stddef.h>
# include <signal.h>
# include <cancel.h>

int sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
	return sigtimedwait(set, info, NULL);
}
/* cancellation handled by sigtimedwait, noop on uClibc */
LIBC_CANCEL_HANDLED();
#endif
