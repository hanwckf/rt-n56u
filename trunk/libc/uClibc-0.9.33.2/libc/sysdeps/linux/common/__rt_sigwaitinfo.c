/* vi: set sw=4 ts=4: */
/*
 * __rt_sigwaitinfo() for uClibc
 *
 * Copyright (C) 2006 by Steven Hill <sjhill@realitydiluted.com>
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include <sys/syscall.h>
#include <signal.h>
#include <string.h>

#ifdef __NR_rt_sigtimedwait

# ifdef __UCLIBC_HAS_THREADS_NATIVE__
#  include <sysdep-cancel.h>

static int do_sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
#  ifdef SIGCANCEL
	sigset_t tmpset;

	if (set != NULL && (__builtin_expect (__sigismember (set, SIGCANCEL), 0)
#   ifdef SIGSETXID
		|| __builtin_expect (__sigismember (set, SIGSETXID), 0)
#   endif
		))
	{
		/* Create a temporary mask without the bit for SIGCANCEL set.  */
		// We are not copying more than we have to.
		memcpy (&tmpset, set, _NSIG / 8);
		__sigdelset (&tmpset, SIGCANCEL);
#   ifdef SIGSETXID
		__sigdelset (&tmpset, SIGSETXID);
#   endif
		set = &tmpset;
	}
#  endif

	/* XXX The size argument hopefully will have to be changed to the
	   real size of the user-level sigset_t.  */
	int result = INLINE_SYSCALL (rt_sigtimedwait, 4, set, info,
								 NULL, _NSIG / 8);

	/* The kernel generates a SI_TKILL code in si_code in case tkill is
	   used.  tkill is transparently used in raise().  Since having
	   SI_TKILL as a code is useful in general we fold the results
	   here.  */
	if (result != -1 && info != NULL && info->si_code == SI_TKILL)
		info->si_code = SI_USER;

	return result;
}

/* Return any pending signal or wait for one for the given time.  */
int __sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
	if(SINGLE_THREAD_P)
		return do_sigwaitinfo(set, info);

	int oldtype = LIBC_CANCEL_ASYNC();

	/* XXX The size argument hopefully will have to be changed to the
	   real size of the user-level sigset_t.  */
	int result = do_sigwaitinfo(set, info);

	LIBC_CANCEL_RESET(oldtype);

	return result;
}
# else
#  define __need_NULL
#  include <stddef.h>
#  define __NR___rt_sigwaitinfo __NR_rt_sigtimedwait
static _syscall4(int, __rt_sigwaitinfo, const sigset_t *, set,
				 siginfo_t *, info, const struct timespec *, timeout,
				 size_t, setsize);

int attribute_hidden __sigwaitinfo(const sigset_t * set, siginfo_t * info)
{
	return __rt_sigwaitinfo(set, info, NULL, _NSIG / 8);
}
# endif
libc_hidden_proto(sigwaitinfo)
weak_alias (__sigwaitinfo, sigwaitinfo)
libc_hidden_weak(sigwaitinfo)
#endif
