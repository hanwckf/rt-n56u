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
#ifndef __LINUX_TC_MIR_H
#define __LINUX_TC_MIR_H

#include <linux/pkt_cls.h>

#define TCA_ACT_MIRRED 8
#define TCA_EGRESS_REDIR 1  /* packet redirect to EGRESS*/
#define TCA_EGRESS_MIRROR 2 /* mirror packet to EGRESS */
#define TCA_INGRESS_REDIR 3  /* packet redirect to INGRESS*/
#define TCA_INGRESS_MIRROR 4 /* mirror packet to INGRESS */
                                                                                
struct tc_mirred
{
	tc_gen;
	int                     eaction;   /* one of IN/EGRESS_MIRROR/REDIR */
	__u32                   ifindex;  /* ifindex of egress port */
};
                                                                                
enum
{
	TCA_MIRRED_UNSPEC,
	TCA_MIRRED_TM,
	TCA_MIRRED_PARMS,
	__TCA_MIRRED_MAX
};
#define TCA_MIRRED_MAX (__TCA_MIRRED_MAX - 1)
                                                                                
#endif
