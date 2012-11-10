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
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <stdint.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>

#include <nvram/bcmnvram.h>
#include <shutils.h>
#include "rstats.h"
#include "traffic.h"


//	#define DEBUG_NOISY
//	#define DEBUG_STIME


#ifdef DEBUG_NOISY
#define _dprintf(args...)	cprintf(args)
#else
#define _dprintf(args...)	do { } while (0)
#endif



#define K 1024
#define M (1024 * 1024)
#define G (1024 * 1024 * 1024)

#define SMIN	60
#define	SHOUR	(60 * 60)
#define	SDAY	(60 * 60 * 24)
#define Y2K		946684800UL

#define INTERVAL		120

#define MAX_NSPEED		((24 * SHOUR) / INTERVAL)
#define MAX_NDAILY		62
#define MAX_NMONTHLY	25
#define MAX_SPEED_IF	7
#define MAX_ROLLOVER	(225 * M)

#define MAX_COUNTER	2
#define RX 			0
#define TX 			1

#define DAILY		0
#define MONTHLY		1

#define ID_V0		0x30305352
#define ID_V1		0x31305352
#define CURRENT_ID	ID_V1


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

typedef struct {
	uint32_t id;

	data_t daily[62];
	int dailyp;

	data_t monthly[12];
	int monthlyp;
} history_v0_t;

typedef struct {
	char ifname[12];
	long utime;
	unsigned long speed[MAX_NSPEED][MAX_COUNTER];
	unsigned long last[MAX_COUNTER];
	int tail;
	char sync;
} speed_t;

history_t history;
speed_t speed[MAX_SPEED_IF];
int speed_count;
long save_utime;
char save_path[96];
long uptime;

volatile int gothup = 0;
volatile int gotuser = 0;
volatile int gotterm = 0;

