/* vi: set sw=4 ts=4: */
/*
 * exit syscall for uClibc
 *
 * Copyright (C) 2002 by Erik Andersen <andersen@codepoet.org>
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

#define _GNU_SOURCE
#include <features.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>

#ifndef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...) __syscall_exit (args)
#undef __NR___syscall_exit
#define __NR___syscall_exit __NR_exit
static inline _syscall1(void, __syscall_exit, int, status);
#endif

void _exit(int status)
{
	/* The loop is added only to keep gcc happy. */
	while(1)
		INLINE_SYSCALL(exit, 1, status);
}
weak_alias(_exit,_Exit)
