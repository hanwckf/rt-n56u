/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>


/* TODO: make this threadsafe with a reentrant version of strsignal? */

void psignal(int signum, register const char *message)
{
	/* If the program is calling psignal, it's a safe bet that printf and
	 * friends are used as well.  It is also possible that the calling
	 * program could buffer stderr, or reassign it. */

	register const char *sep;

	sep = ": ";
	if (!(message && *message)) { /* Caller did not supply a prefix message */
		message = (sep += 2);	/* or passed an empty string. */
	}

	fprintf(stderr, "%s%s%s\n", message, sep, strsignal(signum));
}
