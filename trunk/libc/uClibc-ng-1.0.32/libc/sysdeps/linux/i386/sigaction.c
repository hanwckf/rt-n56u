/* Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   see <http://www.gnu.org/licenses/>.

   Totally hacked up for uClibc by Erik Andersen <andersen@codepoet.org>
   */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
#include <bits/kernel_sigaction.h>

#define SA_RESTORER	0x04000000

#if defined __NR_rt_sigaction

extern void restore_rt(void) __asm__ ("__restore_rt") attribute_hidden;
extern void restore(void) __asm__ ("__restore") attribute_hidden;

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int __libc_sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
	struct sigaction kact;

	if (act) {
		memcpy(&kact, act, sizeof(kact));
		kact.sa_flags |= SA_RESTORER;
		kact.sa_restorer = (act->sa_flags & SA_SIGINFO) ? &restore_rt : &restore;
		act = &kact;
	}
	/* NB: kernel (as of 2.6.25) will return EINVAL
	 * if sizeof(act->sa_mask) does not match kernel's sizeof(sigset_t) */
	return __syscall_rt_sigaction(sig, act, oact, sizeof(act->sa_mask));
}

#else

extern void restore(void) __asm__ ("__restore") attribute_hidden;

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int __libc_sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
	int result;
	struct old_kernel_sigaction kact, koact;

	if (act) {
		kact.k_sa_handler = act->sa_handler;
		kact.sa_mask = act->sa_mask.__val[0];
		kact.sa_flags = act->sa_flags | SA_RESTORER;
		kact.sa_restorer = &restore;
	}
	__asm__ __volatile__ (
		"	pushl	%%ebx\n"
		"	movl	%3, %%ebx\n"
		"	int	$0x80\n"
		"	popl	%%ebx\n"
		: "=a" (result), "=m" (koact)
		: "0" (__NR_sigaction), "r" (sig), "m" (kact),
		  "c" (act ? &kact : NULL),
		  "d" (oact ? &koact : NULL));
	if (result < 0) {
		__set_errno(-result);
		return -1;
	}
	if (oact) {
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


/* NOTE: Please think twice before making any changes to the bits of
   code below.  GDB needs some intimate knowledge about it to
   recognize them as signal trampolines, and make backtraces through
   signal handlers work right.  Important are both the names
   (__restore and __restore_rt) and the exact instruction sequence.
   If you ever feel the need to make any changes, please notify the
   appropriate GDB maintainer.  */

#define RESTORE(name, syscall) RESTORE2(name, syscall)

#ifdef __NR_rt_sigaction
/* The return code for realtime-signals.  */
# define RESTORE2(name, syscall) \
__asm__	(						\
	"nop\n"						\
	".text\n"					\
	"__" #name ":\n"				\
	"	movl	$" #syscall ", %eax\n"		\
	"	int	$0x80\n"			\
);
RESTORE(restore_rt, __NR_rt_sigreturn)
#endif

#ifdef __NR_sigreturn
/* For the boring old signals.  */
# undef RESTORE2
# define RESTORE2(name, syscall) \
__asm__ (						\
	"nop\n"						\
	".text\n"					\
	"__" #name ":\n"				\
	"	popl	%eax\n"				\
	"	movl	$" #syscall ", %eax\n"		\
	"	int	$0x80\n"			\
);
RESTORE(restore, __NR_sigreturn)
#endif
