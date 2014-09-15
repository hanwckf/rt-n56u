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
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#include "rc.h"

#define DI_MAX_HOSTS	6
#define DI_STATUS_INIT	2
#define DI_PID_FILE	"/var/run/detect_internet.pid"

static struct sockaddr_in    di_host_sa[DI_MAX_HOSTS];
static int                   di_host_ok[DI_MAX_HOSTS];
static int                   di_host_next;
static int                   di_host_total;
static int                   di_poll_mode;
static int                   di_status;
static int                   di_is_ap_mode;
static long                  di_time_last_activity;
static long                  di_time_last_state;
static long                  di_time_diff_state;
static long                  di_time_fail_event;
static volatile sig_atomic_t di_pause_received = 0;
static struct itimerval      di_itv;

static void
di_alarmtimer(unsigned long sec)
{
	di_itv.it_value.tv_sec  = sec;
	di_itv.it_value.tv_usec = 0;
	di_itv.it_interval = di_itv.it_value;
	setitimer(ITIMER_REAL, &di_itv, NULL);
}

static int
di_connect_to_host(const struct sockaddr_in *p_sa_dst, int timeout)
{
	int fd, flags, ret;
	struct linger lgr;
	struct timeval to;
	fd_set rset;

	ret = -1;

	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0)
		return ret;

	/* allow immediate reuse of the address */
	flags = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&flags, sizeof(flags));

	/* don't wait for data to be delivered on close(). */
	memset(&lgr, 0, sizeof(lgr));
	setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&lgr, sizeof(lgr));

	/* set non-blocking mode */
	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		goto done_exit;

	ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	if (ret < 0)
		goto done_exit;

	/* try connect to host */
	ret = connect(fd, (struct sockaddr *)p_sa_dst, sizeof(struct sockaddr_in));
	if (ret < 0 && errno != EINPROGRESS)
		goto done_exit;

	if (ret == 0)
		goto done_exit;

	/* now wait connection established */
	FD_ZERO(&rset);
	FD_SET(fd, &rset);

	to.tv_sec = timeout;
	to.tv_usec = 0;

	ret = select(fd + 1, NULL, &rset, NULL, &to);
	if (ret <= 0) {
		if (ret == 0)
			errno = ETIMEDOUT;
		ret = -1;
		goto done_exit;
	}

	if (di_pause_received) {
		ret = -1;
		goto done_exit;
	}

	if (FD_ISSET(fd, &rset)) {
		socklen_t len = (socklen_t)sizeof(ret);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &ret, &len) < 0) {
			ret = -1;
			goto done_exit;
		}
		
		/* oops - something wrong with connect */
		if (ret != 0) {
			errno = ret;
			ret = -1;
		}
	}

done_exit:

	close(fd);

	return ret;
}

static void
di_reset_state(void)
{
	int i;

	for (i = 0; i < DI_MAX_HOSTS; i++)
		di_host_ok[i] = DI_STATUS_INIT;

	di_host_next = 0;

	di_time_fail_event = 0;
	di_time_last_state = uptime();
	di_time_diff_state = 0;

	di_status = DI_STATUS_INIT;
	nvram_set_int_temp("link_internet", DI_STATUS_INIT);
}

static void
di_load_settings(void)
{
	int i, i_items;
	char nvram_addr[16], nvram_port[16];

	memset(di_host_sa, 0, sizeof(di_host_sa));

	i_items = 0;
	for (i = 0; i < DI_MAX_HOSTS; i++) {
		snprintf(nvram_addr, sizeof(nvram_addr), "di_addr%d", i);
		snprintf(nvram_port, sizeof(nvram_port), "di_port%d", i);
		
		di_host_sa[i_items].sin_family = AF_INET;
		di_host_sa[i_items].sin_addr.s_addr = inet_addr_safe(nvram_safe_get(nvram_addr));
		di_host_sa[i_items].sin_port = htons(nvram_safe_get_int(nvram_port, 53, 1, 65535));
		
		if (di_host_sa[i_items].sin_addr.s_addr != INADDR_ANY &&
		    di_host_sa[i_items].sin_addr.s_addr != INADDR_NONE)
			i_items++;
	}

	if (i_items < 1) {
		di_host_sa[0].sin_family = AF_INET;
		di_host_sa[0].sin_addr.s_addr = 0x08080808;
		di_host_sa[0].sin_port = htons(53);
		i_items = 1;
	}

	di_host_total = i_items;
	if (i_items > 1)
		di_host_next = rand_seed_by_time() % i_items;

	di_poll_mode = nvram_safe_get_int("di_poll_mode", 0, 0, 1);

	di_is_ap_mode = get_ap_mode();
}

static void
di_update_last_activity(void)
{
	long last_activity = di_time_last_activity;

	di_time_last_activity = uptime();

	/* boost poll timer once */
	if (di_poll_mode == 0) {
		if ((unsigned long)(di_time_last_activity - last_activity) > 60) {
			if (!di_pause_received && di_itv.it_value.tv_sec != 1)
				di_alarmtimer(1);
		}
	}
}

