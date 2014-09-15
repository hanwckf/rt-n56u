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
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdarg.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include "rc.h"

#define INICD_POLL_INTERVAL	15 /* every 15 seconds  */
#define INICD_PID_FILE		"/var/run/inicd.pid"

static void
inicd_alarmtimer(unsigned long sec)
{
	struct itimerval itv;

	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = 0;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void
catch_sig_inicd(int sig)
{
	switch (sig)
	{
	case SIGALRM:
		check_inic_mii_rebooted();
		break;
	case SIGTERM:
		remove(INICD_PID_FILE);
		inicd_alarmtimer(0);
		exit(0);
		break;
	}
}

int
stop_inicd(void)
{
	return doSystem("killall %s %s", "-q", "inicd");
}

int
start_inicd(void)
{
	return eval("/sbin/inicd");
}

int
inicd_main(int argc, char *argv[])
{
	FILE *fp;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_inicd;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);

	if (daemon(0, 0) < 0) {
		perror("daemon");
		exit(errno);
	}

	/* write pid */
	if ((fp = fopen(INICD_PID_FILE, "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* set poll timer */
	inicd_alarmtimer(INICD_POLL_INTERVAL);

	/* Most of time it goes to sleep */
	while (1) {
		pause();
	}

	return 0;
}

