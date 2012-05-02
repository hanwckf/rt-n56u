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
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "utils.h"

static u_int32_t hexget(char c)
{
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= '0' && c <= '9')
		return c - '0';

	return 0xf0;
}

static int ipx_getnet(u_int32_t *net, const char *str)
{
	int i;
	u_int32_t tmp;

	for(i = 0; *str && (i < 8); i++) {

		if ((tmp = hexget(*str)) & 0xf0) {
			if (*str == '.')
				return 0;
			else
				return -1;
		}

		str++;
		(*net) <<= 4;
		(*net) |= tmp;
	}

	if (*str == 0)
		return 0;

	return -1;
}

static int ipx_getnode(u_int8_t *node, const char *str)
{
	int i;
	u_int32_t tmp;

	for(i = 0; i < 6; i++) {
		if ((tmp = hexget(*str++)) & 0xf0)
			return -1;
		node[i] = (u_int8_t)tmp;
		node[i] <<= 4;
		if ((tmp = hexget(*str++)) & 0xf0)
			return -1;
		node[i] |= (u_int8_t)tmp;
		if (*str == ':')
			str++;
	}

	return 0;
}

static int ipx_pton1(const char *src, struct ipx_addr *addr)
{
	char *sep = (char *)src;
	int no_node = 0;

	memset(addr, 0, sizeof(struct ipx_addr));

	while(*sep && (*sep != '.'))
		sep++;

	if (*sep != '.')
		no_node = 1;

	if (ipx_getnet(&addr->ipx_net, src))
		return 0;

	addr->ipx_net = htonl(addr->ipx_net);

	if (no_node)
		return 1;

	if (ipx_getnode(addr->ipx_node, sep + 1))
		return 0;

	return 1;
}

int ipx_pton(int af, const char *src, void *addr)
{
	int err;

	switch (af) {
	case AF_IPX:
		errno = 0;
		err = ipx_pton1(src, (struct ipx_addr *)addr);
		break;
	default:
		errno = EAFNOSUPPORT;
		err = -1;
	}

	return err;
}
