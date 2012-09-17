/*  Copyright (C) 2002     Manuel Novoa III
 *  An __assert() function compatible with the modified glibc assert.h
 *  that is used by uClibc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Oct 28, 2002
 *
 * ANSI/ISO C99 requires assert() to write to stderr.  This means that
 * writing to STDERR_FILENO is insufficient, as the user could freopen
 * stderr.  It is also insufficient to output to fileno(stderr) since
 * this would fail in the custom stream case.  I didn't remove the
 * old code though, as it doesn't use stdio stream functionality
 * and is useful in debugging the stdio code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Get the prototype from assert.h as a double-check. */
#undef NDEBUG
#include <assert.h>
#undef assert


#define ASSERT_SHOW_PROGNAME 1

static smallint in_assert;			/* bss inits to 0. */

void __assert(const char *assertion, const char * filename,
	      unsigned int linenumber, register const char * function)
{
	if (!in_assert) {
		in_assert = 1;

		fprintf(stderr,
#ifdef ASSERT_SHOW_PROGNAME
				"%s: %s: %d: %s: Assertion `%s' failed.\n", __uclibc_progname,
#else
				"%s: %d: %s: Assertion `%s' failed.\n",
#endif
				filename,
				linenumber,
				/* Function name isn't available with some compilers. */
				((function == NULL) ? "?function?" : function),
				assertion
				);
	}
	/* shouldn't we? fflush(stderr); */
	abort();
}
libc_hidden_def(__assert)
