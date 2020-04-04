/* Copyright (C) 2003, 2005, 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

/* It's vitally important that _Unwind_Resume not have a stack frame; the
   ARM unwinder relies on register state at entrance.  So we write this in
   assembly.  */

#include <sysdep.h>

__asm__ (
"       .globl  _Unwind_Resume\n"
"	.hidden	_Unwind_Resume\n"
"       .type   _Unwind_Resume, %function\n"
"_Unwind_Resume:\n"
"       " CFI_SECTIONS (.debug_frame) "\n"
"       " CFI_STARTPROC "\n"
"       stmfd   sp!, {r4, r5, r6, lr}\n"
"       " CFI_ADJUST_CFA_OFFSET (16)" \n"
"       " CFI_REL_OFFSET (r4, 0) "\n"
"       " CFI_REL_OFFSET (r5, 4) "\n"
"       " CFI_REL_OFFSET (r6, 8) "\n"
"       " CFI_REL_OFFSET (lr, 12) "\n"
"       " CFI_REMEMBER_STATE "\n"
"       ldr     r4, 1f\n"
"       ldr     r5, 2f\n"
"3:     add     r4, pc, r4\n"
"       ldr     r3, [r4, r5]\n"
"       mov     r6, r0\n"
"       cmp     r3, #0\n"
"       beq     4f\n"
"5:     mov     r0, r6\n"
"       ldmfd   sp!, {r4, r5, r6, lr}\n"
"       " CFI_ADJUST_CFA_OFFSET (-16) "\n"
"       " CFI_RESTORE (r4) "\n"
"       " CFI_RESTORE (r5) "\n"
"       " CFI_RESTORE (r6) "\n"
"       " CFI_RESTORE (lr) "\n"
"       bx      r3\n"
"       " CFI_RESTORE_STATE "\n"
"4:     bl      __libgcc_s_init\n"
"       ldr     r3, [r4, r5]\n"
"       b       5b\n"
"       " CFI_ENDPROC "\n"
"       .align 2\n"
#ifdef __thumb2__
"1:     .word   _GLOBAL_OFFSET_TABLE_ - 3b - 4\n"
#else
"1:     .word   _GLOBAL_OFFSET_TABLE_ - 3b - 8\n"
#endif
"2:     .word   __libgcc_s_resume(GOTOFF)\n"
"       .size   _Unwind_Resume, .-_Unwind_Resume\n"
);
