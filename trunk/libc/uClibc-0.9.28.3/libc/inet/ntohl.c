/* vi: set sw=4 ts=4:
 * Functions to convert between host and network byte order.
 *
 * Copyright (C) 2003 by Erik Andersen <andersen@uclibc.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdint.h>
#include <endian.h>
#include <byteswap.h>

#if __BYTE_ORDER == __BIG_ENDIAN
extern uint32_t ntohl (uint32_t x)
{
	return x;
}

extern uint16_t ntohs (uint16_t x)
{
	return x;
}

extern uint32_t htonl (uint32_t x)
{
	return x;
}

extern uint16_t htons (uint16_t x)
{
	return x;
}
#elif __BYTE_ORDER == __LITTLE_ENDIAN
extern uint32_t ntohl (uint32_t x)
{
	return __bswap_32(x);
}

extern uint16_t ntohs (uint16_t x)
{
	return __bswap_16(x);
}

extern uint32_t htonl (uint32_t x)
{
	return __bswap_32(x);
}

extern uint16_t htons (uint16_t x)
{
	return __bswap_16(x);
}
#else
#error "You seem to have an unsupported byteorder"
#endif
