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
#include <syslog.h>
#include <fcntl.h>

#include "rc.h"

#define STORAGE_PATH  "/etc/storage/rstats-history"

#define K 1024
#define M (1024 * 1024)
#define G (1024 * 1024 * 1024)

#define SMIN		60
#define	SHOUR		(60 * 60)
#define	SDAY		(60 * 60 * 24)

#define INTERVAL	60

#define MAX_NSPEED	((24 * SHOUR) / INTERVAL)
#define MAX_NDAILY	62
#define MAX_NMONTHLY	25
#define MAX_SPEED_IF	IFDESCS_MAX_NUM
#define MAX_ROLLOVER	(225 * M)

#define MAX_COUNTER	2
#define RX 		0
#define TX 		1

#define DAILY		0
#define MONTHLY		1

#define CURRENT_ID	0x31305352

typedef struct {
	uint32_t xtime;
	uint64_t counter[MAX_COUNTER];
} data_t;

typedef struct {
	char ifdesc[16];
	long utime;
	uint64_t speed[MAX_NSPEED][MAX_COUNTER];
	uint64_t last[MAX_COUNTER];
	int tail;
	int sync;
} speed_t;

typedef struct {
	uint32_t id;
	data_t daily[MAX_NDAILY];
	int dailyp;
	data_t monthly[MAX_NMONTHLY];
	int monthlyp;
} history_t;

history_t history;
speed_t speed[MAX_SPEED_IF];
static int speed_count = 0;
static int wwan_deleted = 0;
static long now_uptime;

volatile int gothup = 0;
volatile int gotuser = 0;
volatile int gotterm = 0;

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

static void clear_history(void)
{
	memset(speed, 0, sizeof(speed));

	memset(&history, 0, sizeof(history));
	history.id = CURRENT_ID;
}

static void load_history(void)
{
	int n;
	history_t hist;

	if (nvram_match("rstats_stored", "0"))
		return;

	memset(&hist, 0, sizeof(hist));
	n = f_read(STORAGE_PATH, &hist, sizeof(hist));
	
	if (n == sizeof(hist) && hist.id == CURRENT_ID) {
		memcpy(&history, &hist, sizeof(history));
	}
}

static void save_history(void)
{
	if (nvram_match("rstats_stored", "0"))
		return;

	f_write(STORAGE_PATH, &history, sizeof(history));
}

static void save_speedjs(long next)
{
	int i, j, k;
	speed_t *sp;
	int p;
	FILE *f;
	uint64_t total;
	uint64_t tmax;
	uint64_t tnow;
	char c;

	if ((f = fopen("/var/tmp/rstats-speed.js", "w")) == NULL) return;

	fprintf(f, "\nspeed_history = {\n");

	for (i = 0; i < speed_count; ++i) {
		sp = &speed[i];
		fprintf(f, "%s'%s': {\n", i ? " },\n" : "", sp->ifdesc);
		for (j = 0; j < MAX_COUNTER; ++j) {
			total = 0;
			tmax = 0;
			fprintf(f, "%sx: [", j ? ",\n t" : " r");
			p = sp->tail;
			for (k = 0; k < MAX_NSPEED; ++k) {
				p = (p + 1) % MAX_NSPEED;
				tnow = sp->speed[p][j];
				fprintf(f, "%s%llu", k ? "," : "", tnow);
				total += tnow;
				if (tnow > tmax) tmax = tnow;
			}
			fprintf(f, "],\n");
			c = j ? 't' : 'r';
			fprintf(f, " %cx_avg: %llu,\n %cx_max: %llu,\n %cx_total: %llu",
				c, total / MAX_NSPEED, c, tmax, c, total);
		}
	}
	fprintf(f, "%s_next: %ld};\n", speed_count ? "},\n" : "", ((next >= 1) ? next : 1));
	fclose(f);

	rename("/var/tmp/rstats-speed.js", "/var/spool/rstats-speed.js");
}

static void save_datajs(FILE *f, int mode)
{
	data_t *data;
	int p;
	int max;
	int k, kn;

	fprintf(f, "\n%s_history = [\n", (mode == DAILY) ? "daily" : "monthly");

	if (mode == DAILY) {
		data = history.daily;
		p = history.dailyp;
		max = MAX_NDAILY;
	}
	else {
		data = history.monthly;
		p = history.monthlyp;
		max = MAX_NMONTHLY;
	}
	kn = 0;
	for (k = max; k > 0; --k) {
		p = (p + 1) % max;
		if (data[p].xtime == 0) continue;
		fprintf(f, "%s[0x%lx,0x%llx,0x%llx]", kn ? "," : "",
			(unsigned long)data[p].xtime, (data[p].counter[0] >> 10), (data[p].counter[1] >> 10));
		++kn;
	}
	fprintf(f, "];\n");
}

static void save_histjs(void)
{
	FILE *f;

	if ((f = fopen("/var/tmp/rstats-history.js", "w")) != NULL) {
		save_datajs(f, DAILY);
		save_datajs(f, MONTHLY);
		fclose(f);
		rename("/var/tmp/rstats-history.js", "/var/spool/rstats-history.js");
	}
}

static void bump(data_t *data, int *tail, int max, uint32_t xnow, uint64_t *counter)
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
	for (i = 0; i < MAX_COUNTER; ++i) {
		data[t].counter[i] += counter[i];
	}
}

