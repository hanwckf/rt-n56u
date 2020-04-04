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

	.global __longjmp

__longjmp:
	LDW .D1T1    *+A4(48),A3	; return address
	MV .D2X     A4,B6		; jmp_buf pointer
||	MV .D1     A4,A6
||	MV .S2     B4,B2		; val

	LDW .D1T1    *+A6(0),A10
||	LDW .D2T2    *+B6(4),B10
|| [B2]	MV .S1X     B4,A4
||[!B2]	MVK .L1    1,A4			;  return val or 1

	LDW .D1T1    *+A6(8),A11
||	LDW .D2T2    *+B6(12),B11
	LDW .D1T1    *+A6(16),A12
||	LDW .D2T2    *+B6(20),B12
	LDW .D1T1    *+A6(24),A13
||	LDW .D2T2    *+B6(28),B13
	LDW .D1T1    *+A6(32),A14
||	LDW .D2T2    *+B6(36),B14
	LDW .D1T1    *+A6(40),A15
||	LDW .D2T2    *+B6(44),B15
||	B .S2X	A3
	NOP    5

libc_hidden_def(__longjmp)
