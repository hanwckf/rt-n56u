/* setjmp for CRIS.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <sysdep.h>
#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>

        .syntax no_register_prefix

ENTRY (__sigsetjmp)
.Local__sigsetjmp:      
        moveq 1,r9
        movem sp,[r10+1*4]
#ifdef __PIC__
        move.d pc,r9
        addq 0f-.,r9
#else
        move.d 0f,r9
#endif
        move.d r9,[r10]
        move srp,[r10+16*4]
        PLTJUMP (__sigjmp_save)
0: /* This is where longjmp returns.  (Don't use "ret" - it's a macro.  */
        Ret
        move.d r9,r10
END (__sigsetjmp)
	
/* Binary compatibility entry points.  Having these in separate files
   is not meaningful and just adds library overhead.  */

ENTRY (__setjmp)
        ba .Local__sigsetjmp
        moveq 0,r11
END (__setjmp)

ENTRY (_setjmp)
        ba .Local__sigsetjmp
        moveq 0,r11
END (_setjmp)

ENTRY (setjmp)
        ba .Local__sigsetjmp
        moveq 1,r11
END (setjmp)

/*
weak_extern (__setjmp)
weak_extern (_setjmp)
weak_extern (setjmp)
*/