static void
di_on_timer(void)
{
	int i, link_error, link_internet, has_unchecked;
	long now;
	unsigned long period_next;

	/* check last user activity */
	if (di_poll_mode == 0) {
		if ((unsigned long)(uptime() - di_time_last_activity) > 60) {
			/* relax poll timer */
			if (!di_pause_received && di_itv.it_value.tv_sec != 60)
				di_alarmtimer(60);
			return;
		}
	}

	link_error = -1;

	i = di_host_next;

	if (di_is_ap_mode || has_wan_ip4(0))
		link_error = di_connect_to_host(&di_host_sa[i], nvram_safe_get_int("di_timeout", 3, 1, 10));

	di_host_ok[i] = (link_error == 0) ? 1 : 0;
	di_host_next = (di_host_next + 1) % di_host_total;

	if (link_error == 0)
		period_next = (unsigned long)nvram_safe_get_int("di_time_done", 55, 15, 600);
	else
		period_next = (unsigned long)nvram_safe_get_int("di_time_fail",  5,  3, 120);

	has_unchecked = 0;
	link_internet = DI_STATUS_INIT;
	for (i = 0; i < di_host_total; i++) {
		if (di_host_ok[i] == DI_STATUS_INIT)
			has_unchecked = 1;
		if (di_host_ok[i] == 1) {
			link_internet = 1;
			break;
		}
	}

	now = uptime();

	/* all hosts failed */
	if (link_internet != 1 && !has_unchecked)
		link_internet = 0;

	if (di_pause_received)
		link_internet = 0;

	if (link_internet == 1 && di_time_fail_event)
		di_time_fail_event = 0;

	if ((di_time_fail_event > 0) && (now >= di_time_fail_event)) {
		di_time_fail_event = 0;
		
		if (link_internet == 0 && di_poll_mode != 0 && !di_pause_received)
			notify_on_internet_state_changed(0, di_time_diff_state);
	}

	if (link_internet != DI_STATUS_INIT && di_status != link_internet) {
		di_status = link_internet;
		di_time_diff_state = now - di_time_last_state;
		
		nvram_set_int_temp("link_internet", link_internet);
		
#if defined (BOARD_GPIO_LED_WAN)
		if (nvram_get_int("front_led_wan") == 3)
			LED_CONTROL(BOARD_GPIO_LED_WAN, (link_internet) ? LED_ON : LED_OFF);
#endif
		if (di_poll_mode != 0 && !di_pause_received) {
			long fail_delay = (long)nvram_safe_get_int("di_lost_delay", 10, 0, 600);
			
			if (link_internet || fail_delay == 0) {
				notify_on_internet_state_changed(link_internet, di_time_diff_state);
			} else {
				di_time_fail_event = now + fail_delay;
				di_time_diff_state += fail_delay;
			}
		}
		
		di_time_last_state = now;
	}

	if (!di_pause_received && di_itv.it_value.tv_sec != period_next)
		di_alarmtimer(period_next);
}

static void
di_on_sigusr1(void)
{
	int delay_time;

	di_pause_received = 0;

	di_alarmtimer(0);
	di_reset_state();
	di_load_settings();

	delay_time = nvram_get_int("di_notify_delay");
	if (delay_time < 1)
		delay_time = 1;

	di_alarmtimer(delay_time);
}

static void
di_on_sigusr2(void)
{
	di_pause_received = 1;
	di_alarmtimer(0);
	nvram_set_int_temp("link_internet", 0);
}

static void
catch_sig_detect_internet(int sig)
{
	switch (sig)
	{
	case SIGALRM:
		di_on_timer();
		break;
	case SIGHUP:
		di_update_last_activity();
		break;
	case SIGUSR1:
		di_on_sigusr1();
		break;
	case SIGUSR2:
		di_on_sigusr2();
		break;
	case SIGTERM:
		di_pause_received = 1;
		remove(DI_PID_FILE);
		di_alarmtimer(0);
		exit(0);
		break;
	}
}

void
stop_detect_internet(void)
{
	doSystem("killall %s %s", "-q", "detect_internet");
}

int
start_detect_internet(int autorun_time)
{
	char arun[16];
	char *di_argv[] = {
		"/sbin/detect_internet",
		NULL,		/* -aX */
		NULL
	};

	if (autorun_time > 0) {
		snprintf(arun, sizeof(arun), "-a%u", autorun_time);
		di_argv[1] = arun;
	}

	return _eval(di_argv, NULL, 0, NULL);
}

void
notify_run_detect_internet(int delay_time)
{
	nvram_set_int_temp("di_notify_delay", delay_time);

	if (!pids("detect_internet"))
		start_detect_internet(delay_time);
	else
		kill_pidfile_s(DI_PID_FILE, SIGUSR1);
}

void
notify_pause_detect_internet(void)
{
	kill_pidfile_s(DI_PID_FILE, SIGUSR2);
}

int
detect_internet_main(int argc, char *argv[])
{
	FILE *fp;
	pid_t pid;
	int c, auto_run_time = 0;
	struct sigaction sa;

	// usage : detect_internet -a X
	if(argc) {
		while ((c = getopt(argc, argv, "a:")) != -1) {
			switch (c) {
			case 'a':
				auto_run_time = atoi(optarg);
				break;
			}
		}
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_detect_internet;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaddset(&sa.sa_mask, SIGALRM);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, NULL);

	if (daemon(0, 0) < 0) {
		perror("daemon");
		exit(errno);
	}

	pid = getpid();

	/* never invoke oom killer */
	oom_score_adjust(pid, OOM_SCORE_ADJ_MIN);

	/* write pid */
	if ((fp = fopen(DI_PID_FILE, "w")) != NULL) {
		fprintf(fp, "%d", pid);
		fclose(fp);
	}

	nvram_set_int_temp("di_notify_delay", 0);

	di_pause_received = (auto_run_time > 0) ? 0 : 1;
	di_time_last_activity = 0;

	di_reset_state();
	di_load_settings();

	if (auto_run_time > 0)
		di_alarmtimer(auto_run_time);

	while (1) {
		pause();
	}

	return 0;
}

