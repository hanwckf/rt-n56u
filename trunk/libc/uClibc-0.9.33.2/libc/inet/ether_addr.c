/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.
*/

/*
 *  2002-12-24  Nick Fedchik <nick@fedchik.org.ua>
 * 	- initial uClibc port
 */

#define __FORCE_GLIBC
#include <features.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>

struct ether_addr *ether_aton_r(const char *asc, struct ether_addr *addr)
{
	/* asc is "X:XX:XX:x:xx:xX" */
	int cnt;

	for (cnt = 0; cnt < 6; ++cnt) {
		unsigned char number;
		char ch = *asc++;

		if (ch < 0x20)
			return NULL;
		/* | 0x20 is cheap tolower(), valid for letters/numbers only */
		ch |= 0x20;
		if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
			return NULL;
		number = !(ch > '9') ? (ch - '0') : (ch - 'a' + 10);

		ch = *asc++;
		if ((cnt != 5 && ch != ':') /* not last group */
		/* What standard says ASCII ether address representation
		 * may also finish with whitespace, not only NUL?
		 * We can get rid of isspace() otherwise */
		 || (cnt == 5 && ch != '\0' /*&& !isspace(ch)*/)
		) {
			ch |= 0x20; /* cheap tolower() */
			if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
				return NULL;
			number = (number << 4) + (!(ch > '9') ? (ch - '0') : (ch - 'a' + 10));

			if (cnt != 5) {
				ch = *asc++;
				if (ch != ':')
					return NULL;
			}
		}

		/* Store result.  */
		addr->ether_addr_octet[cnt] = number;
	}
	/* Looks like we allow garbage after last group?
	 * "1:2:3:4:5:66anything_at_all"? */

	return addr;
}
libc_hidden_def(ether_aton_r)

struct ether_addr *ether_aton(const char *asc)
{
	static struct ether_addr result;

	return ether_aton_r(asc, &result);
}

char *ether_ntoa_r(const struct ether_addr *addr, char *buf)
{
	sprintf(buf, "%x:%x:%x:%x:%x:%x",
			addr->ether_addr_octet[0], addr->ether_addr_octet[1],
			addr->ether_addr_octet[2], addr->ether_addr_octet[3],
			addr->ether_addr_octet[4], addr->ether_addr_octet[5]);
	return buf;
}
libc_hidden_def(ether_ntoa_r)

char *ether_ntoa(const struct ether_addr *addr)
{
	static char asc[18];

	return ether_ntoa_r(addr, asc);
}
