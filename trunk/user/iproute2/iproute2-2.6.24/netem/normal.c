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
 * Normal distribution table generator
 * Taken from the uncopyrighted NISTnet code.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#include <linux/types.h>
#include <linux/pkt_sched.h>

#define TABLESIZE 16384
#define TABLEFACTOR NETEM_DIST_SCALE

static double
normal(double x, double mu, double sigma)
{
	return .5 + .5*erf((x-mu)/(sqrt(2.0)*sigma));
}


int
main(int argc, char **argv)
{
	int i, n;
	double x;
	double table[TABLESIZE+1];

	for (x = -10.0; x < 10.05; x += .00005) {
		i = rint(TABLESIZE * normal(x, 0.0, 1.0));
		table[i] = x;
	}

	
	printf("# This is the distribution table for the normal distribution.\n");
	for (i = n = 0; i < TABLESIZE; i += 4) {
		int value = (int) rint(table[i]*TABLEFACTOR);
		if (value < SHRT_MIN) value = SHRT_MIN;
		if (value > SHRT_MAX) value = SHRT_MAX;

		printf(" %d", value);
		if (++n == 8) {
			putchar('\n');
			n = 0;
		}
	}

	return 0;
}