static void calc(void)
{
	FILE *f;
	char buf[256];
	char *p, *ifname;
	const char *ifdesc;
	uint64_t counter[MAX_COUNTER];
	speed_t *sp;
	int i, j, is_wwan, wwan_found;
	time_t now;
	time_t mon;
	struct tm *tms;
	uint64_t diff;
	long tick, n;

	now = time(0);
	wwan_found = 0;

	if (check_if_file_exist(FLAG_FILE_WWAN_GONE)) {
		wwan_deleted = 1;
		unlink(FLAG_FILE_WWAN_GONE);
	}

	if ((f = fopen("/proc/net/dev", "r")) == NULL) return;
	fgets(buf, sizeof(buf), f);	// header
	fgets(buf, sizeof(buf), f);	// "
	while (fgets(buf, sizeof(buf), f)) {
		if ((p = strchr(buf, ':')) == NULL) continue;
		*p = 0;
		if ((ifname = strrchr(buf, ' ')) == NULL) ifname = buf;
		else ++ifname;
		
		ifdesc = get_ifname_descriptor(ifname);
		if (!ifdesc)
			continue;
		
		// <rx bytes, packets, errors, dropped, fifo errors, frame errors, compressed, multicast><tx ...>
		if (sscanf(p + 1, "%llu%*u%*u%*u%*u%*u%*u%*u%llu", &counter[0], &counter[1]) != 2) continue;
		
		is_wwan = (strcmp(ifdesc, IFDESC_WWAN) == 0) ? 1 : 0;
		
		sp = speed;
		for (i = speed_count; i > 0; --i) {
			if (strcmp(sp->ifdesc, ifdesc) == 0) break;
			++sp;
		}
		if (i == 0) {
			if (speed_count >= MAX_SPEED_IF) continue;
			i = speed_count++;
			sp = &speed[i];
			memset(sp, 0, sizeof(*sp));
			strcpy(sp->ifdesc, ifdesc);
			sp->sync = 1;
			sp->utime = now_uptime;
		}
		if (sp->sync) {
			sp->sync = 0;
			memcpy(sp->last, counter, sizeof(sp->last));
			memset(counter, 0, sizeof(counter));
		}
		else {
			tick = now_uptime - sp->utime;
			n = tick / INTERVAL;
			if (n < 1) {
				continue;
			}
			sp->utime += (n * INTERVAL);
			
			for (i = 0; i < MAX_COUNTER; ++i) {
				if (counter[i] < sp->last[i]) {
					uint64_t min_limit = 0x70000000; // ~40MB/s max for Wireless (Wired used 64 bit counters).
					diff = 0;
					if (is_wwan) {
						min_limit = 0xD3000000; // ~12MB/s max for LTE
						if (wwan_deleted && (counter[i] < 0x2CFFFFFFul))
							diff = counter[i];
					}
					
					if (!diff && (sp->last[i] <= 0xFFFFFFFFull) && (sp->last[i] > min_limit))
						diff = (0xFFFFFFFFull - sp->last[i]) + counter[i];
				}
				else {
					diff = counter[i] - sp->last[i];
				}
				sp->last[i] = counter[i];
				counter[i] = diff;
			}
			for (j = 0; j < n; ++j) {
				sp->tail = (sp->tail + 1) % MAX_NSPEED;
				for (i = 0; i < MAX_COUNTER; ++i) {
					sp->speed[sp->tail][i] = (counter[i] / n);
				}
			}
		}
		
		if (is_wwan || strcmp(ifdesc, IFDESC_WAN) == 0) {
			tms = localtime(&now);
			if (tms->tm_year >= (2012-1900)) {
				bump(history.daily, &history.dailyp, MAX_NDAILY, (tms->tm_year << 16) | ((uint32_t)tms->tm_mon << 8) | tms->tm_mday, counter);
				n = 1;
				mon = now + ((1 - n) * (60 * 60 * 24));
				tms = localtime(&mon);
				bump(history.monthly, &history.monthlyp, MAX_NMONTHLY, (tms->tm_year << 16) | ((uint32_t)tms->tm_mon << 8), counter);
			}
		}
		
		if (is_wwan)
			wwan_found = 1;
	}
	fclose(f);

	wwan_deleted = !wwan_found;

	save_history();
}

static void sig_handler(int sig)
{
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		gotterm = 1;
		break;
	case SIGHUP:
		gothup = 1;
		break;
	case SIGUSR1:
		gotuser = 1;
		break;
	case SIGUSR2:
		gotuser = 2;
		break;
	}
}

void notify_rstats_time(void)
{
	doSystem("killall %s %s", "-SIGHUP", "rstats");
}

int rstats_main(int argc, char *argv[])
{
	struct sigaction sa;
	pid_t pid;
	long z;

	printf("rstats\nCopyright (C) 2006-2009 Jonathan Zarate\n\n");

	sa.sa_handler = sig_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	if (daemon(0, 0) < 0) {
		perror("daemon");
		exit(errno);
	}

	pid = getpid();

	/* never invoke oom killer */
	oom_score_adjust(pid, OOM_SCORE_ADJ_MIN);

	clear_history();
	load_history();

	z = now_uptime = uptime();
	while (1) {
		while (now_uptime < z) {
			sleep(z - now_uptime);
			if (gothup) {
				setenv_tz();
				gothup = 0;
			}
			if (gotterm) {
				save_history();
				exit(0);
			}
			if (gotuser == 1) {
				save_speedjs(z - uptime());
				gotuser = 0;
			}
			else if (gotuser == 2) {
				save_histjs();
				gotuser = 0;
			}
			now_uptime = uptime();
		}
		calc();
		z += INTERVAL;
	}

	return 0;
}
