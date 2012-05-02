/* POSIX.1 `sigaction' call for Linux/x86-64.
   Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
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
   02111-1307 USA.  */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
#include <bits/kernel_sigaction.h>

#define SA_RESTORER 0x04000000


#if defined __NR_rt_sigaction
#warning Yes there are two warnings here.  Don't worry about it.
static void restore_rt (void) asm ("__restore_rt");
static void restore (void) asm ("__restore");

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int __libc_sigaction (int sig, const struct sigaction *act, struct sigaction *oact)
{
	int result;
	struct kernel_sigaction kact, koact;

	if (act) {
		kact.k_sa_handler = act->sa_handler;
		memcpy (&kact.sa_mask, &act->sa_mask, sizeof (sigset_t));
		kact.sa_flags = act->sa_flags | SA_RESTORER;

		kact.sa_restorer = &restore_rt;
	}

	/* XXX The size argument hopefully will have to be changed to the
	   real size of the user-level sigset_t.  */
	result = INLINE_SYSCALL (rt_sigaction, 4,
	                         sig, act ? __ptrvalue (&kact) : NULL,
	                         oact ? __ptrvalue (&koact) : NULL, _NSIG / 8);
	if (oact && result >= 0) {
		oact->sa_handler = koact.k_sa_handler;
		memcpy (&oact->sa_mask, &koact.sa_mask, sizeof (sigset_t));
		oact->sa_flags = koact.sa_flags;
		oact->sa_restorer = koact.sa_restorer;
	}
	return result;
}
#else

#warning "Yes there is a warning here.  Don't worry about it."
static void restore (void) asm ("__restore");

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int __libc_sigaction (int sig, const struct sigaction *act, struct sigaction *oact)
{
	int result;
	struct old_kernel_sigaction kact, koact;

#ifdef SIGCANCEL
	if (sig == SIGCANCEL) {
		__set_errno (EINVAL);
		return -1;
	}
#endif

	if (act) {
		kact.k_sa_handler = act->sa_handler;
		kact.sa_mask = act->sa_mask.__val[0];
		kact.sa_flags = act->sa_flags | SA_RESTORER;
		kact.sa_restorer = &restore;
	}

	asm volatile ("syscall\n"
	              : "=a" (result)
	              : "0" (__NR_sigaction), "mr" (sig),
	                "c" (act ? __ptrvalue (&kact) : 0),
	                "d" (oact ? __ptrvalue (&koact) : 0));

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

weak_alias (__libc_sigaction, sigaction)

/* NOTE: Please think twice before making any changes to the bits of
   code below.  GDB needs some intimate knowledge about it to
   recognize them as signal trampolines, and make backtraces through
   signal handlers work right.  Important are both the names
   (__restore_rt) and the exact instruction sequence.
   If you ever feel the need to make any changes, please notify the
   appropriate GDB maintainer.  */

#define RESTORE(name, syscall) RESTORE2 (name, syscall)
# define RESTORE2(name, syscall) \
asm						\
  (						\
   ".text\n" \
   ".align 16\n"				\
   "__" #name ":\n"				\
   "	movq $" #syscall ", %rax\n"		\
   "	syscall\n"				\
   );
#ifdef __NR_rt_sigaction
/* The return code for realtime-signals.  */
RESTORE (restore_rt, __NR_rt_sigreturn)
#endif
#ifdef __NR_sigreturn
RESTORE (restore, __NR_sigreturn)
#endif
