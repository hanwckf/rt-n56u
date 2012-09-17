/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
#include <bits/kernel_sigaction.h>

#define SA_RESTORER	0x04000000
extern void __default_rt_sa_restorer(void);

extern __typeof(sigaction) __libc_sigaction;

/*
 * If act is not NULL, change the action for sig to *act.
 * If oact is not NULL, put the old action for sig in *oact.
 */
int __libc_sigaction(int sig, const struct sigaction *act,
		     struct sigaction *oact)
{
	struct sigaction kact;

	if (act && !(act->sa_flags & SA_RESTORER)) {
		memcpy(&kact, act, sizeof(kact));
		kact.sa_restorer = __default_rt_sa_restorer;
		kact.sa_flags |= SA_RESTORER;
		act = &kact;
	}

	/* NB: kernel (as of 2.6.25) will return EINVAL
	 * if sizeof(act->sa_mask) does not match kernel's sizeof(sigset_t) */
	return __syscall_rt_sigaction(sig, act, oact, sizeof(act->sa_mask));
}

#ifndef LIBC_SIGACTION
# ifndef __UCLIBC_HAS_THREADS__
strong_alias(__libc_sigaction,sigaction)
libc_hidden_def(sigaction)
# else
weak_alias(__libc_sigaction,sigaction)
libc_hidden_weak(sigaction)
# endif
#endif
