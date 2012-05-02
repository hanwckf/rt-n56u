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
#ifndef _TC_CORE_H_
#define _TC_CORE_H_ 1

#include <asm/types.h>
#include <linux/pkt_sched.h>

#define TIME_UNITS_PER_SEC	1000000

int  tc_core_time2big(unsigned time);
unsigned tc_core_time2tick(unsigned time);
unsigned tc_core_tick2time(unsigned tick);
unsigned tc_core_time2ktime(unsigned time);
unsigned tc_core_ktime2time(unsigned ktime);
unsigned tc_calc_xmittime(unsigned rate, unsigned size);
unsigned tc_calc_xmitsize(unsigned rate, unsigned ticks);
int tc_calc_rtable(struct tc_ratespec *r, __u32 *rtab, int cell_log, unsigned mtu);

int tc_setup_estimator(unsigned A, unsigned time_const, struct tc_estimator *est);

int tc_core_init(void);

extern struct rtnl_handle g_rth;
extern int is_batch_mode;

#endif
