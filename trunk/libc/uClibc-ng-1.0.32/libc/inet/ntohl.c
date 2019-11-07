/*
 * Functions to convert between host and network byte order.
 *
 * Copyright (C) 2003-2006 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <netinet/in.h>

#undef ntohl
#undef ntohs
#undef htonl
#undef htons

#if __BYTE_ORDER != __BIG_ENDIAN && __BYTE_ORDER != __LITTLE_ENDIAN
# error "You seem to have an unsupported byteorder"
#endif

uint32_t ntohl (uint32_t x)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	return x;
#else
	return __bswap_32(x);
#endif
}
libc_hidden_def(ntohl)
strong_alias(ntohl,htonl)
libc_hidden_def(htonl)

uint16_t ntohs (uint16_t x)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	return x;
#else
	return __bswap_16(x);
#endif
}
libc_hidden_def(ntohs)
strong_alias(ntohs,htons)
libc_hidden_def(htons)
