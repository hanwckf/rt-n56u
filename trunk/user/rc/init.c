/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <paths.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <nvram/bcmnvram.h>

#include "rc.h"

#define loop_forever() do { sleep(1); } while (1)
#define SHELL "/bin/sh"

/* States */
enum {
	IDLE,
	TIMER,
	SERVICE,
};

static int noconsole = 0;
static int state = IDLE;
static int signalled = -1;

static int fatal_signals[] = {
	SIGQUIT,
	SIGILL,
	SIGABRT,
	SIGFPE,
	SIGPIPE,
	SIGBUS,
	SIGSEGV,
	SIGSYS,
	SIGTRAP,
	SIGPWR,
	SIGTERM
};

static const char *const environment[] = {
	"HOME=/",
	"PATH=/sbin:/usr/sbin:/bin:/usr/bin",
	"SHELL=/bin/sh",
	"USER=admin",
	NULL
};


/* Set terminal settings to reasonable defaults */
static void 
set_term(int fd)
{
	struct termios tty;

	tcgetattr(fd, &tty);

	/* set control chars */
	tty.c_cc[VINTR]  = 3;	/* C-c */
	tty.c_cc[VQUIT]  = 28;	/* C-\ */
	tty.c_cc[VERASE] = 127; /* C-? */
	tty.c_cc[VKILL]  = 21;	/* C-u */
	tty.c_cc[VEOF]   = 4;	/* C-d */
	tty.c_cc[VSTART] = 17;	/* C-q */
	tty.c_cc[VSTOP]  = 19;	/* C-s */
	tty.c_cc[VSUSP]  = 26;	/* C-z */

	/* use line dicipline 0 */
	tty.c_line = 0;

	/* Make it be sane */
	tty.c_cflag &= CBAUD|CBAUDEX|CSIZE|CSTOPB|PARENB|PARODD;
	tty.c_cflag |= CREAD|HUPCL|CLOCAL;


	/* input modes */
	tty.c_iflag = ICRNL | IXON | IXOFF;

	/* output modes */
	tty.c_oflag = OPOST | ONLCR;

	/* local modes */
	tty.c_lflag =
		ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;

	tcsetattr(fd, TCSANOW, &tty);
}

static int
console_init(void)
{
	int fd;

	/* Clean up */
	ioctl(0, TIOCNOTTY, 0);
	close(0);
	close(1);
	close(2);
	setsid();

	/* Reopen console */
	if ((fd = open(_PATH_CONSOLE, O_RDWR)) < 0) {
		perror(_PATH_CONSOLE);
		return errno;
	}
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);

	ioctl(0, TIOCSCTTY, 1);
	tcsetpgrp(0, getpgrp());
	set_term(0);

	return 0;
}

#undef forkexit_or_rexec
void forkexit_or_rexec(void)
{
	pid_t pid;
	pid = fork();
	//if (pid < 0) /* wtf? */
	//	bb_perror_msg_and_die("fork");
	if (pid) /* parent */
		exit(EXIT_SUCCESS);
	/* child */
}
#define forkexit_or_rexec(argv) forkexit_or_rexec()

static pid_t
run_shell(int timeout, int nowait)
{
	pid_t pid;
	char tz[128];
	char *envp[] = {
		"TERM=vt100",
		"HOME=/",
		"PATH=/usr/bin:/bin:/usr/sbin:/sbin",
		"SHELL=" SHELL,
		"USER=root",
		tz,
		NULL
	};
	int sig;

	/* Wait for user input */
	//cprintf("Hit enter to continue...");
	//if (waitfor (STDIN_FILENO, timeout) <= 0)	// disable for tmp
	//	return 0;

	switch ((pid = fork())) {
	case -1:
		perror("fork");
		return 0;
	case 0:
		/* Reset signal handlers set for parent process */
		for (sig = 0; sig < (_NSIG-1); sig++)
			signal(sig, SIG_DFL);

		/* Reopen console */
		console_init();

		/* Pass on TZ */
		time_zone_x_mapping();
		snprintf(tz, sizeof(tz), "TZ=%s", nvram_safe_get("time_zone_x"));

		/* Now run it.  The new program will take over this PID, 
		 * so nothing further in init.c should be run. */
		execve(SHELL, (char *[]) { "/bin/sh", NULL }, envp);

		/* We're still here?  Some error happened. */
		perror(SHELL);
		exit(errno);
	default:
		if (nowait)
			return pid;
		else {
			waitpid(pid, NULL, 0);
			return 0;
		}
	}
}

