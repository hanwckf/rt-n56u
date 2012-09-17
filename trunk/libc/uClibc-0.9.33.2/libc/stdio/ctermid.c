/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


char *ctermid(register char *s)
{
	static char sbuf[L_ctermid];

#ifdef __BCC__
	/* Currently elks doesn't support /dev/tty. */
	if (!s) {
		s = sbuf;
	}
	*s = 0;

	return s;
#else
	/* glibc always returns /dev/tty for linux. */
	return strcpy((s ? s : sbuf), "/dev/tty");
#endif
}
