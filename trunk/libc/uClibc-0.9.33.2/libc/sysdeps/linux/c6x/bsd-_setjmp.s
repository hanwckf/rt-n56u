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
 ; along with this program; if not, write to the Free Software Foundation,
 ; Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 ;

	.global _setjmp

_setjmp:
	MV .D2X	A4,B4		; jmp_buf address
||	STW .D1T2	B3,*+A4(48)	; return address

	STW .D1T1	A10,*+A4(0)
||	STW .D2T2	B10,*+B4(4)
||	ZERO .L1	A6
	
	STW .D1T1	A6,*+A4(52)	; no signal mask set
||	B .S2	B3		; returns in 5 cycles

	STW .D1T1	A11,*+A4(8)
||	STW .D2T2	B11,*+B4(12)
	STW .D1T1	A12,*+A4(16)
||	STW .D2T2	B12,*+B4(20)
	STW .D1T1	A13,*+A4(24)
||	STW .D2T2	B13,*+B4(28)
	STW .D1T1	A14,*+A4(32)
||	STW .D2T2	B14,*+B4(36)
	STW .D1T1	A15,*+A4(40)
||	STW .D2T2	B15,*+B4(44)
||	ZERO .L1	A4		; return values




