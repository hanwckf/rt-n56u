/* Copyright (C) 2011-2018 Free Software Foundation, Inc.

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after it's all over.  */

#include <sysdep.h>
#define _ERRNO_H 1
#include <bits/errno.h>

#include <asm/unistd.h>
#include <arch/abi.h>
#include <linux/sched.h>

/* What we save where in the stack frame; must include all callee-saves. */
#define FRAME_NEXT_LR   (0 * REGSIZE)  /* reserved by ABI; not used here */
#define FRAME_SP        (1 * REGSIZE)
#define FRAME_R30       (2 * REGSIZE)
#define FRAME_R31       (3 * REGSIZE)
#define FRAME_R32       (4 * REGSIZE)
#define FRAME_SIZE      (5 * REGSIZE)

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
             pid_t *ptid, struct user_desc *tls, pid_t *ctid); */

	.text
ENTRY (__clone)
	/* Create a stack frame so we can pass callee-saves to new task. */
	{
	 move r10, sp
	 st sp, lr
	 ADDI_PTR sp, sp, -FRAME_SIZE
	}
	cfi_offset (lr, 0)
	cfi_def_cfa_offset (FRAME_SIZE)
	ADDI_PTR r11, sp, FRAME_SP
	{
	 st r11, r10
	 ADDI_PTR r11, sp, FRAME_R30
	}
	{
	 st r11, r30
	 ADDI_PTR r11, sp, FRAME_R31
	}
	cfi_offset (r30, FRAME_R30 - FRAME_SIZE)
	{
	 st r11, r31
	 ADDI_PTR r11, sp, FRAME_R32
	}
	cfi_offset (r31, FRAME_R31 - FRAME_SIZE)
	st r11, r32
	cfi_offset (r32, FRAME_R32 - FRAME_SIZE)

	/* sanity check arguments */
	beqz r0, .Linvalid
	beqz r1, .Linvalid

	/* Make sure child stack is properly aligned, and set up the
	   top frame so that we can call out of it immediately in the
	   child.  Setting it up here means we fault in the parent if
	   it's bogus, which is probably cleaner than faulting first
	   thing in the child. */
	ADDI_PTR r1, r1, -C_ABI_SAVE_AREA_SIZE
	andi r1, r1, -C_ABI_SAVE_AREA_SIZE
	ADDI_PTR r9, r1, REGSIZE /* sp of this frame on entry, i.e. zero */
	st r9, zero

	/* We need to switch the argument convention around from
	   libc to kernel:

	   libc:
	    r0 fn
	    r1 child_stack
	    r2 flags
	    r3 arg
	    r4 ptid
	    r5 tls
	    r6 ctid

	   kernel:
	    r0 flags
	    r1 child_stack [same as libc]
	    r2 ptid
	    r3 ctid
	    r4 tls

	   Plus the callee-saves as described at .Lthread_start, below.  */
	{
	 move r32, r0
	 move r0, r2
	}
	{
	 move r31, r3
	 move r3, r6
	}
	{
	 move r30, r2
	 move r2, r4
	}
	{
	 move r4, r5
	 moveli TREG_SYSCALL_NR_NAME, __NR_clone
	}
	swint1
	beqz r0, .Lthread_start  /* If in child task.  */

.Ldone:
	/* Restore the callee-saved registers and return. */
	ADDLI_PTR lr, sp, FRAME_SIZE
	{
	 ld lr, lr
	 ADDLI_PTR r30, sp, FRAME_R30
	}
	{
	 ld r30, r30
	 ADDLI_PTR r31, sp, FRAME_R31
	}
	{
	 ld r31, r31
	 ADDLI_PTR r32, sp, FRAME_R32
	}
	{
	 ld r32, r32
	 ADDI_PTR sp, sp, FRAME_SIZE
	}
	cfi_def_cfa_offset (0)

	bnez r1, .Lerror
	jrp lr

.Lerror:
	j SYSCALL_ERROR_NAME

.Linvalid:
	{
	 movei r1, EINVAL
	 j .Ldone
	}

/* This function expects to receive:

   sp: the top of a valid stack area
   r30: clone() flags
   r31: the argument to pass to the user function
   r32: the user function pointer  */

.Lthread_start:
	cfi_def_cfa_offset (FRAME_SIZE)
	cfi_undefined (lr)
	{
	 /* Invoke user function with specified argument. */
	 move r0, r31
	 jalr r32
	}
	moveli TREG_SYSCALL_NR_NAME, __NR_exit
	swint1
PSEUDO_END (__clone)

libc_hidden_def (clone)
weak_alias (__clone, clone)
