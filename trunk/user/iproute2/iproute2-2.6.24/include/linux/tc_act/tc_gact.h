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
#ifndef __LINUX_TC_GACT_H
#define __LINUX_TC_GACT_H

#include <linux/pkt_cls.h>

#define TCA_ACT_GACT 5
struct tc_gact
{
	tc_gen;

};

struct tc_gact_p
{
#define PGACT_NONE              0
#define PGACT_NETRAND           1
#define PGACT_DETERM            2
#define MAX_RAND                (PGACT_DETERM + 1 )
	__u16                 ptype;
	__u16                 pval;
	int                   paction;
};
 
enum
{
	TCA_GACT_UNSPEC,
	TCA_GACT_TM,
	TCA_GACT_PARMS,
	TCA_GACT_PROB,
	__TCA_GACT_MAX
};
#define TCA_GACT_MAX (__TCA_GACT_MAX - 1)
 
#endif
