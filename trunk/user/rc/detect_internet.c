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
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#include <nvram/bcmnvram.h>

#include "rc.h"

//#define DEBUG		1
#define DI_RES_FILE	"/tmp/internet_check_result"
#define DI_PID_FILE	"/var/run/detect_internet.pid"

static long detect_last = 0;
static unsigned long skip_last = 3;

void stop_detect_internet(void)
{
	doSystem("killall %s %s", "-q", "detect_internet");
}

int start_detect_internet(void)
{
	stop_detect_internet();

	if (get_ap_mode())
		return 1;

	return eval("detect_internet");
}

static int
is_internet_detected(void)
{
	FILE *fp = NULL;
	char line[128];
	char *detect_host[] = {"8.8.8.8", "208.67.220.220", "8.8.4.4", "208.67.222.222"};
	int i;

#ifdef DEBUG
	dbg("## detect internet status ##\n");
#endif
	remove(DI_RES_FILE);

	i = rand_seed_by_time() % 4;
	doSystem("/usr/sbin/tcpcheck 4 %s:53 %s:53 >%s", detect_host[i], detect_host[(i+1)%4], DI_RES_FILE);

	if ((fp = fopen(DI_RES_FILE, "r")) != NULL)
	{
		while(fgets(line, sizeof(line), fp) != NULL)
		{
			if (strstr(line, "alive"))
			{
#ifdef DEBUG
				dbg("[di] got response!\n");
#endif
				fclose(fp);
				return 1;
			}
		}
		
		fclose(fp);
	}

#ifdef DEBUG
	dbg("[di] no response!\n");
#endif

	return 0;
}

static void
try_detect_internet(void)
{
	long now;
	int link_internet;
	char *login_timestamp = nvram_safe_get("login_timestamp");

	if (strlen(login_timestamp) < 1)
		return;

	if (!has_wan_ip(0) || !found_default_route(0))
	{
#ifdef DEBUG
		dbg("[di] no WAN IP, or no default route!\n");
#endif
		skip_last = 3;
		nvram_set_int_temp("link_internet", 0);
		
		return;
	}

	now = uptime();
	if ((unsigned long)(now - detect_last) < skip_last)
		return;

	detect_last = now;
	link_internet = is_internet_detected();

	skip_last = (link_internet) ? 55 : 3;

	nvram_set_int_temp("link_internet", link_internet);
}

static void catch_sig_detect_internet(int sig)
{
	if (sig == SIGTERM)
	{
		remove(DI_PID_FILE);
		exit(0);
	}
	else if (sig == SIGHUP)
	{
		try_detect_internet();
	}
}

int
detect_internet_main(int argc, char *argv[])
{
	FILE *fp;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP,  catch_sig_detect_internet);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGTERM, catch_sig_detect_internet);

	if (daemon(0, 0) < 0) {
		perror("daemon");
		exit(errno);
	}

	/* write pid */
	if ((fp = fopen(DI_PID_FILE, "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	nvram_set_int_temp("link_internet", 2);

	while (1)
	{
		pause();
	}

	remove(DI_PID_FILE);

	return 0;
}

