/* Copyright (C) 1991, 1995, 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

/* Definitions relevant to functions that operate on `sigset_t's.  */

#include <errno.h>
#define __USE_GNU
#include <signal.h>
#include <string.h>

#define	BITS		(_NSIG - 1)
#define	ELT(signo)	(((signo) - 1) / BITS)
#define	MASK(signo)	(1 << (((signo) - 1) % BITS))

#undef	sigemptyset
#undef	sigfillset
#undef	sigaddset
#undef	sigdelset
#undef	sigismember
