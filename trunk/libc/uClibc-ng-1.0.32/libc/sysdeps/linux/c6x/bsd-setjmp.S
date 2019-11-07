 ;
 ; Port of uClibc for TMS320C6000 DSP architecture
 ; Copyright (C) 2004 Texas Instruments Incorporated
 ; Author of TMS320C6000 port: Aurelien Jacquiot
 ;
 ; This program is free software; you can redistribute it and/or modify it
 ; under the terms of the GNU Library General Public License as published by
 ; the Free Software Foundation; either version 2 of the License, or (at your
 ; option) any later version.
 ;
 ; This program is distributed in the hope that it will be useful, but WITHOUT
 ; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ; FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 ; for more details.
 ;
 ; You should have received a copy of the GNU Library General Public License
 ; along with this program; if not, see <http://www.gnu.org/licenses/>.
 ;

	.global setjmp
setjmp:
#if 0
.if 1 /* was: .if (CONFIG_UCLIBC_SHARED == 0) */

	MVKL .S1	___curr_eh_stack_entry,A6
||	SUB .D1X	A4,B15,A3
	MVKH .S1	___curr_eh_stack_entry,A6
||	CMPGT .L1	A3,4,A0		; A0 set if C++ exceptions case

	LDW .D1T1	*A6,A2
.else
	MVKL .S2	(___curr_eh_stack_entry - $bss)/4,B6
||	SUB .D1X	A4,B15,A3
	MVKH .S2	(___curr_eh_stack_entry - $bss)/4,B6
||	CMPGT .L1	A3,4,A0		; A0 set if C++ exceptions case

	LDW .D2T1	*+B14[B6],A2
.endif
	NOP
#else
	MVK .S1		0, A0
#endif

	MVK .L2	1,B4		; indicate to ___sigjmp_save to save signal mask
||	MV .D2X	A4,B6		; jmp_buf address
||	STW .D1T2	B3,*+A4(48)	; return address

	ADDAW .D1	A2,2,A2
 [A0]	CMPEQ .L1	A4,A2,A0	; A0 set if C++ exceptions case

	STW .D1T1	A10,*+A4(0)
||	STW .D2T2	B10,*+B6(4)
||[!A0]	B .S1		__sigjmp_save		; branch to ___sigjmp_save in 5 cycles
||[A0]	B .S2		B3

	STW .D1T1	A11,*+A4(8)
||	STW .D2T2	B11,*+B6(12)
	STW .D1T1	A12,*+A4(16)
||	STW .D2T2	B12,*+B6(20)
	STW .D1T1	A13,*+A4(24)
||	STW .D2T2	B13,*+B6(28)
	STW .D1T1	A14,*+A4(32)
||	STW .D2T2	B14,*+B6(36)
	STW .D1T1	A15,*+A4(40)
||	STW .D2T2	B15,*+B6(44)
||[A0]	ZERO .L1	A4		; returns 0 for the C++ case
