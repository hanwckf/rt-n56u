/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*
 *  Sep 11, 2003
 *  Patch by Atsushi Nemoto <anemo@mba.ocn.ne.jp> to do arch-required
 *  mapping of signal strings (alpha, mips, hppa, sparc).
 */

/* TODO: make a threadsafe version? */

#include <features.h>
#include <string.h>
#include <bits/uClibc_uintmaxtostr.h>
#include <signal.h>

#define _SYS_NSIG			32

#ifdef __UCLIBC_HAS_SIGNUM_MESSAGES__
# define _SYS_SIGMSG_MAXLEN	25
#else
# define _SYS_SIGMSG_MAXLEN	0
#endif

#if _SYS_SIGMSG_MAXLEN < __UIM_BUFLEN_INT + 15
# define _STRSIGNAL_BUFSIZE (__UIM_BUFLEN_INT + 15)
#else
# define _STRSIGNAL_BUFSIZE _SYS_SIGMSG_MAXLEN
#endif

#ifdef __UCLIBC_HAS_SIGNUM_MESSAGES__

extern const char _string_syssigmsgs[] attribute_hidden;

#if defined(__alpha__) || defined(__mips__) || defined(__hppa__) || defined(__sparc__)
static const unsigned char sstridx[] = {
	0,
	SIGHUP,
	SIGINT,
	SIGQUIT,
	SIGILL,
	SIGTRAP,
	SIGIOT,
	SIGBUS,
	SIGFPE,
	SIGKILL,
	SIGUSR1,
	SIGSEGV,
	SIGUSR2,
	SIGPIPE,
	SIGALRM,
	SIGTERM,
#if defined SIGSTKFLT
	SIGSTKFLT,
#else
	0,
#endif
	SIGCHLD,
	SIGCONT,
	SIGSTOP,
	SIGTSTP,
	SIGTTIN,
	SIGTTOU,
	SIGURG,
	SIGXCPU,
	SIGXFSZ,
	SIGVTALRM,
	SIGPROF,
	SIGWINCH,
	SIGIO,
	SIGPWR,
	SIGSYS,
#if defined SIGEMT
	SIGEMT,
#endif
};
#endif

char *strsignal(int signum)
{
	register char *s;
	int i;
	static char buf[_STRSIGNAL_BUFSIZE];
	static const char unknown[] = {
		'U', 'n', 'k', 'n', 'o', 'w', 'n', ' ', 's', 'i', 'g', 'n', 'a', 'l', ' '
	};

#if defined(__alpha__) || defined(__mips__) || defined(__hppa__) || defined(__sparc__)
	/* Need to translate signum to string index. */
	for (i = 0; i < sizeof(sstridx)/sizeof(sstridx[0]); i++) {
		if (sstridx[i] == signum) {
			goto GOT_SSTRIDX;
		}
	}
	i = INT_MAX;	/* Failed. */
 GOT_SSTRIDX:
#else
	/* No signum to string index translation needed. */
	i = signum;
#endif

	if (((unsigned int) signum) < _SYS_NSIG) {
		/* Trade time for space.  This function should rarely be called
		 * so rather than keeping an array of pointers for the different
		 * messages, just run through the buffer until we find the
		 * correct string. */
		for (s = (char *) _string_syssigmsgs; i; ++s) {
			if (!*s) {
				--i;
			}
		}
		if (*s) {		/* Make sure we have an actual message. */
			goto DONE;
		}
	}

	s = _int10tostr(buf + sizeof(buf)-1, signum) - sizeof(unknown);
	memcpy(s, unknown, sizeof(unknown));

 DONE:
	return s;
}

#else  /* __UCLIBC_HAS_SIGNUM_MESSAGES__ */

char *strsignal(int signum)
{
	static char buf[_STRSIGNAL_BUFSIZE];
	static const char unknown[] = {
		'U', 'n', 'k', 'n', 'o', 'w', 'n', ' ', 's', 'i', 'g', 'n', 'a', 'l', ' '
	};

	return memcpy(_int10tostr(buf + sizeof(buf)-1, signum) - sizeof(unknown),
						   unknown, sizeof(unknown));
}

#endif /* __UCLIBC_HAS_SIGNUM_MESSAGES__ */

libc_hidden_def(strsignal)
