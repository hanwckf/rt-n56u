/* Copyright (C) 1997,1998,1999,2000,2002,2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.

   Totally hacked up for uClibc by Erik Andersen <andersen@codepoet.org>
   */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
#include <bits/kernel_sigaction.h>

#define SA_RESTORER	0x04000000
extern void __default_sa_restorer(void);
extern void __default_rt_sa_restorer(void);

extern __typeof(sigaction) __libc_sigaction;

/* When RT signals are in use we need to use a different return stub.  */
#ifdef __NR_rt_sigreturn
#define choose_restorer(flags)					\
	(flags & SA_SIGINFO) ? __default_rt_sa_restorer		\
	: __default_sa_restorer
#else
#define choose_restorer(flags)					\
	__default_sa_restorer
#endif


#ifdef __NR_rt_sigaction

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int __libc_sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
	struct sigaction kact;
	if (act && !(act->sa_flags & SA_RESTORER)) {
		memcpy(&kact, act, sizeof(kact));
		kact.sa_restorer = choose_restorer(kact.sa_flags);
		kact.sa_flags |= SA_RESTORER;
		act = &kact;
	}
	/* NB: kernel (as of 2.6.25) will return EINVAL
	 * if sizeof(act->sa_mask) does not match kernel's sizeof(sigset_t) */
	return __syscall_rt_sigaction(sig, act, oact, sizeof(act->sa_mask));
}

#else

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int __libc_sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
	int result;
	struct old_kernel_sigaction kact, koact;

	if (act) {
		kact.k_sa_handler = act->sa_handler;
		kact.sa_mask = act->sa_mask.__val[0];
		kact.sa_flags = act->sa_flags;
		if (kact.sa_flags & SA_RESTORER) {
			kact.sa_restorer = act->sa_restorer;
		} else {
			kact.sa_restorer = choose_restorer(kact.sa_flags);
			kact.sa_flags |= SA_RESTORER;
		}
	}
	result = __syscall_sigaction(sig,
			act ? &kact : NULL,
			oact ? &koact : NULL);
	if (oact && result >= 0) {
		oact->sa_handler = koact.k_sa_handler;
		oact->sa_mask.__val[0] = koact.sa_mask;
		oact->sa_flags = koact.sa_flags;
		oact->sa_restorer = koact.sa_restorer;
	}
	return result;
}

#endif


#ifndef LIBC_SIGACTION
# ifndef __UCLIBC_HAS_THREADS__
strong_alias(__libc_sigaction,sigaction)
libc_hidden_def(sigaction)
# else
weak_alias(__libc_sigaction,sigaction)
libc_hidden_weak(sigaction)
# endif
#endif
