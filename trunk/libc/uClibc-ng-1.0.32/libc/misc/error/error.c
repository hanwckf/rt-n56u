/* Error handler for noninteractive utilities
   Copyright (C) 1990-1998, 2000-2004, 2005 Free Software Foundation, Inc.
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

/* Written by David MacKenzie <djm@gnu.ai.mit.edu>.  */
/* Adjusted slightly by Erik Andersen <andersen@uclibc.org> */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>


/* This variable is incremented each time `error' is called.  */
unsigned int error_message_count = 0;
/* Sometimes we want to have at most one error per line.  This
   variable controls whether this mode is selected or not.  */
int error_one_per_line;
/* If NULL, error will flush stdout, then print on stderr the program
   name, a colon and a space.  Otherwise, error will call this
   function without parameters instead.  */
void (*error_print_progname) (void) = NULL;

void error (int status, int errnum, const char *message, ...)
{
    va_list args;

    fflush (stdout);

    if (error_print_progname)
	(*error_print_progname) ();
    else
	fprintf (stderr, "%s: ", __uclibc_progname);

    va_start (args, message);
    vfprintf (stderr, message, args);
    va_end (args);
    ++error_message_count;
    if (errnum) {
	fprintf (stderr, ": %s", strerror (errnum));
    }
    putc ('\n', stderr);
    if (status)
	exit (status);
}

void error_at_line (int status, int errnum, const char *file_name,
		    unsigned int line_number, const char *message, ...)
{
    va_list args;

    if (error_one_per_line) {
	static const char *old_file_name;
	static unsigned int old_line_number;

	if (old_line_number == line_number &&
		(file_name == old_file_name || !strcmp (old_file_name, file_name)))
	    /* Simply return and print nothing.  */
	    return;

	old_file_name = file_name;
	old_line_number = line_number;
    }

    fflush (stdout);

    if (error_print_progname)
	(*error_print_progname) ();
    else
	fprintf (stderr, "%s:", __uclibc_progname);

    if (file_name != NULL)
	fprintf (stderr, "%s:%d: ", file_name, line_number);

    va_start (args, message);
    vfprintf (stderr, message, args);
    va_end (args);

    ++error_message_count;
    if (errnum) {
	fprintf (stderr, ": %s", strerror (errnum));
    }
    putc ('\n', stderr);
    if (status)
	exit (status);
}
