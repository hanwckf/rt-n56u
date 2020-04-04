!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.


#include <features.h>

	.text
	.global	_strcmp
	.type	_strcmp,function
!D1Ar1 s1
!D0Ar2 s2
_strcmp:
	TST	D1Ar1,#3
	TSTZ	D0Ar2,#3
	MOVT	D1Re0,#0x0101
	ADD	D1Re0,D1Re0,#0x0101
	BNZ	$Lstrcmp_slow
	GETD	D1Ar3,[D1Ar1+#4++]	! Load 32-bits from s1
	GETD	D1Ar5,[D0Ar2+#4++]      ! Load 32-bits from s2
	LSL	D0FrT,D1Re0,#7		! D0FrT = 0x80808080
$Lstrcmp4_loop:
	SUB	D0Re0,D1Ar3,D1Re0	! D1Re0 = 0x01010101
	MOV	D0Ar6,D1Ar3
	SUBS	D0Ar4,D1Ar3,D1Ar5	! Calculate difference
	XOR	D0Ar6,D0Ar6,#-1
	GETD	D1Ar3,[D1Ar1+#4++]	! Load 32-bits from s1
	AND	D0Re0,D0Re0,D0Ar6
	ANDSZ	D0Ar6,D0Re0,D0FrT	! D0FrT = 0x80808080
	GETD	D1Ar5,[D0Ar2+#4++]      ! Load 32-bits from s2
	BZ	$Lstrcmp4_loop
	AND	D0Ar6, D0Re0, D0FrT	! D0FrT = 0x80808080
!
! Either they are different or they both contain a NULL + junk
!
$Lstrcmp4_end:
	LSLS	D0Re0,D0Ar4,#24		! Was Byte[0] the same?
	LSLSZ	D0Ar2,D0Ar6,#24		! Yes: AND they where not zero?
	LSLSZ	D0Re0,D0Ar4,#16		! Yes: Was Byte[1] the same?
	LSLSZ	D0Ar2,D0Ar6,#16		! Yes: AND they where not zero?
	LSLSZ	D0Re0,D0Ar4,#8		! Tes: Was Byte[2] the same?
	LSLSZ	D0Ar2,D0Ar6,#8		! Yes: AND they where not zero?
	MOVZ	D0Re0,D0Ar4		! Yes: Must by Byte[3] thats the result
	ASR	D0Re0,D0Re0,#24		! Sign extend result to integer
	MOV	PC,D1RtP
!
! Misaligned case, byte at a time
!
$Lstrcmp_slow:
	GETB	D1Ar3,[D1Ar1++]		! Load char from s1
	GETB	D1Ar5,[D0Ar2++]         ! Load char from s2
	CMP	D1Ar3,#1		! Null -> C and NZ, rest -> NC (\1->Z)
	CMPNC	D1Ar3,D1Ar5		! NOT Null: Same -> Z, else -> NZ
	BZ	$Lstrcmp_slow		! NOT Null and Same: Loop
	SUB	D0Re0,D1Ar3,D1Ar5	! Generate result
	MOV	PC,D1RtP

        .size _strcmp,.-_strcmp


libc_hidden_def(strcmp)
#ifndef __UCLIBC_HAS_LOCALE__
strong_alias(strcmp,strcoll)
libc_hidden_def(strcoll)
#endif
