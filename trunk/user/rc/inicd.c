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

#include <nvram/bcmnvram.h>

#include "rc.h"

struct itimerval itv;

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void check_inic_radio(void)
{
	if (get_mlme_radio_rt()) {
		int rt_mode_x = nvram_get_int("rt_mode_x");
		
		/* start guest AP */
		if (rt_mode_x != 1 && rt_mode_x != 3 &&
		   !is_interface_up(IFNAME_INIC_GUEST) && is_guest_allowed_rt()) {
			doSystem("ifconfig %s %s 2>/dev/null", IFNAME_INIC_GUEST, "up");
			if (is_interface_up(IFNAME_INIC_GUEST))
				restart_guest_lan_isolation();
		}
	} else {
		/* disable mlme radio */
		doSystem("iwpriv %s set RadioOn=%d", IFNAME_INIC_MAIN, 0);
	}
}

static void catch_sig_inicd(int sig)
{
	if (sig == SIGTERM)
	{
		alarmtimer(0, 0);
		remove("/var/run/inicd.pid");
		exit(0);
	}
	else if (sig == SIGALRM)
	{
		check_inic_radio();
	}
}

int start_inicd(void)
{
	return eval("/sbin/inicd");
}

int stop_inicd(void)
{
	return doSystem("killall %s %s", "-q", "inicd");
}

int 
inicd_main(int argc, char *argv[])
{
	FILE *fp;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGTERM, catch_sig_inicd);
	signal(SIGALRM, catch_sig_inicd);

	if (daemon(0, 0) < 0) {
		perror("daemon");
		exit(errno);
	}

	/* write pid */
	if ((fp = fopen("/var/run/inicd.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* set timer 10s */
	alarmtimer(10, 0);

	/* Most of time it goes to sleep */
	while (1)
	{
		pause();
	}

	return 0;
}

