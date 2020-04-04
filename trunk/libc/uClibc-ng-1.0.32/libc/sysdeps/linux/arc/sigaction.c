/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
#include <bits/kernel_sigaction.h>

/*
 * Default sigretrun stub if user doesn't specify SA_RESTORER
 */
extern void __default_rt_sa_restorer(void);

#define SA_RESTORER	0x04000000

/* If @act is not NULL, change the action for @sig to @act.
   If @oact is not NULL, put the old action for @sig in @oact.  */
int
__libc_sigaction (int sig, const struct sigaction *act, struct sigaction *oact)
{
	struct sigaction kact;

	/*
	 * SA_RESTORER is only relevant for act != NULL case
	 * (!act means caller only wants to know @oact)
	 */
	if (act && !(act->sa_flags & SA_RESTORER)) {
		kact.sa_restorer = __default_rt_sa_restorer;
		kact.sa_flags = act->sa_flags | SA_RESTORER;

		kact.sa_handler = act->sa_handler;
		kact.sa_mask = act->sa_mask;

		act = &kact;
	}

	return INLINE_SYSCALL(rt_sigaction, 4,
			sig, act, oact, sizeof(act->sa_mask));
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
