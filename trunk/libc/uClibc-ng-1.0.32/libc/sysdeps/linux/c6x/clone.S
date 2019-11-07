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
#define __ASSEMBLY__

	; int _clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg);

#include <asm/errno.h>
#include <sys/syscall.h>

	.global __clone
	.global	clone
	.global	__errno_location

 ;Currently supports only
 ;int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg)
 ;
 ;Requires update for supporting
 ; int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
 ;	    int *parent_tidptr, struct user_desc *newtls, int *child_pidptr)

__clone:
	; index 1 points to the forth argument and is to be moved to B6
	LDW .D2T2	*+B15[1],B5
	NOP     4
	OR .D2X	B4,A4,B2	; sanity check arguments, no NULL function or stack pointers
||	MV .S2	B4,B9
||	MV .D1	A4,A9		; backup fn and child_stack pointers

  [!B2]	B .S2	__syscall_error
||[!B2] MVK .S1	EINVAL,A4
	NOP	4

	MV .D1	A6,A4		; get flags as arg0, arg1 is the new stack
||	AND .D2	~7,B4,B4

	; do the system call
||	MVK .S2	__NR_clone,B0
||	MV .L2  B5,B6
0:
#ifndef	_TMS320C6400_PLUS
  	MVC .S2     CSR,B2
	CLR .S2     B2,0,0,B1
	MVC .S2     B1,CSR
	MVC .S2     IFR,B1
	SET .S2     B1,6,6,B1
	MVC .S2     B1,ISR
	MVC .S2     B2,CSR
	NOP
#else
	SWE
#endif

	MV .D2	B9,B4		; restore child stack

||	CMPEQ .L1	0,A4,A2
||	CMPLT .L2X	A4,0,B2

   [B2]	B .S2	__syscall_error	; if syscall < 0, it is an error
	NOP	5
   [A2] B .S2X	A9		; branch to function
|| [A2] MV .D1X	B6,A4		; set arg (B6 is preserved by syscall)
  [!A2] B .S2	B3		; otherwise (syscall result > 0) returns directly
   [A2]	ADDKPC .S2	__return_thread,B3, 4

__return_thread:
	b	.s2	HIDDEN_JUMPTARGET(_exit)
	nop	5

__syscall_error:
	NEG .S1	A4,A4
	STW .D2T1	A4,*B15--[2]
	STW .D2T2	B3,*+B15[1]
	CALLP .S2	__errno_location,B3
	LDW .D2T2	*+B15[1],B3
	LDW .D2T1	*++B15[2],A5
	NOP	3
	BNOP .S2	B3,3
	STW .D1T1	A5,*A4
	MVK .L1	-1,A4

.set clone, __clone
