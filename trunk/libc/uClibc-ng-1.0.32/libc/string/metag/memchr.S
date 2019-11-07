!    Copyright (C) 2013 Imagination Technologies Ltd.
!
!    Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.

	.text
	.global	_memchr
	.type	_memchr,function
! D0Ar6 src
! D0Ar2 c
! D1Ar3 n
_memchr:
	CMP     D1Ar3, #0
	BEQ	$Lexit_fail
	!! convert c to unsigned char
	AND     D0Ar2,D0Ar2,#0xff
	MOV	D0Ar6, D1Ar1
	MOV	D1Ar5, D0Ar6
	!! test alignment
	AND	D1Ar5, D1Ar5, #7
	CMP	D1Ar5, #0
	BNZ	$Lunaligned_loop
	!! length must be greater than or equal to 8 for aligned loop
	CMP     D1Ar3, #8
	BGE     $Laligned_setup
$Lunaligned_loop:
	!! get 1 char from s
	GETB	D0Re0, [D0Ar6++]
	!! increase alignment counter
	ADD	D1Ar5, D1Ar5, #1
	!! decrement n
	SUB     D1Ar3, D1Ar3, #1
	!! exit if we have a match
	CMP	D0Re0, D0Ar2
	BZ	$Lexit_success1
	!! exit if we have hit the end of the string
	CMP	D1Ar3, #0
	BZ	$Lexit_fail
	!! fall through if the buffer is aligned now
	CMP	D1Ar5, #8
	BNE	$Lunaligned_loop
	!! fall through if there is more than 8 bytes left
	CMP	D1Ar3, #8
	BLT	$Lunaligned_loop
$Laligned_setup:
	!! fill the c into 4 bytes
	MOV	D0Ar4, D0Ar2
	LSL	D0Ar4, D0Ar4, #8
	ADD	D0Ar4, D0Ar4, D0Ar2
	LSL	D0Ar4, D0Ar4, #8
	ADD	D0Ar4, D0Ar4, D0Ar2
	LSL	D0Ar4, D0Ar4, #8
	ADD	D0Ar4, D0Ar4, D0Ar2
	!! divide n by 8
	MOV	D1Ar5, D1Ar3
	LSR	D1Ar5, D1Ar5, #3
$Laligned_loop:
	!! get 8 chars from s
	GETL	D0Re0, D1Re0, [D0Ar6++]
	!! decrement loop counter
	SUB	D1Ar5, D1Ar5, #1
	!! test first 4 chars
	XOR	D0Re0, D0Re0, D0Ar4
	!! test second 4 chars
	MOV	D0Ar2, D1Re0
	XOR	D1Re0, D0Ar2, D0Ar4
	!! check for matches in the first 4 chars
	MOV	D0Ar2, D0Re0
	ADDT	D0Re0, D0Re0, #HI(0xfefefeff)
	ADD	D0Re0, D0Re0, #LO(0xfefefeff)
	XOR	D0Ar2, D0Ar2, #-1
	AND	D0Re0, D0Re0, D0Ar2
	ANDMT	D0Re0, D0Re0, #HI(0x80808080)
	ANDMB	D0Re0, D0Re0, #LO(0x80808080)
	CMP	D0Re0, #0
	BNZ	$Lmatch_word1
	!! check for matches in the second 4 chars
	MOV	D1Ar1, D1Re0
	ADDT	D1Re0, D1Re0, #HI(0xfefefeff)
	ADD	D1Re0, D1Re0, #LO(0xfefefeff)
	XOR	D1Ar1, D1Ar1, #-1
	AND	D1Re0, D1Re0, D1Ar1
	ANDMT	D1Re0, D1Re0, #HI(0x80808080)
	ANDMB	D1Re0, D1Re0, #LO(0x80808080)
	CMP	D1Re0, #0
	BNZ	$Lmatch_word2
	!! check if we have reached the end of the buffer
	CMP	D1Ar5, #0
	BNE	$Laligned_loop
	!! exit if there are no chars left to check
	AND	D1Ar3, D1Ar3, #7
	CMP	D1Ar3, #0
	BZ	$Lexit_fail
	!! recover c
	AND	D0Ar2, D0Ar4, #0xff
$Lbyte_loop:
	!! get 1 char from s
	GETB	D0Re0, [D0Ar6++]
	!! decrement n
	SUB	D1Ar3, D1Ar3, #1
	!! exit if we have a match
	CMP	D0Re0, D0Ar2
	BZ	$Lexit_success1
	!! fall through if we have run out of chars
	CMP	D1Ar3, #0
	BNE	$Lbyte_loop

$Lexit_fail:
	MOV	D0Re0, #0
	B	$Lend

$Lmatch_word1:
	!! move the match word into D1Re0
	MOV	D1Re0, D0Re0
	!! roll back the buffer pointer by 4 chars
	SUB	D0Ar6, D0Ar6, #4
$Lmatch_word2:
	!! roll back the buffer pointer by 4 chars
	SUB	D0Ar6, D0Ar6, #4
	!! exit if lowest byte is 0
	MOV	D1Ar1, D1Re0
	AND	D1Ar1, D1Ar1, #0xff
	CMP	D1Ar1, #0
	BNE	$Lexit_success2
	!! advance buffer pointer to the next char
	ADD	D0Ar6, D0Ar6, #1
	!! shift in the next lowest byte
	LSR	D1Re0, D1Re0, #8
	!! exit if lowest byte is 0
	MOV	D1Ar1, D1Re0
	AND	D1Ar1, D1Ar1, #0xff
	CMP	D1Ar1, #0
	BNE	$Lexit_success2
	!! advance buffer pointer to the next char
	ADD	D0Ar6, D0Ar6, #1
	!! shift in the next lowest byte
	LSR	D1Re0, D1Re0, #8
	!! exit if lowest byte is 0
	MOV	D1Ar1, D1Re0
	AND	D1Ar1, D1Ar1, #0xff
	CMP	D1Ar1, #0
	BNE	$Lexit_success2
	!! the match must be in the last byte, exit
	ADD	D0Ar6, D0Ar6, #1
	B	$Lexit_success2

$Lexit_success1:
	SUB	D0Ar6, D0Ar6, #1
$Lexit_success2:
	!! return the buffer pointer
	MOV	D0Re0, D0Ar6
$Lend:
	MOV	PC, D1RtP

	.size _memchr,.-_memchr

libc_hidden_def(memchr)
