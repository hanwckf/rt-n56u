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
#ifndef _TC_CBQ_H_
#define _TC_CBQ_H_ 1

unsigned tc_cbq_calc_maxidle(unsigned bndw, unsigned rate, unsigned avpkt,
			     int ewma_log, unsigned maxburst);
unsigned tc_cbq_calc_offtime(unsigned bndw, unsigned rate, unsigned avpkt,
			     int ewma_log, unsigned minburst);

#endif
