/*

	rstats
	Copyright (C) 2006-2009 Jonathan Zarate


	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>

#include <include/bsd_queue.h>
#include <rstats.h>

#include "rc.h"
#include "switch.h"

#define SMIN		60
#define SHOUR		(60 * 60)
#define SDAY		(60 * 60 * 24)

#define MAX_NSPEED	((24 * SHOUR) / RSTATS_INTERVAL)
#define MAX_NDAILY	62
#define MAX_NMONTHLY	25

#if !defined(RSTATS_SKIP_ESW)
#define MAX_SPEED_IF	(IFDESCS_MAX_NUM + BOARD_NUM_ETH_EPHY)
#else
#define MAX_SPEED_IF	IFDESCS_MAX_NUM
#endif

#if defined(USE_USB_SUPPORT)
#define MAX_HISTORY_IF	3
#else
#define MAX_HISTORY_IF	2
#endif

#define MAX_COUNTER	2
#define RX		0
#define TX		1

#define DAILY		0
#define MONTHLY		1

#define CURRENT_ID	0x31305352

typedef struct speed_item {
	SLIST_ENTRY(speed_item) entries;
	char ifdesc[16];
	int ifindex;
	int tail;
	int skips;
	uint64_t last[MAX_COUNTER];
	uint32_t speed[MAX_NSPEED][MAX_COUNTER];
} speed_item_t;

typedef struct speed_item_list {
	SLIST_HEAD(, speed_item) head;
	int count;
	int tail;
} speed_item_list_t;

typedef struct {
	uint32_t xtime;
	uint64_t counter[MAX_COUNTER];
} data_t;

typedef struct {
	uint32_t id;
	data_t daily[MAX_NDAILY];
	int dailyp;
	data_t monthly[MAX_NMONTHLY];
	int monthlyp;
} history_t;

static speed_item_list_t g_speed_list;

static history_t *g_history = NULL;

static struct history_desc_t {
	const char *ifdesc;
	unsigned char is_wisp;
	unsigned char is_active;
} g_history_desc[MAX_HISTORY_IF] = {
	{IFDESC_WAN,  0, 0},
	{IFDESC_WISP, 1, 0},
#if defined(USE_USB_SUPPORT)
	{IFDESC_WWAN, 0, 0},
#endif
};

static int g_ap_mode = 0;

static long g_uptime_now = 0;
static long g_uptime_old = 0;
static long g_store_cntr = 0;

static volatile sig_atomic_t gothup = 0;
static volatile sig_atomic_t gotusr1 = 0;
static volatile sig_atomic_t gotusr2 = 0;

// ===========================================

static int f_read(const char *path, void *buffer, int max)
{
	int fd;
	int n;

	if ((fd = open(path, O_RDONLY)) < 0) return -1;
	n = read(fd, buffer, max);
	close(fd);
	return n;
}

static int f_write(const char *path, const void *buffer, int len)
{
	int fd;
	int r = -1;
	mode_t m;

	m = umask(0);
	if ((fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0666)) >= 0) {
		r = write(fd, buffer, len);
		close(fd);
	}
	umask(m);
	return r;
}

static int alloc_rstats(void)
{
	int i;
	size_t sz_history = MAX_HISTORY_IF * sizeof(history_t);

	SLIST_INIT(&g_speed_list.head);
	g_speed_list.count = 0;
	g_speed_list.tail = 0;

	g_history = malloc(sz_history);
	if (!g_history)
		return -1;

	memset(g_history, 0, sz_history);

	for (i = 0; i < MAX_HISTORY_IF; i++)
		g_history[i].id = CURRENT_ID;

	return 0;
}

static void free_rstats(void)
{
	speed_item_t *item, *next;

	SLIST_FOREACH_SAFE(item, &g_speed_list.head, entries, next) {
		free(item);
	}

	SLIST_INIT(&g_speed_list.head);
	g_speed_list.count = 0;
	g_speed_list.tail = 0;

	if (g_history) {
		free(g_history);
		g_history = NULL;
	}
}

static int is_history_active(const history_t *ph)
{
	int i;

	for (i = 0; i < MAX_NDAILY; i++) {
		if (ph->daily[i].xtime != 0)
			return 1;
	}

	for (i = 0; i < MAX_NMONTHLY; i++) {
		if (ph->monthly[i].xtime != 0)
			return 1;
	}

	return 0;
}

static void load_history_raw(void)
{
	char h_path[40];
	int i, n_len;
	history_t hist;

	if (g_ap_mode || nvram_match("rstats_stored", "0"))
		return;

	snprintf(h_path, sizeof(h_path), "%s/%s", "/etc/storage", "rstats-history");
	for (i = 0; i < MAX_HISTORY_IF; i++) {
		if (i > 0)
			snprintf(h_path, sizeof(h_path), "%s/%s.%s", "/etc/storage", "rstats-history", g_history_desc[i].ifdesc);
		n_len = f_read(h_path, &hist, sizeof(history_t));
		if (n_len == sizeof(history_t) && hist.id == CURRENT_ID) {
			memcpy(&g_history[i], &hist, sizeof(history_t));
			if (is_history_active(&hist))
				g_history_desc[i].is_active = 1;
		}
	}
}

static void save_history_raw(long ticks)
{
	char h_path[40];
	int i, rstats_stored, auto_save_th;

	if (g_ap_mode)
		return;

	rstats_stored = nvram_get_int("rstats_stored");
	if (rstats_stored < 1)
		return;

	snprintf(h_path, sizeof(h_path), "%s/%s", "/etc/storage", "rstats-history");
	for (i = 0; i < MAX_HISTORY_IF; i++) {
		if (i > 0) {
			if (!g_history_desc[i].is_active)
				continue;
			snprintf(h_path, sizeof(h_path), "%s/%s.%s", "/etc/storage", "rstats-history", g_history_desc[i].ifdesc);
		}
		f_write(h_path, &g_history[i], sizeof(history_t));
	}

	if (ticks < 1 || rstats_stored < 2)
		return;

	auto_save_th = 60 * 60 * 24 * 30;		// every month
	switch (rstats_stored)
	{
	case 3:
		auto_save_th = 60 * 60 * 24 * 14;	// every 2 weeks
		break;
	case 4:
		auto_save_th = 60 * 60 * 24 * 7;	// every week
		break;
	case 5:
		auto_save_th = 60 * 60 * 24 * 2;	// every 2 days
		break;
	case 6:
		auto_save_th = 60 * 60 * 24;		// every day
		break;
	case 7:
		auto_save_th = 60 * 60 * 12;		// every 12h
		break;
	}

	g_store_cntr += (ticks * RSTATS_INTERVAL);
	if (g_store_cntr >= auto_save_th) {
		g_store_cntr = 0;
		write_storage_to_mtd();
	}
}

static void save_speed_json(long next_time)
{
	FILE *fp;
	speed_item_t *item, *sp;
	int i, j, k, p, tail;
	uint64_t total;
	uint32_t tnow, tmax;
	char ch, *netdev, *netdev_1st;
	const char *fn_tmp = RSTATS_JS_SPEED ".tmp";

	if (next_time < 1)
		next_time = 1;
	else if (next_time > RSTATS_INTERVAL)
		next_time = RSTATS_INTERVAL;

	netdev = nvram_safe_get(RSTATS_NVKEY_24);
	netdev_1st = "";

	sp = NULL;
	SLIST_FOREACH(item, &g_speed_list.head, entries) {
		if (strcmp(item->ifdesc, netdev) == 0) {
			sp = item;
			break;
		}
		netdev_1st = item->ifdesc;
	}

	if (!sp) {
		netdev = netdev_1st;
		nvram_set(RSTATS_NVKEY_24, netdev);
	}

	tail = g_speed_list.tail;

	fp = fopen(fn_tmp, "w");
	if (!fp)
		return;

	i = 0;
	fprintf(fp, "\nnetdev = '%s';\n", netdev);
	fprintf(fp, "speed_history = {\n");
	SLIST_FOREACH(item, &g_speed_list.head, entries) {
		fprintf(fp, "%s'%s': {\n", (i) ? " },\n" : "", item->ifdesc);
		i++;
		if (strcmp(item->ifdesc, netdev) != 0)
			continue;
		for (j = 0; j < MAX_COUNTER; ++j) {
			total = 0;
			tmax = 0;
			fprintf(fp, "%sx: [", j ? ",\n t" : " r");
			p = tail;
			for (k = 0; k < MAX_NSPEED; ++k) {
				p = (p + 1) % MAX_NSPEED;
				tnow = item->speed[p][j];
				total += tnow;
				if (tnow > tmax)
					tmax = tnow;
				fprintf(fp, "%s%u", k ? "," : "", tnow);
			}
			fprintf(fp, "],\n");
			ch = j ? 't' : 'r';
			fprintf(fp, " %cx_avg: %llu,\n %cx_max: %u,\n %cx_total: %llu\n",
				ch, total / MAX_NSPEED, ch, tmax, ch, total * RSTATS_INTERVAL);
		}
	}
	fprintf(fp, "%s\n};\n", (i > 0) ? " }" : "");
	fprintf(fp, "data_period = %d;\npoll_next = %ld;\n", RSTATS_INTERVAL, next_time);

	fclose(fp);

	rename(fn_tmp, RSTATS_JS_SPEED);
}

static void save_data_js(FILE *fp, const history_t *ph, int mode)
{
	const data_t *data;
	int p, max, k;
	char comma = ' ';

	if (mode == DAILY) {
		data = ph->daily;
		p = ph->dailyp;
		max = MAX_NDAILY;
	} else {
		data = ph->monthly;
		p = ph->monthlyp;
		max = MAX_NMONTHLY;
	}

	fprintf(fp, "%s_history = [\n", (mode == DAILY) ? "daily" : "monthly");
	for (k = max; k > 0; --k) {
		p = (p + 1) % max;
		if (data[p].xtime == 0)
			continue;
		fprintf(fp, "%c[0x%lx,0x%llx,0x%llx]\n", comma,
			(unsigned long)data[p].xtime, (data[p].counter[0] >> 10), (data[p].counter[1] >> 10));
		if (comma != ',')
			comma = ',';
	}
	fprintf(fp, "];\n");
}

static void save_history_json(long next_time)
{
	FILE *fp;
	int i, wan_idx;
	char *netdev;
	const history_t *ph;
	const char *fn_tmp = RSTATS_JS_HISTORY ".tmp";

	if (next_time < 1)
		next_time = 1;
	else if (next_time > RSTATS_INTERVAL)
		next_time = RSTATS_INTERVAL;

	netdev = nvram_safe_get(RSTATS_NVKEY_DM);

	wan_idx = 0;
	for (i = 0; i < MAX_HISTORY_IF; i++) {
		if (strcmp(g_history_desc[i].ifdesc, netdev) == 0) {
			wan_idx = i;
			break;
		}
	}

	ph = &g_history[wan_idx];

	fp = fopen(fn_tmp, "w");
	if (fp) {
		fprintf(fp, "\nnetdev = '%s';\n", g_history_desc[wan_idx].ifdesc);
		fprintf(fp, "netdevs = [");
		for (i = 0; i < MAX_HISTORY_IF; i++) {
			if (i > 0) {
				if (!g_history_desc[i].is_active)
					continue;
			}
			fprintf(fp, "%s'%s'", (i) ? "," : "", g_history_desc[i].ifdesc);
		}
		fprintf(fp, "];\n");
		save_data_js(fp, ph, DAILY);
		save_data_js(fp, ph, MONTHLY);
		fprintf(fp, "poll_next = %ld;\n", next_time);
		fclose(fp);
		rename(fn_tmp, RSTATS_JS_HISTORY);
	}
}

static int is_system_time_valid(struct tm *tms)
{
	/* system time is not changed since boot */
	if (tms->tm_year <= (SYS_START_YEAR - 1900))
		return 0;

