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
#include <unistd.h>
#include <sys/types.h>
#include <shutils.h>
#include <nvram/bcmnvram.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#include "rc.h"

#define DETECT_FILE "/tmp/internet_check_result"

int di_debug = 0;

int do_detect()
{
	FILE *fp = NULL;
	char line[80], cmd[128];
	char *detect_host[] = {"8.8.8.8", "208.67.220.220", "8.8.4.4", "208.67.222.222"};
	int i;

	if (di_debug) dbg("## detect internet status ##\n");

	remove(DETECT_FILE);
	i = rand_seed_by_time() % 4;
	snprintf(cmd, sizeof(cmd), "/usr/sbin/tcpcheck 4 %s:53 %s:53 >%s", detect_host[i], detect_host[(i+1)%4], DETECT_FILE);
	if (di_debug) dbg("cmd: %s\n", cmd);
	system(cmd);
	if (di_debug) doSystem("cat %s", DETECT_FILE);

        if ((fp = fopen(DETECT_FILE, "r")) != NULL)
        {
		while(1)
		{
			if ( fgets(line, sizeof(line), fp) != NULL )
			{
				if (strstr(line, "alive"))
				{
					if (di_debug) dbg("got response!\n");
					fclose(fp);
					return 1;
				}
			}
			else
				break;
		}

		fclose(fp);
		if (di_debug) dbg("no response!\n");
		return 0;
	}

	if (di_debug) dbg("fopen %s error!\n", DETECT_FILE);
	return 0;
}

struct itimerval itv;

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void catch_sig_detect_internet(int sig)
{
	time_t now;
	int link_internet = 0;

	int no_login_timestamp = 0;
	int no_detect_timestamp = 0;
	int no_detect_for_timeout = 0;

	if (sig == SIGALRM)
	{
		if (di_debug) dbg("[di] SIGALRM\n");

		now = uptime();

		if (!nvram_get("login_timestamp") || nvram_match("login_timestamp", ""))
			no_login_timestamp = 1;

		if (nvram_match("detect_timestamp", "0"))
			no_detect_timestamp = 1;

		if ((unsigned long)(now - strtoul(nvram_safe_get("detect_timestamp"), NULL, 10)) > 60)
			no_detect_for_timeout = 1;

		if (no_login_timestamp || no_detect_timestamp || no_detect_for_timeout)
		{
			alarmtimer(0, 0);
			return;
		}

		if (nvram_match("no_internet_detect", "1"))
		{
			if (di_debug) dbg("pause for wan rate detection!\n");
			alarm(1);
			return;
		}

		if (!is_phyconnected() || !has_wan_ip(0) || !found_default_route(0))
		{
			if (di_debug) dbg("link down, no WAN IP, or no default route!\n");
			nvram_set("link_internet", "0");
			alarm(3);
			return;
		}

		if (do_detect() == 1)
		{
			if (di_debug) dbg("internet connection ok!\n");
			nvram_set("link_internet", "1");
			link_internet = 1;
		}
		else
		{
			if (di_debug) dbg("no connection!\n");
			nvram_set("link_internet", "0");
			link_internet = 0;
		}
		
		if (link_internet == 1)
			alarm(15);
		else
			alarm(1);
	}
	else if (sig == SIGTERM)
	{
		if (di_debug) dbg("[di] SIGTERM\n");

		alarmtimer(0, 0);
		remove("/var/run/detect_internet.pid");
		exit(0);
	}
	else if (sig == SIGUSR1)
	{
		if (nvram_match("di_debug", "1"))
			di_debug = 1;
		else
			di_debug = 0;

		if (di_debug) dbg("[di] SIGUSR1\n");
		alarmtimer(1, 0);
	}
}

int
detect_internet_main(int argc, char *argv[])
{
	FILE *fp;

	/* write pid */
	if ((fp = fopen("/var/run/detect_internet.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	nvram_set("link_internet", "2");

	if (nvram_match("di_debug", "1"))
		di_debug = 1;
	else
		di_debug = 0;

	if (SIG_ERR == signal(SIGTERM, catch_sig_detect_internet))
		dbg("signal SIGTERM error\n");

	if (SIG_ERR == signal(SIGALRM, catch_sig_detect_internet))
		dbg("signal SIGALRM error\n");

	if (SIG_ERR == signal(SIGUSR1, catch_sig_detect_internet))
		dbg("signal SIGUSR1 error\n");

	alarmtimer(1, 0);

	while (1)
	{
		pause();
	}

	return 0;
}
