/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <stdio_ext.h>


/* Solaris function --
 * Flush all line buffered (writing) streams.
 */

void _flushlbf(void)
{
	__STDIO_FLUSH_LBF_STREAMS;
}
