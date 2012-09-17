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
#ifndef _IPT_DSTLIMIT_H
#define _IPT_DSTLIMIT_H

/* timings are in milliseconds. */
#define IPT_DSTLIMIT_SCALE 10000
/* 1/10,000 sec period => max of 10,000/sec.  Min rate is then 429490
   seconds, or one every 59 hours. */

/* details of this structure hidden by the implementation */
struct ipt_dstlimit_htable;

#define IPT_DSTLIMIT_HASH_DIP	0x0001
#define IPT_DSTLIMIT_HASH_DPT	0x0002
#define IPT_DSTLIMIT_HASH_SIP	0x0004

struct dstlimit_cfg {
	u_int32_t mode;	  /* bitmask of IPT_DSTLIMIT_HASH_* */
	u_int32_t avg;    /* Average secs between packets * scale */
	u_int32_t burst;  /* Period multiplier for upper limit. */

	/* user specified */
	u_int32_t size;		/* how many buckets */
	u_int32_t max;		/* max number of entries */
	u_int32_t gc_interval;	/* gc interval */
	u_int32_t expire;	/* when do entries expire? */
};

struct ipt_dstlimit_info {
	char name [IFNAMSIZ];		/* name */
	struct dstlimit_cfg cfg;
	struct ipt_dstlimit_htable *hinfo;

	/* Used internally by the kernel */
	union {
		void *ptr;
		struct ipt_dstlimit_info *master;
	} u;
};
#endif /*_IPT_DSTLIMIT_H*/
