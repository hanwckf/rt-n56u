/* ARCH_FORK definition for Linux fork implementation.  m68k version.
   Copyright (C) 2010-2017 Free Software Foundation, Inc.
   Contributed by Maxim Kuvyrkov <maxim@codesourcery.com>, 2010.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sched.h>
#include <signal.h>
#include <sysdep.h>
#include <tls.h>

#define ARCH_FORK() \
  INLINE_SYSCALL (clone, 5,						      \
		  CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID | SIGCHLD, 0,     \
		  NULL, &THREAD_SELF->tid, NULL)

#include "../fork.c"
