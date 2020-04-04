!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.


	.text
	.global	___longjmp
	.type	___longjmp,function

___longjmp:
        !! If val is 0, set it to 1
	CMP 	D0Ar2,#0
	ADDZ	D0Ar2,D0Ar2,#1

        !! Restore A0/A1 regs
        MGETL   A0.0,A0.1,[D1Ar1++]
        !! Restore D0/D1 regs
	MOV 	A0.3,D1Ar1
        MGETL   D0FrT,D0.5,D0.6,D0.7,[A0.3++]
        !! Move 2nd argument to return value
        MOV     D0Re0,D0Ar2
        MOV     PC,D1RtP
        .size   ___longjmp,.-___longjmp

libc_hidden_def(__longjmp)
