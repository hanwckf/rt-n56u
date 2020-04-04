;
; Port of uClibc for TMS320C6000 DSP architecture
; 
; Copyright (C) 2010 Texas Instruments Incorporated
; Mark Salter <msalter@redhat.com>
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

       .text

	;; On entry, the dynamic linker
	;;
	;;	0(sp)			pad0
	;;	4(sp)			pad1
	;;	8(sp)			argc
	;;	12(sp)			argv[0]
	;;	...
	;;	(4*(argc+3))(sp)	NULL
	;;	(4*(argc+4))(sp)	envp[0]
	;;	...
	;;				NULL

	;; Register values are unspecified, except:
	;;
	;;	A4  --> pointer to rtld fini rountine
	;;	B14 --> pointer to application DSBT table

	.global	_start
_start:
	.global _c_int00
_c_int00:
	;; Things to do:
	;;
	;; 	* call __uClibc_main(
	;; 		int (*main)(int, char **, char **),	A4
	;; 		int argc,				B4
	;;		char **argv,			   	A6
	;; 		void (*app_init)(void),		   	B6
	;; 		void (*app_fini)(void),		   	A8
	;;		void (*rtld_fini)(void),		B8
	;; 		void *stack_end)			A10

	MV .D2X	A4,B8		; rtld_fini

	LDW .D2T1	*+B14($GOT(main)), A4
	LDW .D2T2	*+B14($GOT(_init)), B6
	B .S2		__uClibc_main
||	LDW .D2T1	*+B14($GOT(_fini)), A8

	LDW .D2T2	*+B15(8),B4	; argc
	ADDAW .D1X	B15,3,A6	; **argv
	MV .D1X		B15,A10		; stack_end
||	ZERO .L1	A15		; clear FP
	NOP		2
