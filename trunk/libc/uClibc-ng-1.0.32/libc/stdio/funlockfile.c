/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

void funlockfile(FILE *stream)
{
	__STDIO_STREAM_VALIDATE(stream);

	__STDIO_ALWAYS_THREADUNLOCK_CANCEL_UNSAFE(stream);
}
