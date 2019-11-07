/*
 * getlogin for uClibc
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


/* uClibc makes it policy to not mess with the utmp file whenever
 * possible, since I consider utmp a complete waste of time.  Since
 * getlogin() should never be used for security purposes, we kindly let
 * the user specify whatever they want via the LOGNAME environment
 * variable, or we return NULL if getenv() fails to find anything */

char * getlogin(void)
{
	return (getenv("LOGNAME"));
}
libc_hidden_def(getlogin)

int getlogin_r(char *name, size_t len)
{
	char * foo = getenv("LOGNAME");

	if (! foo)
		return -1;

	strncpy(name, foo, len);
	name[len-1] = '\0';
	return 0;
}

char *cuserid(char *s)
{
	char *name = getlogin();
	if (s) {
		return(strcpy(s, name ? name : ""));
	}
	return name;
}
