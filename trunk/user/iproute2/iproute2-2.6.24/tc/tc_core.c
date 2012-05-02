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
/*
 * tc_core.c		TC core library.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "tc_core.h"

static double tick_in_usec = 1;
static double clock_factor = 1;

int tc_core_time2big(unsigned time)
{
	__u64 t = time;

	t *= tick_in_usec;
	return (t >> 32) != 0;
}


unsigned tc_core_time2tick(unsigned time)
{
	return time*tick_in_usec;
}

unsigned tc_core_tick2time(unsigned tick)
{
	return tick/tick_in_usec;
}

unsigned tc_core_time2ktime(unsigned time)
{
	return time * clock_factor;
}

unsigned tc_core_ktime2time(unsigned ktime)
{
	return ktime / clock_factor;
}

unsigned tc_calc_xmittime(unsigned rate, unsigned size)
{
	return tc_core_time2tick(TIME_UNITS_PER_SEC*((double)size/rate));
}

unsigned tc_calc_xmitsize(unsigned rate, unsigned ticks)
{
	return ((double)rate*tc_core_tick2time(ticks))/TIME_UNITS_PER_SEC;
}

/*
   rtab[pkt_len>>cell_log] = pkt_xmit_time
 */

int tc_calc_rtable(struct tc_ratespec *r, __u32 *rtab, int cell_log, unsigned mtu)
{
	int i;
	unsigned bps = r->rate;
	unsigned mpu = r->mpu;

	if (mtu == 0)
		mtu = 2047;

	if (cell_log < 0) {
		cell_log = 0;
		while ((mtu>>cell_log) > 255)
			cell_log++;
	}
	for (i=0; i<256; i++) {
		unsigned sz = ((i+1)<<cell_log);
		if (sz < mpu)
			sz = mpu;
		rtab[i] = tc_calc_xmittime(bps, sz);
	}
	r->cell_align=-1; // Due to the sz calc
	r->cell_log=cell_log;
	return cell_log;
}

int tc_core_init()
{
	FILE *fp;
	__u32 clock_res;
	__u32 t2us;
	__u32 us2t;

	fp = fopen("/proc/net/psched", "r");
	if (fp == NULL)
		return -1;

	if (fscanf(fp, "%08x%08x%08x", &t2us, &us2t, &clock_res) != 3) {
		fclose(fp);
		return -1;
	}
	fclose(fp);

	/* compatibility hack: for old iproute binaries (ignoring
	 * the kernel clock resolution) the kernel advertises a
	 * tick multiplier of 1000 in case of nano-second resolution,
	 * which really is 1. */
	if (clock_res == 1000000000)
		t2us = us2t;

	clock_factor  = (double)clock_res / TIME_UNITS_PER_SEC;
	tick_in_usec = (double)t2us / us2t * clock_factor;
	return 0;
}
