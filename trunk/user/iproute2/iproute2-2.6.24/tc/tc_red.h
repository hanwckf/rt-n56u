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
#ifndef _TC_RED_H_
#define _TC_RED_H_ 1

extern int tc_red_eval_P(unsigned qmin, unsigned qmax, double prob);
extern int tc_red_eval_ewma(unsigned qmin, unsigned burst, unsigned avpkt);
extern int tc_red_eval_idle_damping(int wlog, unsigned avpkt, unsigned bandwidth, __u8 *sbuf);

#endif
