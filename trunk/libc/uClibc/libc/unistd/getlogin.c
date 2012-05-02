/* vi: set sw=4 ts=4: */
/* getlogin for uClibc
 *
 * Copyright (C) 2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 2000-2002 by Erik Andersen <andersen@uclibc.org>
 * Written by Erik Andersen <andersen@uclibc.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdlib.h>
#include <string.h>

/* uClibc makes it policy to not mess with the utmp file whenever
 * possible, since I consider utmp a complete waste of time.  Since
 * getlogin() should never be used for security purposes, we kindly let
 * the user specify whatever they want via the LOGNAME environment
 * variable, or we return NULL if getenv() fails to find anything */

char * getlogin(void)
{
	return (getenv("LOGNAME"));
}

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
