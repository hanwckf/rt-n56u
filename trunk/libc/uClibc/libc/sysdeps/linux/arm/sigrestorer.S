/* Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sys/syscall.h>

/* If no SA_RESTORER function was specified by the application we use
   one of these.  This avoids the need for the kernel to synthesise a return
   instruction on the stack, which would involve expensive cache flushes. */

.global __default_sa_restorer
.type __default_sa_restorer,%function
.align 4
__default_sa_restorer:
	swi	__NR_sigreturn


#ifdef __NR_rt_sigreturn

.global __default_rt_sa_restorer
.type __default_rt_sa_restorer,%function
.align 4
__default_rt_sa_restorer:
	swi	__NR_rt_sigreturn

#endif