#if !defined (USE_RTC_HCTOSYS)
	/* NTP client updated at least one time */
	if (!is_ntpc_updated())
		return 0;
#endif

	return 1;
}

static void bump_history(data_t *data, int *tail, int max, uint32_t xnow, uint64_t *diff)
{
	int t, i;

	t = *tail;
	if (data[t].xtime != xnow) {
		for (i = max - 1; i >= 0; --i) {
			if (data[i].xtime == xnow) {
				t = i;
				break;
			}
		}
		if (i < 0) {
			*tail = t = (t + 1) % max;
			data[t].xtime = xnow;
			memset(data[t].counter, 0, sizeof(data[0].counter));
		}
	}

	for (i = 0; i < MAX_COUNTER; ++i)
		data[t].counter[i] += diff[i];
}

static void fill_speed_gaps(speed_item_t *sp, long ticks)
{
	int i, j, tail;

	tail = g_speed_list.tail;

	for (j = 0; j < ticks; ++j) {
		tail = (tail + 1) % MAX_NSPEED;
		for (i = 0; i < MAX_COUNTER; ++i)
			sp->speed[tail][i] = 0;
	}
}

static int iterate_netdev(const char *ifdesc, int ifindex, int wan_no, long ticks, uint64_t *counter)
{
	uint64_t diff[MAX_COUNTER];
	speed_item_t *item, *sp;
	int i, j, tail;

	sp = NULL;
	SLIST_FOREACH(item, &g_speed_list.head, entries) {
		if (strcmp(item->ifdesc, ifdesc) == 0) {
			sp = item;
			break;
		}
	}

	if (!sp) {
		if (g_speed_list.count >= MAX_SPEED_IF)
			return 0;
		sp = malloc(sizeof(speed_item_t));
		if (sp) {
			memset(sp, 0, sizeof(speed_item_t));
			strcpy(sp->ifdesc, ifdesc);
			sp->ifindex = ifindex;
			sp->tail = g_speed_list.tail;
			memcpy(sp->last, counter, sizeof(sp->last));
			SLIST_INSERT_HEAD(&g_speed_list.head, sp, entries);
			g_speed_list.count++;
		}
		return 0;
	}

	if (ticks < 1)
		return 0;

	if (sp->ifindex != ifindex) {
		sp->ifindex = ifindex;
		memset(sp->last, 0, sizeof(sp->last));
	}

	for (i = 0; i < MAX_COUNTER; ++i) {
		if (counter[i] < sp->last[i]) {
			diff[i] = 0;
			if ((sp->last[i] <= 0xFFFFFFFFull) && (sp->last[i] > 0x70000000ull))
				diff[i] = (0xFFFFFFFFull - sp->last[i]) + counter[i];
		} else
			diff[i] = counter[i] - sp->last[i];
		
		sp->last[i] = counter[i];
	}

	tail = g_speed_list.tail;

	for (j = 0; j < ticks; ++j) {
		tail = (tail + 1) % MAX_NSPEED;
		for (i = 0; i < MAX_COUNTER; ++i)
			sp->speed[tail][i] = (uint32_t)(diff[i] / ticks / RSTATS_INTERVAL);
	}

	sp->tail = tail;
	if (sp->skips)
		sp->skips = 0;

	if (wan_no > 0 && wan_no <= MAX_HISTORY_IF && !g_ap_mode) {
		time_t now;
		struct tm *tms;
		history_t *ph;
		
		if (g_history_desc[wan_no-1].is_wisp) {
			if (!get_wan_wisp_active(NULL))
				return 1;
		}
		
		now = time(NULL);
		tms = localtime(&now);
		
		if (is_system_time_valid(tms)) {
			ph = &g_history[wan_no-1];
			if (!g_history_desc[wan_no-1].is_active)
				g_history_desc[wan_no-1].is_active = 1;
			bump_history(ph->daily, &ph->dailyp, MAX_NDAILY, (tms->tm_year << 16) | ((uint32_t)tms->tm_mon << 8) | tms->tm_mday, diff);
			bump_history(ph->monthly, &ph->monthlyp, MAX_NMONTHLY, (tms->tm_year << 16) | ((uint32_t)tms->tm_mon << 8), diff);
		}
	}

	return 1;
}