const char history_fn[] = "/var/lib/misc/rstats-history";
const char speed_fn[] = "/var/lib/misc/rstats-speed";
const char uncomp_fn[] = "/var/tmp/rstats-uncomp";
const char source_fn[] = "/var/lib/misc/rstats-source";
static const char base64_xlat[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// ===========================================

size_t
strlcpy(char *dst, const char *src, size_t siz)
{
        char *d = dst;
        const char *s = src;
        size_t n = siz;

        /* Copy as many bytes as will fit */
        if (n != 0 && --n != 0) {
                do {
                        if ((*d++ = *s++) == 0)
                                break;
                } while (--n != 0);
        }

        /* Not enough room in dst, add NUL and traverse rest of src */
        if (n == 0) {
                if (siz != 0)
                        *d = '\0';                /* NUL-terminate dst */
                while (*s++)
                        ;
        }

        return(s - src - 1);        /* count does not include NUL */
}

int check_action(void)
{
	int a;
	int r = 3;

	while (f_read("/var/lock/action", &a, sizeof(a)) != sizeof(a)) {
		sleep(1);
		if (--r == 0) return ACT_UNKNOWN;
	}
	return a;
}

int wait_action_idle(int n)
{
	while (n-- > 0) {
		if (check_action() == ACT_IDLE) return 1;
		sleep(1);
	}
	return 0;
}

long get_uptime(void)
{
	struct sysinfo si;
	sysinfo(&si);
	return si.uptime;
}

int base64_encode(const unsigned char *in, char *out, int inlen)
{
	char *o;
	o = out;
	while (inlen >= 3) {
		*out++ = base64_xlat[*in >> 2];
		*out++ = base64_xlat[((*in << 4) & 0x3F) | (*(in + 1) >> 4)];
		++in;	// note: gcc complains if *(++in)
		*out++ = base64_xlat[((*in << 2) & 0x3F) | (*(in + 1) >> 6)];
		++in;
		*out++ = base64_xlat[*in++ & 0x3F];
		inlen -= 3;
	}
	if (inlen > 0) {
		*out++ = base64_xlat[*in >> 2];
		if (inlen == 2) {
			*out++ = base64_xlat[((*in << 4) & 0x3F) | (*(in + 1) >> 4)];
			++in;
			*out++ = base64_xlat[((*in << 2) & 0x3F)];
		}
		else {
			*out++ = base64_xlat[(*in << 4) & 0x3F];
			*out++ = '=';
		}
		*out++ = '=';
	}
	return out - o;
}

int base64_decode(const char *in, unsigned char *out, int inlen)
{
	char *p;
	int n;
	unsigned long x;
	unsigned char *o;
	char c;

	o = out;
	n = 0;
	x = 0;
	while (inlen-- > 0) {
		if (*in == '=') break;
		if ((p = strchr(base64_xlat, c = *in++)) == NULL) {
//			printf("ignored - %x %c\n", c, c);
			continue;	// note: invalid characters are just ignored
		}
		x = (x << 6) | (p - base64_xlat);
		if (++n == 4) {
			*out++ = x >> 16;
			*out++ = (x >> 8) & 0xFF;
			*out++ = x & 0xFF;
			n = 0;
			x = 0;
		}
	}
	if (n == 3) {
		*out++ = x >> 10;
		*out++ = (x >> 2) & 0xFF;
	}
	else if (n == 2) {
		*out++ = x >> 4;
	}
	return out - o;
}

int base64_encoded_len(int len)
{
	return ((len + 2) / 3) * 4;
}

int base64_decoded_len(int len)
{
	return ((len + 3) / 4) * 3;
}

const char *find_word(const char *buffer, const char *word)
{
	const char *p, *q;
	int n;

	n = strlen(word);
	p = buffer;
	while ((p = strstr(p, word)) != NULL) {
		if ((p == buffer) || (*(p - 1) == ' ') || (*(p - 1) == ',')) {
			q = p + n;
			if ((*q == ' ') || (*q == ',') || (*q == 0)) {
				return p;
			}
		}
		++p;
	}
	return NULL;
}

// ===========================================

static int get_stime(void)
{
#ifdef DEBUG_STIME
	return 90;
#else
	int t;
	t = nvram_get_int("rstats_stime");
	if (t < 1) t = 1;
		else if (t > 8760) t = 8760;
	return t * SHOUR;
#endif
}

static int comp(const char *path, void *buffer, int size)
{
	char s[256];

	if (f_write(path, buffer, size, 0, 0) != size) return 0;

	sprintf(s, "%s.gz", path);
	unlink(s);

	sprintf(s, "gzip %s", path);
	return system(s) == 0;
}

static void save(int quick)
{
	int i;
	char *bi, *bo;
	int n;
	char hgz[256];
	char tmp[256];
	char bak[256];
	time_t now;
	struct tm *tms;
	static int lastbak = -1;

	_dprintf("%s: quick=%d\n", __FUNCTION__, quick);

	f_write("/var/lib/misc/rstats-stime", &save_utime, sizeof(save_utime), 0, 0);

	comp(speed_fn, speed, sizeof(speed[0]) * speed_count); 

/*
	if ((now = time(0)) < Y2K) {
		_dprintf("%s: time not set\n", __FUNCTION__);
		return;
	}
*/

  comp(history_fn, &history, sizeof(history));

	_dprintf("%s: write source=%s\n", __FUNCTION__, save_path);
	f_write_string(source_fn, save_path, 0, 0);

	if (quick) {
		return;
	}

	sprintf(hgz, "%s.gz", history_fn);

	if (strcmp(save_path, "*nvram") == 0) {
//		if (!wait_action_idle(10)) {
//			_dprintf("%s: busy, not saving\n", __FUNCTION__);
//			return;
//		}

		if ((n = f_read_alloc(hgz, &bi, 20 * 1024)) > 0) {
			if ((bo = malloc(base64_encoded_len(n) + 1)) != NULL) {
				n = base64_encode(bi, bo, n);
				bo[n] = 0;
				nvram_set("rstats_data", bo);
				if (!nvram_match("debug_nocommit", "1")) nvram_commit();

				_dprintf("%s: nvram commit\n", __FUNCTION__);

				free(bo);
			}
		}
		free(bi);
	}
	else if (save_path[0] != 0) {
		strcpy(tmp, save_path);
		strcat(tmp, ".tmp");

		for (i = 15; i > 0; --i) {
//			if (!wait_action_idle(10)) {
//				_dprintf("%s: busy, not saving\n", __FUNCTION__);
//			}
//			else {
				_dprintf("%s: cp %s %s\n", __FUNCTION__, hgz, tmp);
				if (eval("cp", hgz, tmp) == 0) {
					_dprintf("%s: copy ok\n", __FUNCTION__);

					if (!nvram_match("rstats_bak", "0")) {
						now = time(0);
						tms = localtime(&now);
						if (lastbak != tms->tm_yday) {
							strcpy(bak, save_path);
							n = strlen(bak);
							if ((n > 3) && (strcmp(bak + (n - 3), ".gz") == 0)) n -= 3;
							sprintf(bak + n, "_%d.bak", ((tms->tm_yday / 7) % 3) + 1);
							if (eval("cp", save_path, bak) == 0) lastbak = tms->tm_yday;
						}
					}

					_dprintf("%s: rename %s %s\n", __FUNCTION__, tmp, save_path);
					if (rename(tmp, save_path) == 0) {
						_dprintf("%s: rename ok\n", __FUNCTION__);
						break;
					}
				}
//			}

			// might not be ready
			sleep(3);
			if (gotterm) break;
		}
	}
}

static int decomp(const char *fname, void *buffer, int size, int max)
{
	char s[256];
	int n;

	_dprintf("%s: fname=%s\n", __FUNCTION__, fname);

	unlink(uncomp_fn);

	n = 0;
	sprintf(s, "gzip -dc %s > %s", fname, uncomp_fn);

	if (system(s) == 0) {
		n = f_read(uncomp_fn, buffer, size * max);
		_dprintf("%s: n=%d\n", __FUNCTION__, n);
		if (n <= 0) n = 0;
			else n = n / size;
	}
	else {
		_dprintf("%s: %s != 0\n", __FUNCTION__, s);
	}
	unlink(uncomp_fn);
	memset((char *)buffer + (size * n), 0, (max - n) * size);
	return n;
}

static void clear_history(void)
{
	memset(&history, 0, sizeof(history));
	history.id = CURRENT_ID;
}

static int load_history(const char *fname)
{
	history_t hist;

	_dprintf("%s: fname=%s\n", __FUNCTION__, fname);

	if ((decomp(fname, &hist, sizeof(hist), 1) != 1) || (hist.id != CURRENT_ID)) {
		history_v0_t v0;

		if ((decomp(fname, &v0, sizeof(v0), 1) != 1) || (v0.id != ID_V0)) {
			_dprintf("%s: load failed\n", __FUNCTION__);
			return 0;
		}
		else {
			// --- temp conversion ---
			clear_history();

			// V0 -> V1
			history.id = CURRENT_ID;
			memcpy(history.daily, v0.daily, sizeof(history.daily));
			history.dailyp = v0.dailyp;
			memcpy(history.monthly, v0.monthly, sizeof(v0.monthly));	// v0 is just shorter
			history.monthlyp = v0.monthlyp;
		}
	}
	else {
		memcpy(&history, &hist, sizeof(history));
	}

	_dprintf("%s: dailyp=%d monthlyp=%d\n", __FUNCTION__, history.dailyp, history.monthlyp);
	return 1;
}

static void load_new(void)
{
	char hgz[256];

	sprintf(hgz, "%s.gz.new", history_fn);
	if (load_history(hgz)) save(0);
	unlink(hgz);
}

static void load(int new)
{
	int i;
	long t;
	char *bi, *bo;
	int n;
	char hgz[256];
	char sp[sizeof(save_path)];
	unsigned char mac[6];

	uptime = get_uptime();

	strlcpy(save_path, nvram_safe_get("rstats_path"), sizeof(save_path) - 32);
	if (((n = strlen(save_path)) > 0) && (save_path[n - 1] == '/')) {
		ether_atoe(nvram_safe_get("et0macaddr"), mac);
		sprintf(save_path + n, "tomato_rstats_%02x%02x%02x%02x%02x%02x.gz",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

	if (f_read("/var/lib/misc/rstats-stime", &save_utime, sizeof(save_utime)) != sizeof(save_utime)) {
		save_utime = 0;
	}
	t = uptime + get_stime();
	if ((save_utime < uptime) || (save_utime > t)) save_utime = t;
	_dprintf("%s: uptime = %dm, save_utime = %dm\n", __FUNCTION__, uptime / 60, save_utime / 60);

	sprintf(hgz, "%s.gz", speed_fn);
	speed_count = decomp(hgz, speed, sizeof(speed[0]), MAX_SPEED_IF);
	_dprintf("%s: speed_count = %d\n", __FUNCTION__, speed_count);

	for (i = 0; i < speed_count; ++i) {
		if (speed[i].utime > uptime) {
			speed[i].utime = uptime;
			speed[i].sync = 1;
		}
	}

	sprintf(hgz, "%s.gz", history_fn);

	if (new) {
		unlink(hgz);
		save_utime = 0;
		return;
	}

	f_read_string(source_fn, sp, sizeof(sp));	// always terminated
	_dprintf("%s: read source=%s save_path=%s\n", __FUNCTION__, sp, save_path);
	if ((strcmp(sp, save_path) == 0) && (load_history(hgz))) {
		_dprintf("%s: using local file\n", __FUNCTION__);
		return;
	}

	if (save_path[0] != 0) {
		if (strcmp(save_path, "*nvram") == 0) {
//			if (!wait_action_idle(60)) exit(0);

			bi = nvram_safe_get("rstats_data");
			if ((n = strlen(bi)) > 0) {
				if ((bo = malloc(base64_decoded_len(n))) != NULL) {
					n = base64_decode(bi, bo, n);
					_dprintf("%s: nvram n=%d\n", __FUNCTION__, n);
					f_write(hgz, bo, n, 0, 0);
					free(bo);
					load_history(hgz);
				}
			}
		}
		else {
			i = 1;
			while (1) {
//				if (wait_action_idle(10)) {

					// cifs quirk: try forcing refresh
					eval("ls", save_path);

					if (load_history(save_path)) {
						f_write_string(source_fn, save_path, 0, 0);
						break;
					}
//				}

				// not ready...
				sleep(i);
				if ((i *= 2) > 900) i = 900;	// 15m

				if (gotterm) {
					save_path[0] = 0;
					return;
				}

				if (i > (3 * 60)) {
					syslog(LOG_WARNING, "Problem loading %s. Still trying...", save_path);
				}
			}
		}
	}
}

static void save_speedjs(long next)
{
	int i, j, k;
	speed_t *sp;
	int p;
	FILE *f;
	uint64_t total;
	uint64_t tmax;
	unsigned long n;
	char c;

	if ((f = fopen("/var/tmp/rstats-speed.js", "w")) == NULL) return;

	_dprintf("%s: speed_count = %d\n", __FUNCTION__, speed_count);

	fprintf(f, "\nspeed_history = {\n");

	for (i = 0; i < speed_count; ++i) {
		sp = &speed[i];
		fprintf(f, "%s'%s': {\n", i ? " },\n" : "", sp->ifname);
		for (j = 0; j < MAX_COUNTER; ++j) {
			total = tmax = 0;
			fprintf(f, "%sx: [", j ? ",\n t" : " r");
			p = sp->tail;
			for (k = 0; k < MAX_NSPEED; ++k) {
				p = (p + 1) % MAX_NSPEED;
				n = sp->speed[p][j];
				fprintf(f, "%s%lu", k ? "," : "", n);
				total += n;
				if (n > tmax) tmax = n;
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
			(unsigned long)data[p].xtime, data[p].counter[0] / K, data[p].counter[1] / K);
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

static void bump(data_t *data, int *tail, int max, uint32_t xnow, unsigned long *counter)
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
	char *ifname, *ifname_real;
	char *p;
	unsigned long counter[MAX_COUNTER];
	speed_t *sp;
	int i, j;
	time_t now;
	time_t mon;
	struct tm *tms;
	uint32_t c;
	uint32_t sc;
	unsigned long diff;
	long tick;
	int n;
	char *exclude;

	now = time(0);
//	exclude = nvram_safe_get("rstats_exclude");

	if ((f = fopen("/proc/net/dev", "r")) == NULL) return;
	fgets(buf, sizeof(buf), f);	// header
	fgets(buf, sizeof(buf), f);	// "
	while (fgets(buf, sizeof(buf), f)) {
		if ((p = strchr(buf, ':')) == NULL) continue;
		*p = 0;
		if ((ifname = strrchr(buf, ' ')) == NULL) ifname = buf;
			else ++ifname;
		ifname_real = ifname;
//		if (strncmp(ifname, "ppp", 3) == 0) ifname = "ppp";
//		if ((strcmp(ifname, "lo") == 0) || (find_word(exclude, ifname))) continue;

		// <rx bytes, packets, errors, dropped, fifo errors, frame errors, compressed, multicast><tx ...>
		if (sscanf(p + 1, "%lu%*u%*u%*u%*u%*u%*u%*u%lu", &counter[0], &counter[1]) != 2) continue;

		sp = speed;
		for (i = speed_count; i > 0; --i) {
			if (strcmp(sp->ifname, ifname) == 0) break;
			++sp;
		}
		if (i == 0) {
			if (speed_count >= MAX_SPEED_IF) continue;

			_dprintf("%s: add %s as #%d\n", __FUNCTION__, ifname, speed_count);

			i = speed_count++;
			sp = &speed[i];
			memset(sp, 0, sizeof(*sp));
			strcpy(sp->ifname, ifname);
			sp->sync = 1;
			sp->utime = uptime;
		}
		if (sp->sync) {
			_dprintf("%s: sync %s\n", __FUNCTION__, ifname);
			sp->sync = -1;

			memcpy(sp->last, counter, sizeof(sp->last));
			memset(counter, 0, sizeof(counter));
		}
		else {
			sp->sync = -1;

			tick = uptime - sp->utime;
			n = tick / INTERVAL;
			if (n < 1) {
				_dprintf("%s: %s is a little early... %d < %d\n", __FUNCTION__, ifname, tick, INTERVAL);
				continue;
			}

			sp->utime += (n * INTERVAL);
			_dprintf("%s: %s n=%d tick=%d\n", __FUNCTION__, ifname, n, tick);

			for (i = 0; i < MAX_COUNTER; ++i) {
				c = counter[i];
				sc = sp->last[i];
				if (c < sc) {
					diff = (0xFFFFFFFF - sc) + c;
					if (diff > MAX_ROLLOVER) diff = 0;
				}
				else {
					 diff = c - sc;
				}
				sp->last[i] = c;
				counter[i] = diff;
			}

			for (j = 0; j < n; ++j) {
				sp->tail = (sp->tail + 1) % MAX_NSPEED;
				for (i = 0; i < MAX_COUNTER; ++i) {
					sp->speed[sp->tail][i] = counter[i] / n;
				}
			}
		}

//		_dprintf("\n");

		// todo: split, delay

//		if ((now > Y2K) && (!nvram_match("wan_proto", "disabled")) && (nvram_match("wan_iface", ifname_real))) {
			tms = localtime(&now);
			bump(history.daily, &history.dailyp, MAX_NDAILY,
				(tms->tm_year << 16) | ((uint32_t)tms->tm_mon << 8) | tms->tm_mday, counter);

			n = nvram_get_int("rstats_offset");
			if ((n < 1) || (n > 31)) n = 1;
			mon = now + ((1 - n) * (60 * 60 * 24));
			tms = localtime(&mon);
			bump(history.monthly, &history.monthlyp, MAX_NMONTHLY,
				(tms->tm_year << 16) | ((uint32_t)tms->tm_mon << 8), counter);
//		}
	}
	fclose(f);

	// cleanup stale entries
	for (i = 0; i < speed_count; ++i) {
		sp = &speed[i];
		if (sp->sync == -1) {
			sp->sync = 0;
			continue;
		}
		if ((uptime - sp->utime) > (10 * SMIN))  {
			_dprintf("%s: #%d removing. > time limit or excluded\n", __FUNCTION__, i);
			--speed_count;
			memcpy(sp, sp + 1, (speed_count - i) * sizeof(speed[0]));
		}
		else {
			_dprintf("%s: %s not found setting sync=1\n", __FUNCTION__, sp->ifname, i);
			sp->sync = 1;
		}
	}

	// todo: total > user
	if (uptime >= save_utime) {
		save(0);
		save_utime = uptime + get_stime();
		_dprintf("%s: uptime = %dm, save_utime = %dm\n", __FUNCTION__, uptime / 60, save_utime / 60);
	}
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

int main(int argc, char *argv[])
{
	struct sigaction sa;
	long z;
	int new;

	printf("rstats\nCopyright (C) 2006-2009 Jonathan Zarate\n\n");

	if (fork() != 0) return 0;

	openlog("rstats", LOG_PID, LOG_USER);

	new = 0;
	if (argc > 1) {
		if (strcmp(argv[1], "--new") == 0) {
			new = 1;
			_dprintf("new=1\n");
		}
	}

	clear_history();
	unlink("/var/tmp/rstats-load");

	sa.sa_handler = sig_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

//	load(new);			// J++

	z = uptime = get_uptime();
	while (1) {
		while (uptime < z) {
			sleep(z - uptime);
			if (gothup) {
				if (unlink("/var/tmp/rstats-load") == 0) load_new();
				else save(0);
				gothup = 0;
			}
			if (gotterm) {
				save(!nvram_match("rstats_sshut", "1"));
				exit(0);
			}
			if (gotuser == 1) {
				save_speedjs(z - get_uptime());
				gotuser = 0;
			}
			else if (gotuser == 2) {
				save_histjs();
				gotuser = 0;
			}
			uptime = get_uptime();
		}
		calc();
		z += INTERVAL;
	}

	return 0;
}
