/*
 * Distributed under the terms of the GNU Lesser General Public License
 * $Header: $
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
 * Cory Visi <cory[@]visi.name>
 * Mike Frysinger <vapier[@]gentoo.org>
 */

#if defined __SSP__ || defined __SSP_ALL__
#error "file must not be compiled with stack protection enabled on it. Use -fno-stack-protector"
#endif

#ifdef __PROPOLICE_BLOCK_SEGV__
# define SSP_SIGTYPE SIGSEGV
#else
# define SSP_SIGTYPE SIGABRT
#endif

#include <string.h>
#include <unistd.h>
#include <signal.h>
#if defined __UCLIBC_HAS_SYSLOG__
#include <sys/syslog.h>

#endif


static void block_signals(void)
{
	struct sigaction sa;
	sigset_t mask;

	__sigfillset(&mask);
	__sigdelset(&mask, SSP_SIGTYPE);	/* Block all signal handlers */
	sigprocmask(SIG_BLOCK, &mask, NULL);	/* except SSP_SIGTYPE */

	/* Make the default handler associated with the signal handler */
	memset(&sa, 0, sizeof(sa));
	__sigfillset(&sa.sa_mask);	/* Block all signals */
	if (SIG_DFL) /* if it's constant zero, it's already done */
		sa.sa_handler = SIG_DFL;
	sigaction(SSP_SIGTYPE, &sa, NULL);
}

static void __cold ssp_write(int fd, const char *msg1, const char *msg2, const char *msg3)
{
	write(fd, msg1, strlen(msg1));
	write(fd, msg2, strlen(msg2));
	write(fd, msg3, strlen(msg3));
	write(fd, "()\n", 3);
#if defined __UCLIBC_HAS_SYSLOG__
	openlog("ssp", LOG_CONS | LOG_PID, LOG_USER);
	syslog(LOG_INFO, "%s%s%s()", msg1, msg2, msg3);
	closelog();
#endif
}

static attribute_noreturn void terminate(void)
{
	(void) kill(getpid(), SSP_SIGTYPE);
	_exit(127);
}

#ifdef __UCLIBC_HAS_SSP_COMPAT__
void __stack_smash_handler(char func[], int damaged __attribute__ ((unused))) attribute_noreturn __cold;
void __stack_smash_handler(char func[], int damaged)
{
	static const char message[] = ": stack smashing attack in function ";

	block_signals();

	ssp_write(STDERR_FILENO, __uclibc_progname, message, func);

	/* The loop is added only to keep gcc happy. */
	while(1)
		terminate();
}
#endif

#ifdef __UCLIBC_HAS_SSP__
void __stack_chk_fail(void) attribute_noreturn __cold;
void __stack_chk_fail(void)
{
	static const char msg1[] = "stack smashing detected: ";
	static const char msg3[] = " terminated";

	block_signals();

	ssp_write(STDERR_FILENO, msg1, __uclibc_progname, msg3);

	/* The loop is added only to keep gcc happy. */
	while(1)
		terminate();
}
#endif

#ifdef __UCLIBC_HAS_FORTIFY__
void __chk_fail(void)
{
	static const char msg1[] = "buffer overflow detected: ";
	static const char msg3[] = " terminated";

	block_signals();

	ssp_write(STDERR_FILENO, msg1, __uclibc_progname, msg3);

	/* The loop is added only to keep gcc happy. */
	while(1)
		terminate();
}
libc_hidden_def(__chk_fail)
#endif