static void process_rstats(void)
{
	FILE *fp;
	char buf[256], *p, *ifname;
	const char *ifdesc;
	speed_item_t *item, *next;
	uint64_t counter[MAX_COUNTER];
	long tick, ticks;
	int i, ifindex, wan_no, tail_new;

	tick = g_uptime_now - g_uptime_old;
	ticks = tick / RSTATS_INTERVAL;

	fp = fopen("/proc/net/dev", "r");
	if (fp) {
		fgets(buf, sizeof(buf), fp);
		fgets(buf, sizeof(buf), fp);
		while (fgets(buf, sizeof(buf), fp)) {
			if ((p = strchr(buf, ':')) == NULL)
				continue;
			
			*p = 0;
			if ((ifname = strrchr(buf, ' ')) == NULL)
				ifname = buf;
			else
				++ifname;
			
			if (strcmp(ifname, "lo") == 0)
				continue;
			
			wan_no = 0;
			ifindex = 0;
			ifdesc = get_ifname_descriptor(ifname, g_ap_mode, &ifindex, &wan_no);
			if (!ifdesc)
				continue;
			
			// <rx bytes, packets, errors, dropped, fifo errors, frame errors, compressed, multicast><tx ...>
			if (sscanf(p + 1, "%llu%*u%*u%*u%*u%*u%*u%*u%llu", &counter[0], &counter[1]) != 2)
				continue;
			
			if (!iterate_netdev(ifdesc, ifindex, wan_no, ticks, counter))
				continue;
		}
		fclose(fp);
	}

#if !defined(RSTATS_SKIP_ESW)
	for (i = 0; i < BOARD_NUM_ETH_EPHY; i++) {
		if (phy_status_port_bytes(i, &counter[0], &counter[1]) < 0)
			continue;
		snprintf(buf, sizeof(buf), "ESW_P%d", i);
		if (!iterate_netdev(buf, 0, 0, ticks, counter))
			continue;
	}
#endif

	if (ticks > 0) {
		tail_new = (g_speed_list.tail + ticks) % MAX_NSPEED;
		SLIST_FOREACH_SAFE(item, &g_speed_list.head, entries, next) {
			if (item->tail != tail_new) {
				fill_speed_gaps(item, ticks);
				item->tail = tail_new;
				item->skips += ticks;
				if (item->skips >= MAX_NSPEED) {
					SLIST_REMOVE(&g_speed_list.head, item, speed_item, entries);
					free(item);
				}
			}
		}
		g_speed_list.tail = tail_new;
		g_uptime_old += (ticks * RSTATS_INTERVAL);
		
		save_history_raw(ticks);
	}
}

