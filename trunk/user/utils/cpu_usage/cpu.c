/*****************************************************************************/

/*
 *	cpu.c -- simple CPU usage reporting tool.
 *
 *	(C) Copyright 2000, Greg Ungerer (gerg@snapgear.com)
 */

/*****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/time.h>

/*****************************************************************************/

char *version = "1.0.0";

/*****************************************************************************/

struct stats {
	unsigned int	user;    // user (application) usage
	unsigned int	nice;    // user usage with "niced" priority
	unsigned int	system;  // system (kernel) level usage
	unsigned int	idle;    // CPU idle and no disk I/O outstanding
	unsigned int	iowait;  // CPU idle but with outstanding disk I/O
	unsigned int	irq;     // Interrupt requests
	unsigned int	softirq; // Soft interrupt requests
	unsigned int	steal;   // Invol wait, hypervisor svcing other virtual CPU
	unsigned int	total;
};

/*****************************************************************************/

int getdata(FILE *fp, struct stats *st)
{
	if (fseek(fp, 0, SEEK_SET) < 0)
		return(-1);
	fscanf(fp, "cpu %u %u %u %u %u %u %u %u",
			&st->user, &st->nice, &st->system, &st->idle,
			&st->iowait, &st->irq, &st->softirq, &st->steal);

	st->total = st->user + st->nice  + st->system + st->idle + st->iowait +
				st->irq  + st->steal + st->softirq;
	return(0);
}

/*****************************************************************************/

static volatile int gotalarm;

static void alarm_handler(int i)
{
	        gotalarm = 1;
}

/*****************************************************************************/

void usage(FILE *fp, int rc)
{
	fprintf(fp, "Usage: cpu [-h?rai] [-s seconds] [-c count] "
		"[-d <device>]\n\n");
	fprintf(fp, "        -h?            this help\n");
	fprintf(fp, "        -v             print version info\n");
	fprintf(fp, "        -r             repeated output\n");
	fprintf(fp, "        -a             print system average\n");
	fprintf(fp, "        -i             idle measurement via busy loop\n");
	fprintf(fp, "        -c count       repeat count times\n");
	fprintf(fp, "        -s seconds     seconds between output\n");
	fprintf(fp, "        -d <device>    proc device to use (default /proc/stat)\n");
	exit(rc);
}

/*****************************************************************************/

int main(int argc, char *argv[])
{
	FILE		*fp;
	struct stats	st, stold;
	unsigned int	curtotal;
	int		c, cnt, repeat, delay, average, idle;
	char		*procdevice;
	struct timeval	start, stop;
	double 		idletotal = -1.00, idlediff;
	unsigned int	timediff, idlecount=0, idlepercent;
	struct sigaction sa;

	repeat = 0;
	delay = 1;
	procdevice = "/proc/stat";
	cnt = 1;
	average = 0;
	idle = 0;

	while ((c = getopt(argc, argv, "raivh?s:d:c:")) > 0) {
		switch (c) {
		case 'v':
			printf("%s: version %s\n", argv[0], version);
			exit(0);
		case 'a':
			average++;
			break;
		case 'i':
			idle++;
			break;
		case 'r':
			repeat++;
			break;
		case 's':
			delay = atoi(optarg);
			break;
		case 'd':
			procdevice = optarg;
			break;
		case 'c':
			cnt = atoi(optarg);
			break;
		case 'h':
		case '?':
			usage(stdout, 0);
			break;
		default:
			fprintf(stderr, "ERROR: unkown option '%c'\n", c);
			usage(stderr, 1);
			break;
		}
	}

	/*
	 *	Check device is real, and open it.
	 */
	if ((fp = fopen(procdevice, "r")) == NULL) {
		fprintf(stderr, "ERROR: failed to open %s, errno=%d\n",
			procdevice, errno);
		exit(0);
	}

	if (setvbuf(fp, NULL, _IONBF, 0) != 0) {
		fprintf(stderr, "ERROR: failed to setvbuf(%s), errno=%d\n",
			procdevice, errno);
		exit(0);
	}

	getdata(fp, &st);

	if (average) {
		printf("CPU:  average %d%%  (system=%d%% user=%d%% nice=%d%% "
			   "idle=%d%% iowait=%d%% irq=%d%% softirq=%d%% steal=%d%%)\n",
			(st.system + st.user  + st.nice + st.irq + 
			  st.steal + st.softirq)   * 100 / st.total,
			st.system  * 100 / st.total, st.user  * 100 / st.total,
			st.nice    * 100 / st.total, st.idle  * 100 / st.total,
			st.iowait  * 100 / st.total, st.irq   * 100 / st.total,
			st.softirq * 100 / st.total, st.steal * 100 / st.total);
		cnt = repeat = 0;
	}

	if (idle) {
		nice(19);
		memset(&sa, 0, sizeof(sa));
		sa.sa_flags = 0;
		sa.sa_handler = alarm_handler;
		sigaction(SIGALRM, &sa, NULL);
	} 


	for (c = 0; ((c < cnt) || repeat); c++) {
		if (idle) {
			gotalarm = 0;
			alarm(delay);
			gettimeofday(&start, NULL);
			for (idlecount=0; !gotalarm; idlecount++);
			gettimeofday(&stop, NULL);
		} else {
			sleep(delay);
		}
		stold = st;
		getdata(fp, &st);

		curtotal = st.total - stold.total;
		if (curtotal == 0) {
			/* Hmm, new stats same as old stats?? */
			continue;
		}

		if (idle) {
			timediff = (stop.tv_sec - start.tv_sec)*1000000
				+ stop.tv_usec - start.tv_usec;
			idlediff = (double)idlecount / (double)timediff;
			if (idlediff > idletotal)
				idletotal = idlediff;
			idlepercent = idlediff * 100.0 / idletotal;
			/* Now move ticks from st.nice to st.idle to account
			 * for our busy loop.  Don't recalculate idlepercent
			 * though to preserve accuracy. */
			st.nice = st.nice + st.idle - stold.idle
				- idlepercent * curtotal / 100;
			st.idle = stold.idle
				+ idlepercent * curtotal / 100;
			if (st.nice < stold.nice)
				st.nice = stold.nice;
		} else {
			idlepercent = (st.idle - stold.idle) * 100 / curtotal;
		}

		printf("CPU:  busy %d%%  (system=%d%% user=%d%% nice=%d%% "
		       "iowait=%d%% irq=%d%% softirq=%d%% steal=%d%% idle=%d%%)\n",
			((st.system + st.user + st.nice + st.irq + st.softirq +
			  st.steal) - (stold.system + stold.user + stold.nice +
				  stold.irq + stold.softirq + stold.steal)) *
			 100 / curtotal,
			(st.system  - stold.system ) * 100 / curtotal,
			(st.user    - stold.user   ) * 100 / curtotal,
			(st.nice    - stold.nice   ) * 100 / curtotal,
			(st.iowait  - stold.iowait ) * 100 / curtotal,
			(st.irq     - stold.irq    ) * 100 / curtotal,
			(st.softirq - stold.softirq) * 100 / curtotal,
			(st.steal   - stold.steal  ) * 100 / curtotal,
			idlepercent);
	}

	fclose(fp);
	exit(0);
}

/*****************************************************************************/
