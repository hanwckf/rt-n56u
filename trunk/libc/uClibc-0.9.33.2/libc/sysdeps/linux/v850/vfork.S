/*
 * libc/sysdeps/linux/v850/vfork.S -- `vfork' syscall for linux/v850
 *
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#include <features.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#define _SYSCALL_H
#include <bits/sysnum.h>

#include <clinkage.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

.global C_SYMBOL_NAME(errno)

C_ENTRY (__vfork):
	addi	SYS_vfork, r0, r12
	trap	0
	addi	-125, r0, r11		/* minimum err value */
	cmp	r11, r10
	bh	1f
	jmp	[lp]			/* normal return */
1:	mov	hilo(C_SYMBOL_NAME(errno)), r11
	subr	r0, r10
	st.w	r10, 0[r11]
	jmp	[lp]			/* error return */
C_END(__vfork)
weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