static void catch_sig_rstats(int sig)
{
	switch (sig) {
	case SIGTERM:
		save_history_raw(0);
		free_rstats();
		exit(0);
		break;
	case SIGHUP:
		gothup = 1;
		break;
	case SIGUSR1:
		gotusr1 = 1;
		break;
	case SIGUSR2:
		gotusr2 = 1;
		break;
	}
}

void notify_rstats_time(void)
{
	kill_pidfile_s(RSTATS_PID_FILE, SIGHUP);
}

int rstats_main(int argc, char *argv[])
{
	FILE *fp;
	pid_t pid;
	struct sigaction sa;
	long z;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_rstats;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, NULL);

	printf("rstats\nCopyright (C) 2006-2009 Jonathan Zarate\n\n");

	if (daemon(0, 0) < 0) {
		perror("daemon");
		exit(errno);
	}

	if (alloc_rstats() < 0) {
		free_rstats();
		exit(1);
	}

	pid = getpid();

	/* never invoke oom killer */
	oom_score_adjust(pid, OOM_SCORE_ADJ_MIN);

	/* write pid */
	if ((fp = fopen(RSTATS_PID_FILE, "w")) != NULL) {
		fprintf(fp, "%d", pid);
		fclose(fp);
	}

	g_ap_mode = get_ap_mode();

	load_history_raw();

	z = uptime();
	g_uptime_now = z;
	g_uptime_old = z;

	while (1) {
		while (g_uptime_now < z) {
			sleep(z - g_uptime_now);
			if (gothup) {
				setenv_tz();
				gothup = 0;
			}
			if (gotusr1) {
				save_speed_json(z - uptime());
				gotusr1 = 0;
			}
			if (gotusr2) {
				save_history_json(z - uptime());
				gotusr2 = 0;
			}
			g_uptime_now = uptime();
		}
		process_rstats();
		z += RSTATS_INTERVAL;
	}

	free_rstats();

	return 0;
}
