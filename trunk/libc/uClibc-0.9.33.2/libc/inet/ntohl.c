/* vi: set sw=4 ts=4:
 * Functions to convert between host and network byte order.
 *
 * Copyright (C) 2003-2006 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdint.h>
#include <endian.h>
#include <byteswap.h>
#include <netinet/in.h>

#undef ntohl
#undef ntohs
#undef htonl
#undef htons

#if __BYTE_ORDER == __BIG_ENDIAN
uint32_t ntohl (uint32_t x)
{
	return x;
}

uint16_t ntohs (uint16_t x)
{
	return x;
}

uint32_t htonl (uint32_t x)
{
	return x;
}

uint16_t htons (uint16_t x)
{
	return x;
}
#elif __BYTE_ORDER == __LITTLE_ENDIAN
uint32_t ntohl (uint32_t x)
{
	return __bswap_32(x);
}

uint16_t ntohs (uint16_t x)
{
	return __bswap_16(x);
}

uint32_t htonl (uint32_t x)
{
	return __bswap_32(x);
}

uint16_t htons (uint16_t x)
{
	return __bswap_16(x);
}
#else
#error "You seem to have an unsupported byteorder"
#endif

libc_hidden_def(ntohl)
libc_hidden_def(ntohs)
libc_hidden_def(htonl)
libc_hidden_def(htons)
