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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <bits/arm_asm.h>
#include <sys/syscall.h>
#include <linux/version.h>

/* If no SA_RESTORER function was specified by the application we use
   one of these.  This avoids the need for the kernel to synthesise a return
   instruction on the stack, which would involve expensive cache flushes.

   Nowadays (2.6 series, and somewhat earlier) the kernel uses a high page
   for signal trampolines, so the cache flushes are not an issue.  But since
   we do not have a vDSO, continue to use these so that we can provide
   unwind information.

   Start the unwind tables at least one instruction before the signal
   trampoline, because the unwinder will assume we are returning after
   a call site.

   The signal frame layout changed in 2.6.18.  */

.global __default_sa_restorer
.type __default_sa_restorer,%function
.align 2
#ifdef __ARM_EABI__
#ifdef __thumb__
.thumb_func
#endif
	.fnstart
	.save {r0-r15}
#if LINUX_VERSION_CODE >= 0x020612
	.pad #32
#else
	.pad #12
#endif
	nop
__default_sa_restorer:
	mov	r7, $SYS_ify(sigreturn)
	swi	0x0
	.fnend
#else
__default_sa_restorer:
	DO_CALL (sigreturn)
#endif


#ifdef __NR_rt_sigreturn

.global __default_rt_sa_restorer
.type __default_rt_sa_restorer,%function
.align 2
#ifdef __ARM_EABI__
#ifdef __thumb__
.thumb_func
#endif
	.fnstart
	.save {r0-r15}
#if LINUX_VERSION_CODE >= 0x020612
	.pad #160
#else
	.pad #168
#endif
	nop
__default_rt_sa_restorer:
	mov	r7, $SYS_ify(rt_sigreturn)
	swi	0x0
	.fnend
#else
__default_rt_sa_restorer:
	DO_CALL (rt_sigreturn)
#endif

#endif
