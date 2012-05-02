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
 * tc_cbq.c		CBQ maintanance routines.
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
#include "tc_cbq.h"

unsigned tc_cbq_calc_maxidle(unsigned bndw, unsigned rate, unsigned avpkt,
			     int ewma_log, unsigned maxburst)
{
	double maxidle;
	double g = 1.0 - 1.0/(1<<ewma_log);
	double xmt = (double)avpkt/bndw;

	maxidle = xmt*(1-g);
	if (bndw != rate && maxburst) {
		double vxmt = (double)avpkt/rate - xmt;
		vxmt *= (pow(g, -(double)maxburst) - 1);
		if (vxmt > maxidle)
			maxidle = vxmt;
	}
	return tc_core_time2tick(maxidle*(1<<ewma_log)*TIME_UNITS_PER_SEC);
}

unsigned tc_cbq_calc_offtime(unsigned bndw, unsigned rate, unsigned avpkt,
			     int ewma_log, unsigned minburst)
{
	double g = 1.0 - 1.0/(1<<ewma_log);
	double offtime = (double)avpkt/rate - (double)avpkt/bndw;

	if (minburst == 0)
		return 0;
	if (minburst == 1)
		offtime *= pow(g, -(double)minburst) - 1;
	else
		offtime *= 1 + (pow(g, -(double)(minburst-1)) - 1)/(1-g);
	return tc_core_time2tick(offtime*TIME_UNITS_PER_SEC);
}
