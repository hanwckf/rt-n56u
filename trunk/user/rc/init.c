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
#define __USE_GNU
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>

#include "rc.h"

#define CONSOLE_TERMINAL	"vt100"
#define INIT_HOME_PATH		"/home/root"

static volatile sig_atomic_t sig_term_received = 0;
static volatile sig_atomic_t sig_usr1_received = 0;
static volatile sig_atomic_t sig_alrm_received = 0;
static int noconsole = 0;

static const int signals_fatal[] = {
	SIGILL,
	SIGSEGV,
	SIGABRT,
	SIGFPE,
	SIGPIPE,
	SIGBUS,
	SIGSYS,
	SIGTRAP,
	SIGPWR,
	SIGQUIT,
	SIGTERM
};

static const int signals_event[] = {
	SIGHUP,
	SIGUSR1,
	SIGUSR2,
	SIGALRM
};

static const char *const environment[] = {
	"PATH=" SYS_EXEC_PATH,
	"SHELL=" SYS_SHELL,
	"USER=" SYS_USER_ROOT,
	"HOME=" INIT_HOME_PATH,
	NULL
};

/* backward definition */
static void
reset_signals(void);

/* signals handling */
static void
catch_sig_fatal(int sig)
{
	sig_term_received = 1;

	printf("%s signal: %s\n", "fatal", strsignal(sig));

	if (sig == SIGQUIT) {
		shutdown_router(0);
		return;
	}

	reset_signals();

	shutdown_router(1);

	kill(-1, SIGTERM);
	sleep(1);
	sync();

	reboot(RB_AUTOBOOT);

	do {
		sleep(1);
	} while (1);
}

static void
catch_sig_event(int sig)
{
	printf("%s signal: %s\n", "event", strsignal(sig));

	if (sig_term_received)
		return;

	switch (sig)
	{
	case SIGHUP:
		break;
	case SIGUSR1:
		sig_usr1_received = 1;
		break;
	case SIGUSR2:
		break;
	case SIGALRM:
		sig_alrm_received = 1;
		break;
	}
}

static void
catch_sig_reap(int sig)
{
	pid_t pid;

	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
		dprintf("Reaped %d\n", pid);
}

/* Set terminal settings to reasonable defaults */
static void
setup_term(void)
{
	struct termios tty;

	tcgetattr(STDIN_FILENO, &tty);

	/* set control chars */
	tty.c_cc[VINTR]  = 3;	/* C-c */
	tty.c_cc[VQUIT]  = 28;	/* C-\ */
	tty.c_cc[VERASE] = 127;	/* C-? */
	tty.c_cc[VKILL]  = 21;	/* C-u */
	tty.c_cc[VEOF]   = 4;	/* C-d */
	tty.c_cc[VSTART] = 17;	/* C-q */
	tty.c_cc[VSTOP]  = 19;	/* C-s */
	tty.c_cc[VSUSP]  = 26;	/* C-z */

	/* use line discipline 0 */
	tty.c_line = 0;

	/* added CRTSCTS to fix Debian bug 528560 */
	tty.c_cflag &= CBAUD | CBAUDEX | CSIZE | CSTOPB | PARENB | PARODD | CRTSCTS;
	tty.c_cflag |= CREAD | HUPCL | CLOCAL;

	/* input modes */
	tty.c_iflag = ICRNL | IXON | IXOFF;

	/* output modes */
	tty.c_oflag = OPOST | ONLCR;

	/* local modes */
	tty.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;

	tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

static int
init_console(void)
{
	int fd;
	int ret = 0;

	/* Clean up */
	ioctl(STDIN_FILENO, TIOCNOTTY, 0);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	setsid();

	/* Reopen console */
	if ((fd = open(_PATH_CONSOLE, O_RDWR)) < 0) {
		ret = errno;
		perror(_PATH_CONSOLE);
		if ((fd = open("/dev/null", O_RDWR)) < 0) {
			perror("/dev/null");
			return errno;
		}
	}

	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);

	ioctl(STDIN_FILENO, TIOCSCTTY, 1);
	tcsetpgrp(STDIN_FILENO, getpgrp());

	setup_term();

	return ret;
}

static void
reset_signals(void)
{
	int i;
	sigset_t set;
	struct sigaction sa;

	/* Disable signal handlers */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	for (i = 0; i < (_NSIG-1); i++)
		sigaction(i, &sa, NULL);

	/* Unblock all signals */
	sigfillset(&set);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
}

static pid_t
run_shell(int timeout, int nowait)
{
	pid_t pid;
	char tz[128];
	char *envp[] = {
		"TERM=" CONSOLE_TERMINAL,
		"TERMINFO=/usr/share/terminfo",
		"PATH=" SYS_EXEC_PATH,
		"SHELL=" SYS_SHELL,
		"USER=" SYS_USER_ROOT,
		"HOME=" INIT_HOME_PATH,
		tz,
		NULL
	};

	/* Wait for user input */
//	if (waitfor (STDIN_FILENO, timeout) <= 0)
//		return 0;

	switch ((pid = fork())) {
	case -1:
		perror("fork");
		return 0;
	case 0:
		/* Reset signal handlers set for parent process */
		reset_signals();

		/* Reopen console */
		init_console();

		/* Pass on TZ */
		time_zone_x_mapping();
		snprintf(tz, sizeof(tz), "TZ=%s", nvram_safe_get("time_zone_x"));

		/* Now run it.  The new program will take over this PID, 
		 * so nothing further in init.c should be run. */
		execve(SYS_SHELL, (char *[]) { SYS_SHELL, NULL }, envp);

		/* We're still here?  Some error happened. */
		perror(SYS_SHELL);
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
control_signal(int sig, int how)
{
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, sig);
	sigprocmask(how, &ss, NULL);
}

static void
init_signals(void)
{
	int i;
	struct sigaction sa;

	/* Setup fatal signals (block fatal handler from all own signals) */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_fatal;
	sigemptyset(&sa.sa_mask);
	for (i = 0; i < ARRAY_SIZE(signals_event); i++)
		sigaddset(&sa.sa_mask, signals_event[i]);
	for (i = 0; i < ARRAY_SIZE(signals_fatal); i++)
		sigaddset(&sa.sa_mask, signals_fatal[i]);
	for (i = 0; i < ARRAY_SIZE(signals_fatal); i++)
		sigaction(signals_fatal[i], &sa, NULL);

	/* Setup SIGCHLD signal */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_reap;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGCHLD, &sa, NULL);

	/* Setup event signals */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_event;
	sigemptyset(&sa.sa_mask);
	for (i = 0; i < ARRAY_SIZE(signals_event); i++)
		sigaction(signals_event[i], &sa, NULL);
}

