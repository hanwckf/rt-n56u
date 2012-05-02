/* Error handler for noninteractive utilities
   Copyright (C) 1990-1998, 2000, 2001 Free Software Foundation, Inc.

   This file is part of the GNU C Library.  Its master source is NOT part of
   the C library, however.  The master source lives in /gd/gnu/lib.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* Written by David MacKenzie <djm@gnu.ai.mit.edu>.  */
/* Adjusted slightly by Erik Andersen <andersen@uclibc.org> */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"


/* This variable is incremented each time `error' is called.  */
unsigned int error_message_count;
/* Sometimes we want to have at most one error per line.  This
   variable controls whether this mode is selected or not.  */
int error_one_per_line;
/* If NULL, error will flush stdout, then print on stderr the program
   name, a colon and a space.  Otherwise, error will call this
   function without parameters instead.  */
void (*error_print_progname) (void) = NULL;


void __error (int status, int errnum, const char *message, ...)
{
    va_list args;

    fflush (stdout);

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

void __error_at_line (int status, int errnum, const char *file_name,
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

/* Use the weaks here in an effort at controlling namespace pollution */
#undef error
#undef error_at_line
weak_alias (__error, error)
weak_alias (__error_at_line, error_at_line)
