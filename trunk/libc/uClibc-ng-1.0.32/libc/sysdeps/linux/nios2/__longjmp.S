/* longjmp for Nios II.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <jmpbuf-offsets.h>

ENTRY (__longjmp)
    mov     r2,  r5

    ldw	    r16, (JB_R16*4)(r4)
    ldw	    r17, (JB_R17*4)(r4)
    ldw	    r18, (JB_R18*4)(r4)
    ldw	    r19, (JB_R19*4)(r4)
    ldw	    r20, (JB_R20*4)(r4)
    ldw     r21, (JB_R21*4)(r4)
    ldw	    r22, (JB_R22*4)(r4)
    ldw	    fp,  (JB_FP*4)(r4)
    ldw	    ra,  (JB_RA*4)(r4)
    ldw	    sp,  (JB_SP*4)(r4)
  
    ret

END (__longjmp)
libc_hidden_def(__longjmp)