void sys_exit(void)
{
	kill(1, SIGTERM);
}

int is_system_down(void)
{
	return check_if_file_exist(FLAG_FILE_REBOOT);
}

static void
fatal_signal(int sig)
{
	int i;

	dbG("sig: %d 0x%x\n", sig, sig);

	/* Disable signal handlers */
	for (i = 0; i < (_NSIG-1); i++)
		signal(i, SIG_DFL);

	create_file(FLAG_FILE_REBOOT);

	// Stop all
	shutdown_router();

	kill(-1, SIGTERM);
	sleep(1);

	sync();

	reboot(RB_AUTOBOOT);

	loop_forever();
}

static void
reap(int sig)
{
	pid_t pid;

	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
		dprintf("Reaped %d\n", pid);
}

void
signal_init(void)
{
	int i;

	for (i = 0; i < sizeof(fatal_signals)/sizeof(fatal_signals[0]); i++)
		signal(fatal_signals[i], fatal_signal);

	signal(SIGCHLD, reap);
}


/* Signal handling */
static void
rc_signal(int sig)
{
	dbG("[init] catch signal: %d\n", sig);
	
	if (sig == SIGHUP) {
		;
	} else if (sig == SIGUSR1) {
		signalled = SERVICE;
	} else if (sig == SIGUSR2) {
		;
	} else if (sig == SIGALRM) {
		signalled = TIMER;
	} else if (sig == SIGINT) {
		;
	}
}

/* Main loop */
void init_main_loop(void)
{
	pid_t shell_pid = 0;
	sigset_t sigset;
	struct tm stm;
	time_t st;

	time(&st);
	localtime_r(&st, &stm);
	stm.tm_year = (SYS_START_YEAR - 1900);
	st = mktime(&stm);
	stime(&st);

	/* Basic initialization */
	umask(0000);
	system("dev_init.sh");

#if !defined (USE_HW_NAT_V2)
	mknod("/dev/acl0", S_IFCHR | 0666, makedev(230, 0));
	mknod("/dev/ac0", S_IFCHR | 0666, makedev(240, 0));
	mknod("/dev/mtr0", S_IFCHR | 0666, makedev(250, 0));
#endif

	fput_int("/proc/sys/net/ipv4/conf/all/rp_filter", 0); // new logic for new kernels
#if BOARD_RAM_SIZE > 128
	fput_int("/proc/sys/vm/min_free_kbytes", 16384);
#elif BOARD_RAM_SIZE > 64
	fput_int("/proc/sys/vm/min_free_kbytes", 8192);
#elif BOARD_RAM_SIZE > 32
	fput_int("/proc/sys/vm/min_free_kbytes", 4096);
#else
	fput_int("/proc/sys/vm/min_free_kbytes", 2048);
#endif

	fput_int("/proc/sys/vm/overcommit_memory", 0);
#if defined (USE_IPV6)
	control_if_ipv6_all(0);
#endif
	/* Setup console */
	if (console_init())
		noconsole = 1;
	
	chdir("/");
	setsid();
	
	{
		const char *const *e;
		/* Make sure environs is set to something sane */
		for (e = environment; *e; e++)
			putenv((char *) *e);
	}
	
	/* Setup signal handlers */
	signal_init();
	signal(SIGHUP,  rc_signal);
	signal(SIGUSR1, rc_signal);
	signal(SIGUSR2, rc_signal);
	signal(SIGINT,  rc_signal);
	signal(SIGALRM, rc_signal);
	sigemptyset(&sigset);
	
	init_router();
	
	/* Loop forever */
	for (;;) {
		switch (state) {
		case SERVICE:
			if (!is_system_down())
				handle_notifications();
			state = IDLE;
			break;
		case TIMER:
			if (!is_system_down())
				on_deferred_hotplug_usb();
			state = IDLE;
			break;
		case IDLE:
			/* Wait for user input or state change */
			while (signalled == -1) {
				if (!noconsole && (!shell_pid || kill(shell_pid, 0) != 0))
				{
					shell_pid = run_shell(0, 1);
				}
				else
				{
					sigsuspend(&sigset);
				}
			}
			state = signalled;
			signalled = -1;
			break;
		default:
			return;
		}
	}
}
