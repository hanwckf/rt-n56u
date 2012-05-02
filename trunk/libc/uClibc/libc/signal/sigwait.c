/* vi: set sw=4 ts=4: */
/* sigwait
 *
 * Copyright (C) 2003 by Erik Andersen <andersen@uclibc.org>
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

#include <errno.h>
#include <signal.h>
#include <string.h>

int sigwait (const sigset_t *set, int *sig)
{
    int ret = 1;
    if ((ret = sigwaitinfo(set, NULL)) != -1) {
	*sig = ret;
	return 0;
    }
    return 1;
}
