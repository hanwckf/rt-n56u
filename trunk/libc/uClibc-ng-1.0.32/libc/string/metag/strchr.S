!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.


#include <features.h>

	.text
	.global	_strchr
	.type	_strchr, function
! D1Ar1 src
! D0Ar2 c
_strchr:
	AND     D0Ar2,D0Ar2,#0xff                           ! Drop all but 8 bits of c
	MOV 	D1Ar5, D1Ar1                                ! Copy src to D1Ar5
	AND 	D1Ar5, D1Ar5, #7                            ! Check 64 bit alignment
	CMP 	D1Ar5, #0
	BZ 	$Laligned64bit                              ! Jump to 64 bit aligned strchr
$Lalign64bit:
	GETB 	D0Re0, [D1Ar1++]                            ! Get the next character
	ADD 	D1Ar5, D1Ar5, #1                            ! Increment alignment counter
	CMP 	D0Re0, D0Ar2                                ! Is the char c
	BZ 	$Lcharatprevious                            ! If so exit returning position
	CMP 	D0Re0, #0                                   ! End of string?
	BZ 	$Lnotfound                                  ! If so exit
	CMP 	D1Ar5, #8                                   ! Are we aligned 64bit yet?
	BNZ 	$Lalign64bit                                ! If not keep aligning
$Laligned64bit:                                             ! src is 64bit aligned
	MOV 	D0Ar4, D0Ar2                                ! put c into D0Ar4
	LSL 	D0Ar4, D0Ar4, #8                            ! Shift it up
	ADD 	D0Ar4, D0Ar4, D0Ar2                         ! another c
	LSL 	D0Ar4, D0Ar4, #8                            ! shift
	ADD 	D0Ar4, D0Ar4, D0Ar2                         ! another c
	LSL 	D0Ar4, D0Ar4, #8                            ! shift
	ADD 	D0Ar4, D0Ar4, D0Ar2                         ! 4 copies of c
$Lcheck8bytes:
	GETL 	D0Re0, D1Re0, [D1Ar1++]                     ! grab 16 bytes
	MOV 	A0.3, D0Re0                                 ! save for later use
							    ! first word
							    ! check for \0
	MOV 	D0Ar2, D0Re0                                ! D0Ar2 is a scratch now
	ADDT 	D0Re0, D0Re0, #HI(0xfefefeff)               ! Do 4 1-byte compares
	ADD 	D0Re0, D0Re0, #LO(0xfefefeff)
	XOR 	D0Ar2, D0Ar2, #-1
	AND 	D0Re0, D0Re0, D0Ar2
	ANDMT 	D0Re0, D0Re0, #HI(0x80808080)
	ANDMB 	D0Re0, D0Re0, #LO(0x80808080)
	CMP 	D0Re0, #0
	BNZ 	$Lnullinword1                               ! found \0 (or c if c==\0)

							    ! Check for c
	MOV 	D0Re0, A0.3                                 ! restore the first word
	XOR 	D0Re0, D0Re0, D0Ar4
	MOV 	D0Ar2, D0Re0                                ! DO 4 1-byte compares
	ADDT 	D0Re0, D0Re0, #HI(0xfefefeff)
	ADD 	D0Re0, D0Re0, #LO(0xfefefeff)
	XOR 	D0Ar2, D0Ar2, #-1
	AND 	D0Re0, D0Re0, D0Ar2
	ANDMT 	D0Re0, D0Re0, #HI(0x80808080)
	ANDMB 	D0Re0, D0Re0, #LO(0x80808080)
	CMP 	D0Re0, #0
	BNZ 	$Lcharinword1                               ! found c

							    ! second word
							    ! check for \0
	MOV 	A0.3, D1Re0                                 ! save for later use
	MOV 	D1Ar3, D1Re0
	ADDT 	D1Re0, D1Re0, #HI(0xfefefeff)               ! Do 4 1-byte compares
	ADD 	D1Re0, D1Re0, #LO(0xfefefeff)
	XOR 	D1Ar3, D1Ar3, #-1
	AND 	D1Re0, D1Re0, D1Ar3
	ANDMT 	D1Re0, D1Re0, #HI(0x80808080)
	ANDMB 	D1Re0, D1Re0, #LO(0x80808080)
	CMP 	D1Re0, #0
	BNZ 	$Lnullinword2                               ! Found \0 (or c if c==\0)

	MOV 	D0.4, A0.3                                  ! restore the second word
	XOR 	D1Re0, D0.4, D0Ar4                          ! test c

	MOV 	D1Ar3, D1Re0
	ADDT 	D1Re0, D1Re0, #HI(0xfefefeff)               ! Do 4 1-byte compares
	ADD 	D1Re0, D1Re0, #LO(0xfefefeff)
	XOR 	D1Ar3, D1Ar3, #-1
	AND 	D1Re0, D1Re0, D1Ar3
	ANDMT 	D1Re0, D1Re0, #HI(0x80808080)
	ANDMB 	D1Re0, D1Re0, #LO(0x80808080)
	CMP 	D1Re0, #0
	BNZ 	$Lcharinword2                               ! found c

	B 	$Lcheck8bytes                               ! Keep checking