static void
init_time(void)
{
	struct tm stm;
	time_t st;

	time(&st);
	localtime_r(&st, &stm);
	stm.tm_year = (SYS_START_YEAR - 1900);
	st = mktime(&stm);
	stime(&st);
}

static void
init_nodes(void)
{
	mknod("/dev/spiS0",  S_IFCHR | 0666, makedev(217, 0));
	mknod("/dev/i2cM0",  S_IFCHR | 0666, makedev(218, 0));
	mknod("/dev/nvram",  S_IFCHR | 0666, makedev(228, 0));
	mknod("/dev/gpio",   S_IFCHR | 0666, makedev(252, 0));
	mknod("/dev/rdm0",   S_IFCHR | 0666, makedev(253, 0));

	mknod("/dev/hwnat0", S_IFCHR | 0666, makedev(220, 0));
#if !defined (USE_HW_NAT_V2)
	mknod("/dev/acl0",   S_IFCHR | 0666, makedev(230, 0));
	mknod("/dev/ac0",    S_IFCHR | 0666, makedev(240, 0));
	mknod("/dev/mtr0",   S_IFCHR | 0666, makedev(250, 0));
#endif
}

static void
init_mdev(void)
{
	FILE *fp;

	fp = fopen("/etc/mdev.conf", "w");
	if (fp) {
		fprintf(fp, "%s\n", "# <device regex> <uid>:<gid> <octal permissions> [<@|$|*> <command>]");
#if (BOARD_NUM_USB_PORTS > 0)
		fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "usb/lp[0-9]",  "*", "mdev_lp");
		fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "sd[a-z]",      "*", "mdev_sd");
		fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "sd[a-z][0-9]", "*", "mdev_sd");
		fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "sg[0-9]",      "@", "mdev_sg");
		fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "sr[0-9]",      "@", "mdev_sr");
		fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "weth[0-9]",    "*", "mdev_net");
		fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "wwan[0-9]",    "*", "mdev_net");
		fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "cdc-wdm[0-9]", "*", "mdev_wdm");
		fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "ttyUSB[0-9]",  "*", "mdev_tty");
		fprintf(fp, "%s 0:0 0660 %s/sbin/%s $MDEV $ACTION\n", "ttyACM[0-9]",  "*", "mdev_tty");
		fprintf(fp, "%s 0:0 0660\n", "video[0-9]");
#endif
		fclose(fp);
	}

	fput_string("/proc/sys/kernel/hotplug", "/sbin/mdev");
}

static void
init_sysctl(void)
{
	fput_int("/proc/sys/kernel/panic", 1);

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

	set_tcp_tweaks();

#if defined (USE_IPV6)
	control_if_ipv6_all(0);
#endif
}

/* Main loop */
void
init_main_loop(void)
{
	pid_t shell_pid = 0;

	umask(0000);

	/* Basic initialization */
	init_time();
	system("dev_init.sh");
	init_nodes();
	init_mdev();
	init_sysctl();

	/* Setup console */
	if (init_console())
		noconsole = 1;

	chdir("/");
	setsid();

	/* Setup shell environment */
	{
		const char *const *e;
		/* Make sure environs is set to something sane */
		for (e = environment; *e; e++)
			putenv((char *) *e);
	}

	/* Setup signal handlers */
	init_signals();

	/* block SIGUSR1 during init_router() */
	control_signal(SIGUSR1, SIG_BLOCK);

	/* Router init and start */
	init_router();

	/* unblock SIGUSR1 */
	control_signal(SIGUSR1, SIG_UNBLOCK);

	/* Loop forever */
	for (;;) {
		sigset_t ss;
		sigemptyset(&ss);
		
		/* Wait for user input or state change */
		while ( !sig_usr1_received &&
			!sig_alrm_received ) {
			if (!noconsole && (!shell_pid || kill(shell_pid, 0) != 0))
				shell_pid = run_shell(0, 1);
			else
				sigsuspend(&ss);
		}
		
		/* Handle SIGUSR1 signal */
		if (sig_usr1_received) {
			sig_usr1_received = 0;
			handle_notifications();
		}
		
		/* Handle SIGALRM signal */
		if (sig_alrm_received) {
			sig_alrm_received = 0;
#if (BOARD_NUM_USB_PORTS > 0)
			on_deferred_hotplug_usb();
#endif
		}
	}
}

int
sys_exit(void)
{
	return kill(1, SIGTERM);
}

int
sys_stop(void)
{
	return kill(1, SIGQUIT);
}
