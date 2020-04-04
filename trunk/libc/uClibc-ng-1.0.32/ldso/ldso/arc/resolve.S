/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>
#include <sys/syscall.h>

; Save the registers which resolver could possibly clobber
; 	r0-r9: args to the function - symbol being resolved
; 	r10-r12 are already clobbered by PLTn, PLT0 thus neednot be saved

.macro	SAVE_CALLER_SAVED
	push_s	r0
	push_s	r1
	push_s	r2
	push_s	r3
	st.a	r4, [sp, -4]
	st.a	r5, [sp, -4]
	st.a	r6, [sp, -4]
	st.a	r7, [sp, -4]
	st.a	r8, [sp, -4]
	st.a	r9, [sp, -4]
	push_s	blink
.endm

.macro RESTORE_CALLER_SAVED_BUT_R0
	ld.ab	blink,[sp, 4]
	ld.ab	r9, [sp, 4]
	ld.ab	r8, [sp, 4]
	ld.ab	r7, [sp, 4]
	ld.ab	r6, [sp, 4]
	ld.ab	r5, [sp, 4]
	ld.ab	r4, [sp, 4]
	pop_s   r3
	pop_s   r2
	pop_s   r1
.endm

; Upon entry, PLTn, which led us here, sets up the following regs
; 	r11 = Module info (tpnt pointer as expected by resolver)
;	r12 = PC of the PLTn itself - needed by resolver to find
;	      corresponding .rela.plt entry

ENTRY(_dl_linux_resolve)
	; args to func being resolved, which resolver might clobber
	SAVE_CALLER_SAVED

	mov_s 	r1, r12
	bl.d  	_dl_linux_resolver
	mov   	r0, r11

	RESTORE_CALLER_SAVED_BUT_R0
	j_s.d   [r0]    ; r0 has resolved function addr
	pop_s   r0      ; restore first arg to resolved call
END(_dl_linux_resolve)
