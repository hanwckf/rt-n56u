/* Copyright (C) 1995,96,97,98,99,2000,2002 Free Software Foundation, Inc.
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


#define _ERRNO_H	1
#include <features.h>
#include <bits/errno.h>
#include <sys/syscall.h>

#if defined __UCLIBC_HAS_LFS__ && defined __NR_mmap2


#define LINKAGE     4
#define PTR_SIZE    4
#define SVRSP	16		/* saved register space */
#define PARMS	LINKAGE+SVRSP	/* space for 4 saved regs */
#define ADDR	PARMS
#define LEN	ADDR+PTR_SIZE
#define PROT	LEN+4
#define FLAGS	PROT+4
#define FD	FLAGS+4
#define OFFLO	FD+4
#define OFFHI	OFFLO+4

.text
.global mmap64
.type   mmap64,%function
.align 1<<4

mmap64:
	/* Save registers.  */
	pushl %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	movl OFFLO(%esp), %edx
	movl OFFHI(%esp), %ecx
	testl $0xfff, %edx
	jne L_einval
	shrdl $12, %ecx, %edx		/* mmap2 takes the offset in pages.  */
	shrl $12, %ecx
	jne L_einval
	movl %edx, %ebp

	movl ADDR(%esp), %ebx
	movl LEN(%esp), %ecx
	movl PROT(%esp), %edx
	movl FLAGS(%esp), %esi
	movl FD(%esp), %edi

	movl $__NR_mmap2, %eax	/* System call number in %eax.  */
	/* Do the system call trap.  */
	int $0x80

	/* Restore registers.  */
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp

	/* If 0 > %eax > -4096 there was an error.  */
	cmpl $-4095,%eax
	ja __syscall_error
	/* Successful; return the syscall's value.  */
	ret

	/* This means the offset value is too large.  */
L_einval:
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp
	movl $-EINVAL, %eax
	jmp __syscall_error

.size mmap64,.-mmap64

#endif
