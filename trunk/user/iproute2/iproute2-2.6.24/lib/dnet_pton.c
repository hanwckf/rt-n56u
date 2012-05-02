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
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "utils.h"

static __inline__ u_int16_t dn_htons(u_int16_t addr)
{
        union {
                u_int8_t byte[2];
                u_int16_t word;
        } u;

        u.word = addr;
        return ((u_int16_t)u.byte[0]) | (((u_int16_t)u.byte[1]) << 8);
}


static int dnet_num(const char *src, u_int16_t * dst)
{
	int rv = 0;
	int tmp;
	*dst = 0;

	while ((tmp = *src++) != 0) {
		tmp -= '0';
		if ((tmp < 0) || (tmp > 9))
			return rv;

		rv++;
		(*dst) *= 10;
		(*dst) += tmp;
	}

	return rv;
}

static int dnet_pton1(const char *src, struct dn_naddr *dna)
{
	u_int16_t area = 0;
	u_int16_t node = 0;
	int pos;

	pos = dnet_num(src, &area);
	if ((pos == 0) || (area > 63) || (*(src + pos) != '.'))
		return 0;
	pos = dnet_num(src + pos + 1, &node);
	if ((pos == 0) || (node > 1023))
		return 0;
	dna->a_len = 2;
	*(u_int16_t *)dna->a_addr = dn_htons((area << 10) | node);

	return 1;
}

int dnet_pton(int af, const char *src, void *addr)
{
	int err;

	switch (af) {
	case AF_DECnet:
		errno = 0;
		err = dnet_pton1(src, (struct dn_naddr *)addr);
		break;
	default:
		errno = EAFNOSUPPORT;
		err = -1;
	}

	return err;
}
