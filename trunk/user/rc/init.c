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
 *
 * System V init functionality
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
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
#include <sys/wait.h>
#include <sys/time.h>
#include <shutils.h>
#include <nvram/bcmnvram.h>

#include "rc.h"

#define loop_forever() do { sleep(1); } while (1)
#define SHELL "/bin/sh"

enum {
	DAEMON_CHDIR_ROOT = 1,
	DAEMON_DEVNULL_STDIO = 2,
	DAEMON_CLOSE_EXTRA_FDS = 4,
	DAEMON_ONLY_SANITIZE = 8, /* internal use */
};

/* Set terminal settings to reasonable defaults */
static void set_term(int fd)
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

int
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

pid_t
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

static void
shutdown_system(void)
{
	int sig;

	/* Disable signal handlers */
	for (sig = 0; sig < (_NSIG-1); sig++)
		signal(sig, SIG_DFL);

	// Stop all (except lan)
	stop_misc();
	stop_services(1);
	stop_usb();
	stop_wan();
	stop_services_lan_wan();
	write_storage_to_mtd();
	stop_logger();
	stop_wifi_radio_wl();
	stop_wifi_radio_rt();
	
	cprintf("Sending SIGTERM to all processes\n");
	kill(-1, SIGTERM);
	sleep(1);

	cprintf("Sending SIGKILL to all processes\n");
	kill(-1, SIGKILL);
	sleep(1);

	sync();
}


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
	SIGTERM,	/* reboot */
};

void
fatal_signal(int sig)
{
	dbG("sig: %d 0x%x\n", sig, sig);

	char *message = NULL;

	switch (sig) {
	case SIGQUIT: message = "Quit"; break;
	case SIGILL: message = "Illegal instruction"; break;
	case SIGABRT: message = "Abort"; break;
	case SIGFPE: message = "Floating exception"; break;
	case SIGPIPE: message = "Broken pipe"; break;
	case SIGBUS: message = "Bus error"; break;
	case SIGSEGV: message  = "Segmentation fault"; break;
	case SIGSYS: message = "Bad system call"; break;
	case SIGTRAP: message = "Trace trap"; break;
	case SIGPWR: message = "Power failure"; break;
	case SIGTERM: message = "Terminated"; break;
	}
	if (message)
		dbG("%s\n", message);

	system("touch /tmp/.reboot");
	shutdown_system();
	sleep(1);

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
