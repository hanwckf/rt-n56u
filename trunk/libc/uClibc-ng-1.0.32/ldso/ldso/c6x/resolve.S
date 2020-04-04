;;
;;  Copyright (C) 2010 Texas Instruments Incorporated
;;  Mark Salter <msalter@redhat.com>
;;
;;  Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
;;

;;      The function below is tail-called by resolver stubs when a
;; 	lazily-bound function is called.  It must preserve all
;; 	registers that could be used to pass arguments to the actual
;; 	function.

;; 	_dl_linux_resolver() figures out where the jump symbol is
;; 	_really_ supposed to have jumped to and returns that to us.
;; 	Once we have that, we prepare to tail-call the actual
;; 	function, clean up after ourselves, restoring the original
;; 	arguments, then jump to the fixed up address.  */

; resolver stub - called from PLT to resolve target address and update GOT
;
;   B0        : reloc offset (bytes from DT_RELPLT)
;   B1        : module pointer, loaded from GOT[1]
;   DP        : caller's DP
;   A4,B4, etc: callee's arguments
;   B3        : return address

	.text
	.align 5
	.global	_dl_linux_resolve
_dl_linux_resolve:
	stw .d2t2	B14, *B15--[2]
	stdw .d2t1	A15:A14, *B15--
	stdw .d2t2	B13:B12, *B15--
	stdw .d2t1	A13:A12, *B15--
	stdw .d2t2	B11:B10, *B15--
	stdw .d2t1	A11:A10, *B15--
	stdw .d2t2	B9:B8, *B15--
	stdw .d2t1	A9:A8, *B15--
	stdw .d2t2	B7:B6, *B15--
	stdw .d2t1	A7:A6, *B15--
	stdw .d2t2	B5:B4, *B15--
	stdw .d2t1	A5:A4, *B15--
	stdw .d2t2	B3:B2, *B15--
	stdw .d2t1	A3:A2, *B15--

	; call lookup routine
	MV .S1X		B1, A4		; arg 1: module id
||	MV .S2		B0,B4		; arg 2: reloc offset
	CALLP .S2	_dl_linux_resolver, B3	; returns &f in A4
	MV .S2X		A4,B0            ; &f

	lddw .d2t1	*++B15, A3:A2
	lddw .d2t2	*++B15, B3:B2
	lddw .d2t1	*++B15, A5:A4
	lddw .d2t2	*++B15, B5:B4
	lddw .d2t1	*++B15, A7:A6
	lddw .d2t2	*++B15, B7:B6
	lddw .d2t1	*++B15, A9:A8
	lddw .d2t2	*++B15, B9:B8
	lddw .d2t1	*++B15, A11:A10
	lddw .d2t2	*++B15, B11:B10
	lddw .d2t1	*++B15, A13:A12
	lddw .d2t2	*++B15, B13:B12
	lddw .d2t1	*++B15, A15:A14
	ldw .d2t2	*++B15[2], B14

	B .S2		B0               ; tail-call f
	NOP		5
