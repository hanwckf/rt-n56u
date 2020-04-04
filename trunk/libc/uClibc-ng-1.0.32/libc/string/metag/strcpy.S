!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.


	.text
	.global	_strcpy
	.type	_strcpy,function
! D1Ar1 dst
! D0Ar2 src

_strcpy:
	MOV	A1.2, D1Ar1

	! test 4 byte alignment of src
	ANDS	D0Ar4, D0Ar2, #3
	BNZ	$Lbyteloop

	! test 4 byte alignment of dest
	ANDS	D1Ar5, D1Ar1, #3
	BNZ	$Lbyteloop

	! load mask values for aligned loops
	MOVT	D1Ar3, #HI(0xfefefeff)
	ADD	D1Ar3, D1Ar3, #LO(0xfefefeff)
	MOVT	D0FrT, #HI(0x80808080)
	ADD	D0FrT, D0FrT, #LO(0x80808080)

	! test 8 byte alignment of src
	ANDS	D0Ar4, D0Ar2, #7
	BNZ	$Lwordloop

	! test 8 byte alignment of dest
	ANDS	D1Ar5, D1Ar1, #7
	BNZ	$Lwordloop

$L8byteloop:
	GETL	D1Ar5, D0Ar6, [D0Ar2++]
	MOV	D1Re0, D1Ar5
	MOV	D0Re0, D1Ar5
	ADD	D1Re0, D1Re0, D1Ar3
	XOR	D0Re0, D0Re0, #-1
	AND	D1Re0, D1Re0, D0Re0
	ANDS	D1Re0, D1Re0, D0FrT
	BNZ	$Lnullfound		! NULL in first word

	MOV	D1Re0, D0Ar6
	MOV	D0Re0, D0Ar6
	ADD	D1Re0, D1Re0, D1Ar3
	XOR	D0Re0, D0Re0, #-1
	AND	D1Re0, D1Re0, D0Re0
	ANDS	D1Re0, D1Re0, D0FrT
	BNZ	$Lnullfound2		! NULL in the second word

	SETL	[A1.2++], D1Ar5, D0Ar6
	B	$L8byteloop

$Lwordloop:
	GETD	D0Ar6, [D0Ar2++]
	MOV	D1Re0, D0Ar6
	MOV	D0Re0, D0Ar6
	ADD	D1Re0, D1Re0, D1Ar3
	XOR	D0Re0, D0Re0, #-1
	AND	D1Re0, D1Re0, D0Re0
	ANDS	D1Re0, D1Re0, D0FrT
	MOV	D1Ar5, D0Ar6
	BNZ	$Lnullfound
	SETD	[A1.2++], D0Ar6
	B	$Lwordloop

$Lnullfound2:
	SETD	[A1.2++], D1Ar5
	MOV	D1Ar5, D0Ar6

$Lnullfound:
	SETB	[A1.2++], D1Ar5
	ANDS	D0Ar6, D1Ar5, #0xff
	LSR	D1Ar5, D1Ar5, #8
	BNZ	$Lnullfound
	B	$Lend

$Lbyteloop:
	GETB	D0Ar6, [D0Ar2++]
	SETB	[A1.2++], D0Ar6
	CMP	D0Ar6, #0
	BNZ	$Lbyteloop

$Lend:
	MOV	D0Re0, D1Ar1
	MOV	PC, D1RtP

	.size _strcpy,.-_strcpy

libc_hidden_def(strcpy)
