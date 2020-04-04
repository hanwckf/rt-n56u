/* Functions for storing Ethernet addresses in ASCII and mapping to hostnames.
   Copyright (C) 1996, 1997, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _NETINET_ETHER_H
#define _NETINET_ETHER_H	1

#include <features.h>

/* Get definition of `struct ether_addr'.  */
#include <netinet/if_ether.h>

#ifdef _LIBC
#define ETHER_FILE_NAME "/etc/ethers"
#endif

__BEGIN_DECLS

#if defined __UCLIBC_HAS_SOCKET__ || defined __UCLIBC_HAS_IPV4__ || \
	defined __UCLIBC_HAS_IPV6__
/* Convert 48 bit Ethernet ADDRess to ASCII.  */
extern char *ether_ntoa (const struct ether_addr *__addr) __THROW;
extern char *ether_ntoa_r (const struct ether_addr *__addr, char *__buf)
     __THROW;
libc_hidden_proto(ether_ntoa_r)

/* Convert ASCII string S to 48 bit Ethernet address.  */
extern struct ether_addr *ether_aton (const char *__asc) __THROW;
extern struct ether_addr *ether_aton_r (const char *__asc,
					struct ether_addr *__addr) __THROW;
libc_hidden_proto(ether_aton_r)

/* Map 48 bit Ethernet number ADDR to HOSTNAME.  */
extern int ether_ntohost (char *__hostname, const struct ether_addr *__addr)
     __THROW;

/* Map HOSTNAME to 48 bit Ethernet address.  */
extern int ether_hostton (const char *__hostname, struct ether_addr *__addr)
     __THROW;

/* Scan LINE and set ADDR and HOSTNAME.  */
extern int ether_line (const char *__line, struct ether_addr *__addr,
		       char *__hostname) __THROW;
#endif

__END_DECLS

#endif /* netinet/ether.h */
