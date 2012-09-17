/* vi: set sw=4 ts=4: */
/* strtof for uClibc
 *
 * Copyright (C) 2002 by Erik Andersen <andersen@uclibc.org>
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
 *
 */

/* This implementation is a bit lame, since we are actually making the compiler
 * to an internal conversion from a double to a float, thereby wasting a bunch
 * of precision.  But this is small, and works for now... */

#include <stdlib.h>

float strtof (const char *str, char **endptr)
{
    return(strtod(str,endptr));
}

