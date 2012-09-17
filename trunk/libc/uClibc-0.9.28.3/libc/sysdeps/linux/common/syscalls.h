/* vi: set sw=4 ts=4: */
/*
 * Common header file for uClibc syscalls
 *
 * Copyright (C) 2001-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include <features.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <endian.h>

#undef __OPTIMIZE__
/* We absolutely do _NOT_ want interfaces silently
 *  *  * renamed under us or very bad things will happen... */
#ifdef __USE_FILE_OFFSET64
# undef __USE_FILE_OFFSET64
#endif

