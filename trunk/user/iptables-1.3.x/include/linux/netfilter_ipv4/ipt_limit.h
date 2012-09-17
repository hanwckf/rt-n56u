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
#ifndef _IPT_RATE_H
#define _IPT_RATE_H

/* timings are in milliseconds. */
#define IPT_LIMIT_SCALE 10000

/* 1/10,000 sec period => max of 10,000/sec.  Min rate is then 429490
   seconds, or one every 59 hours. */
struct ipt_rateinfo {
	u_int32_t avg;    /* Average secs between packets * scale */
	u_int32_t burst;  /* Period multiplier for upper limit. */

#ifdef KERNEL_64_USERSPACE_32
	u_int64_t prev;
	u_int64_t placeholder;
#else
	/* Used internally by the kernel */
	unsigned long prev;
	/* Ugly, ugly fucker. */
	struct ipt_rateinfo *master;
#endif

	u_int32_t credit;
	u_int32_t credit_cap, cost;
};
#endif /*_IPT_RATE_H*/
