/*
 * Distributed under the terms of the GNU General Public License v2
 * $Header: /home/cvsroot/RT288x_SDK/source/lib/libc/sysdeps/linux/common/ssp.c,v 1.1.1.1 2007-01-09 06:46:10 steven Exp $
 *
 * This is a modified version of Hiroaki Etoh's stack smashing routines
 * implemented for glibc.
 *
 * The following people have contributed input to this code.
 * Ned Ludd - <solar[@]gentoo.org>
 * Alexander Gabert - <pappy[@]gentoo.org>
 * The PaX Team - <pageexec[@]freemail.hu>
 * Peter S. Mazinger - <ps.m[@]gmx.net>
 * Yoann Vandoorselaere - <yoann[@]prelude-ids.org>
 * Robert Connolly - <robert[@]linuxfromscratch.org>
 * Cory Visi <cory@visi.name>
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef __SSP__
# error ssp.c has to be built w/ -fno-stack-protector
#endif

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/syslog.h>
#include <sys/time.h>
#ifdef __SSP_USE_ERANDOM__
# include <sys/sysctl.h>
#endif

#ifdef __PROPOLICE_BLOCK_SEGV__
# define SSP_SIGTYPE SIGSEGV
#elif __PROPOLICE_BLOCK_KILL__
# define SSP_SIGTYPE SIGKILL
#else
# define SSP_SIGTYPE SIGABRT
#endif

unsigned long __guard = 0UL;

/* Use of __* functions from the rest of glibc here avoids
 * initialisation problems for executables preloaded with
 * libraries that overload the associated standard library
 * functions.
 */
#ifdef __UCLIBC__
extern int __libc_open(__const char *file, int flags, ...);
extern ssize_t __libc_read(int fd, void *buf, size_t count);
extern int __libc_close(int fd);
#else
# define __libc_open(file, flags) __open(file, flags)
# define __libc_read(fd, buf, count) __read(fd, buf, count)
# define __libc_close(fd) __close(fd)
#endif

void __guard_setup(void) __attribute__ ((constructor));
void __guard_setup(void)
{
	size_t size;

	if (__guard != 0UL)
		return;

	/* Start with the "terminator canary". */
	__guard = 0xFF0A0D00UL;

#ifndef __SSP_QUICK_CANARY__
# ifdef __SSP_USE_ERANDOM__
	{
		int mib[3];
		/* Random is another depth in Linux, hence an array of 3. */
		mib[0] = CTL_KERN;
		mib[1] = KERN_RANDOM;
		mib[2] = RANDOM_ERANDOM;

		size = sizeof(unsigned long);
		if (__sysctl(mib, 3, &__guard, &size, NULL, 0) != (-1))
			if (__guard != 0UL)
				return;
	}
# endif /* ifdef __SSP_USE_ERANDOM__ */
	/* 
	 * Attempt to open kernel pseudo random device if one exists before 
	 * opening urandom to avoid system entropy depletion.
	 */
	{
		int fd;

# ifdef __SSP_USE_ERANDOM__
		if ((fd = __libc_open("/dev/erandom", O_RDONLY)) == (-1))
# endif
			fd = __libc_open("/dev/urandom", O_RDONLY);
		if (fd != (-1)) {
			size = __libc_read(fd, (char *) &__guard, sizeof(__guard));
			__libc_close(fd);
			if (size == sizeof(__guard))
				return;
		}
	}
#endif /* ifndef __SSP_QUICK_CANARY__ */

	/* Everything failed? Or we are using a weakened model of the 
	 * terminator canary */
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		__guard ^= tv.tv_usec ^ tv.tv_sec;
	}
}

void __stack_smash_handler(char func[], int damaged __attribute__ ((unused)));
void __stack_smash_handler(char func[], int damaged)
{
	extern char *__progname;
	const char message[] = ": stack smashing attack in function ";
	struct sigaction sa;
	sigset_t mask;

	sigfillset(&mask);

	sigdelset(&mask, SSP_SIGTYPE);	/* Block all signal handlers */
	sigprocmask(SIG_BLOCK, &mask, NULL);	/* except SSP_SIGTYPE */

	/* Print error message to stderr and syslog */
	fprintf(stderr, "%s%s%s()\n", __progname, message, func);
	syslog(LOG_INFO, "%s%s%s()", __progname, message, func);

	/* Make the default handler associated with the signal handler */
	memset(&sa, 0, sizeof(struct sigaction));
	sigfillset(&sa.sa_mask);	/* Block all signals */
	sa.sa_flags = 0;
	sa.sa_handler = SIG_DFL;
	sigaction(SSP_SIGTYPE, &sa, NULL);
	(void) kill(getpid(), SSP_SIGTYPE);
	_exit(127);
}
