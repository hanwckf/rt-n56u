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
#define DI_MAX_HOSTS	4

static long detect_last_time = 0;
static unsigned long skip_last = 3;

void
stop_detect_internet(void)
{
	doSystem("killall %s %s", "-q", "detect_internet");
}

int
start_detect_internet(void)
{
	return eval("detect_internet");
}

void
notify_detect_internet(void)
{
	if (get_ap_mode())
		return;

	if (!pids("detect_internet"))
		start_detect_internet();
	else
		kill_pidfile_s(DI_PID_FILE, SIGUSR1);
}

static void
reset_internet_status(void)
{
	skip_last = 3;
	nvram_set_int_temp("link_internet", 2);
}

static int
is_internet_detected(void)
{
	FILE *fp = NULL;
	char line[128], nvram_addr[16], nvram_port[16];
	char di_host[DI_MAX_HOSTS][32];
	int i, max_items;

#ifdef DEBUG
	dbg("## detect internet status ##\n");
#endif
	remove(DI_RES_FILE);

	memset(di_host, 0, sizeof(di_host));

	max_items = 0;
	for (i = 0; i < DI_MAX_HOSTS; i++) {
		char *di_addr;
		int di_port;
		snprintf(nvram_addr, sizeof(nvram_addr), "di4_addr%d", i);
		snprintf(nvram_port, sizeof(nvram_port), "di4_port%d", i);
		di_addr = nvram_safe_get(nvram_addr);
		di_port = nvram_safe_get_int(nvram_port, 53, 1, 65535);
		if (is_valid_ipv4(di_addr)) {
			sprintf(di_host[max_items], "%s:%d", di_addr, di_port);
			max_items++;
		}
	}

	if (max_items < 1) {
		sprintf(di_host[0], "%s:%d", "8.8.8.8", 53);
		max_items = 1;
	}

	if (max_items > 1) {
		i = rand_seed_by_time() % max_items;
		doSystem("/usr/sbin/tcpcheck %d %s %s >%s", 4, di_host[i], di_host[(i+1)%max_items], DI_RES_FILE);
	} else {
		doSystem("/usr/sbin/tcpcheck %d %s >%s", 4, di_host[0], DI_RES_FILE);
	}

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
	char *login_timestamp;

	/* check last http login */
	login_timestamp = nvram_safe_get("login_timestamp");
	if (strlen(login_timestamp) < 1)
		return;

	if (!has_wan_ip(0) || !has_wan_gateway())
	{
#ifdef DEBUG
		dbg("[di] no WAN IP, or no default route!\n");
#endif
		skip_last = 3;
		nvram_set_int_temp("link_internet", 0);
		
		return;
	}

	now = uptime();
	if ((unsigned long)(now - detect_last_time) < skip_last)
		return;

	detect_last_time = now;
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
	else if (sig == SIGUSR1)
	{
		reset_internet_status();
	}
}

int
detect_internet_main(int argc, char *argv[])
{
	FILE *fp;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_detect_internet;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);

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

	reset_internet_status();

	while (1)
	{
		pause();
	}

	remove(DI_PID_FILE);

	return 0;
}