$Lnullinword1:                                              ! found \0 somewhere, check for c too
	SUB 	D1Ar1, D1Ar1, #4
$Lnullinword2:
	SUB 	D1Ar1, D1Ar1, #4
	AND 	D0Ar2, D0Ar4, #0xff                         ! restore c
	MOV 	D0Re0, A0.3                                 ! restore the word
	MOV 	D0.4, D0Re0                                 ! for shifting later
	AND 	D0Re0, D0Re0, #0xff                         ! take first byte of word
	CMP 	D0Re0, D0Ar2
	BZ 	$Lcharatcurrent                             ! found c
	CMP 	D0Re0, #0!
	BZ 	$Lnotfound                                  ! found \0

	ADD 	D1Ar1, D1Ar1, #1
	LSR 	D0.4, D0.4, #8
	MOV 	D0Re0, D0.4
	AND 	D0Re0, D0Re0, #0xff                         ! take second byte of word
	CMP 	D0Re0, D0Ar2
	BZ 	$Lcharatcurrent                             ! found c
	CMP 	D0Re0, #0
	BZ 	$Lnotfound                                  ! found \0

	ADD 	D1Ar1, D1Ar1, #1
	LSR 	D0.4, D0.4, #8
	MOV 	D0Re0, D0.4
	AND 	D0Re0, D0Re0, #0xff                         ! take third byte of word
	CMP 	D0Re0, D0Ar2
	BZ 	$Lcharatcurrent                             ! found c
	CMP 	D0Re0, #0
	BZ 	$Lnotfound                                  ! found \0

	ADD 	D1Ar1, D1Ar1, #1                            ! move to 4th byte
	CMP     D0Ar2, #0                                   ! If c was \0
	BZ      $Lcharatcurrent                             ! c has been found!

$Lnotfound:
	MOV 	D0Re0,		#0                          ! End of string c not found
	B 	$Lend

$Lcharinword1: 						    ! found c in first word
	MOV 	D1Re0, D0Re0
	SUB 	D1Ar1, D1Ar1, #4
$Lcharinword2:                                              ! found c in second word
	SUB 	D1Ar1, D1Ar1, #4

	AND 	D0Re0, D1Re0, #0xff                         ! First byte
	CMP 	D0Re0, #0                                   ! Test c (zero indicates c due
							    ! to the 4 1-byte compare code)
	BNE 	$Lcharatcurrent
	ADD 	D1Ar1, D1Ar1, #1

	LSR 	D1Re0, D1Re0, #8
	AND 	D0Re0, D1Re0, #0xff                         ! Second byte
	CMP 	D0Re0, #0                                   ! Test c (indicated by zero)
	BNE 	$Lcharatcurrent
	ADD 	D1Ar1, D1Ar1, #1

	LSR 	D1Re0, D1Re0, #8
	AND 	D0Re0, D1Re0, #0xff                         ! Third byte
	CMP 	D0Re0, #0                                   ! Test c (indicated by zero)
	BNE 	$Lcharatcurrent
	ADD 	D1Ar1, D1Ar1, #1                            ! Must be the fourth byte
	B 	$Lcharatcurrent

$Lcharatprevious:
	SUB 	D1Ar1, D1Ar1, #1                            ! Fix-up pointer
$Lcharatcurrent:
	MOV 	D0Re0, D1Ar1                                ! Return the string pointer
$Lend:
	MOV 	PC, D1RtP
	.size _strchr,.-_strchr

libc_hidden_def(strchr)
#ifdef __UCLIBC_SUSV3_LEGACY__
strong_alias(strchr,index)
#endif
