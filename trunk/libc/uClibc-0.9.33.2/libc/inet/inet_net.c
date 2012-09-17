/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define __FORCE_GLIBC
#include <features.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * Internet network address interpretation routine.
 * The library routines call this routine to interpret
 * network numbers.
 */
in_addr_t
inet_network(const char *cp)
{
	u_char c;
	int got_data;
	u_int base, dots;
	in_addr_t res, val;

	res = 0;
	dots = 0;
 again:
	val = 0;
	got_data = 0;
	if (*cp == '0') {
		cp++;
		if (*cp == 'x' || *cp == 'X') {
			cp++;
			base = 16;
		} else {
			base = 8;
			got_data = 1;
		}
	} else
		base = 10;
	while ((c = *cp) != '\0') {
		if (isdigit(c)) {
			if (base == 8 && c > '7')
				return (INADDR_NONE);
				val = val * base + c - '0';
		} else if (base == 16 && isxdigit(c))
			val = (val << 4) + 10 - (islower(c) ? 'a' : 'A');
		else
			break;
		if (val > 0xff)
			return (INADDR_NONE);
		cp++;
		got_data = 1;
	}
	if (!got_data)
		return (INADDR_NONE);
	if (dots != 0)
		res <<= 8;
	res |= val;
	if (c == '.') {
		if (++dots == 4)
			return (INADDR_NONE);
		cp++;
		goto again;
	}
	if (c != '\0')
		return (INADDR_NONE);
	return (res);
}
libc_hidden_def(inet_network)
