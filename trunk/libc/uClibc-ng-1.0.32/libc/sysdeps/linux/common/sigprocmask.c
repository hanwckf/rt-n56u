/*
 * sigprocmask() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __USE_POSIX
#include <signal.h>
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
# include <pthreadP.h>	/* SIGCANCEL */
#endif

#undef sigprocmask

libc_hidden_proto(sigprocmask)

#ifdef __NR_rt_sigprocmask

# define __NR___rt_sigprocmask __NR_rt_sigprocmask
static __always_inline
_syscall4(int, __rt_sigprocmask, int, how, const sigset_t *, set,
		  sigset_t *, oldset, size_t, size)

int sigprocmask(int how, const sigset_t * set, sigset_t * oldset)
{
#ifdef SIGCANCEL
	sigset_t local_newmask;

	/*
	 * The only thing we have to make sure here is that SIGCANCEL and
	 * SIGSETXID are not blocked.
	 */
	if (set != NULL && (unlikely (__sigismember (set, SIGCANCEL))
# ifdef SIGSETXID
		|| unlikely (__sigismember (set, SIGSETXID))
# endif
		))
	{
		local_newmask = *set;
		__sigdelset (&local_newmask, SIGCANCEL);
# ifdef SIGSETXID
		__sigdelset (&local_newmask, SIGSETXID);
# endif
		set = &local_newmask;
	}
#endif

	return __rt_sigprocmask(how, set, oldset, __SYSCALL_SIGSET_T_SIZE);
}


#else

# define __NR___syscall_sigprocmask __NR_sigprocmask
static __always_inline
_syscall3(int, __syscall_sigprocmask, int, how, const sigset_t *, set,
		  sigset_t *, oldset)

int sigprocmask(int how, const sigset_t * set, sigset_t * oldset)
{
#ifdef SIGCANCEL
	sigset_t local_newmask;

	/*
	 * The only thing we have to make sure here is that SIGCANCEL and
	 * SIGSETXID are not blocked.
	 */
	if (set != NULL && (unlikely (__sigismember (set, SIGCANCEL))
# ifdef SIGSETXID
		|| unlikely (__sigismember (set, SIGSETXID))
# endif
		))
	{
		local_newmask = *set;
		__sigdelset (&local_newmask, SIGCANCEL);
# ifdef SIGSETXID
		__sigdelset (&local_newmask, SIGSETXID);
# endif
		set = &local_newmask;
	}
#endif

	return (__syscall_sigprocmask(how, set, oldset));
}
#endif
libc_hidden_def(sigprocmask)
#endif
