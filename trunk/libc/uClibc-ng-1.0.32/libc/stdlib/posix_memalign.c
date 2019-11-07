/* posix_memalign for uClibc
 *
 * Copyright (C) 1996-2002, 2003, 2004, 2005 Free Software Foundation, Inc.
 * Copyright (C) 2005 by Erik Andersen <andersen@uclibc.org>
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
 * along with this program; see the file COPYING.LIB.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/param.h>

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	/* Make sure alignment is correct. */
	if (alignment % sizeof(void *) != 0)
	    /* Skip these checks because the memalign() func does them for us
	     || !powerof2(alignment / sizeof(void *)) != 0
	     || alignment == 0
	     */
		return EINVAL;
	void *mem = memalign(alignment, size);
	if (mem != NULL) {
		*memptr = mem;
		return 0;
	} else
		return ENOMEM;
}
