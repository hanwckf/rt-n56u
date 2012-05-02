/*
 * Stolen from glibc-2.2.2 by David Schleef <ds@schleef.org>
 */

.text
.align 4

.globl _dl_linux_resolver

.globl _dl_linux_resolve
.type	_dl_linux_resolve,@function

_dl_linux_resolve:
// We need to save the registers used to pass parameters, and register 0,
// which is used by _mcount; the registers are saved in a stack frame.
	stwu 1,-64(1)
	stw 0,12(1)
	stw 3,16(1)
	stw 4,20(1)
// The code that calls this has put parameters for 'fixup' in r12 and r11.
	mr 3,12
	stw 5,24(1)
	mr 4,11
	stw 6,28(1)
	mflr 0
// We also need to save some of the condition register fields.
	stw 7,32(1)
	stw 0,48(1)
	stw 8,36(1)
	mfcr 0
	stw 9,40(1)
	stw 10,44(1)
	stw 0,8(1)
	bl _dl_linux_resolver@local
// 'fixup' returns the address we want to branch to.
	mtctr 3
// Put the registers back...
	lwz 0,48(1)
	lwz 10,44(1)
	lwz 9,40(1)
	mtlr 0
	lwz 8,36(1)
	lwz 0,8(1)
	lwz 7,32(1)
	lwz 6,28(1)
	mtcrf 0xFF,0
	lwz 5,24(1)
	lwz 4,20(1)
	lwz 3,16(1)
	lwz 0,12(1)
// ...unwind the stack frame, and jump to the PLT entry we updated.
	addi 1,1,64
	bctr

.LFE2:
	.size _dl_linux_resolve,.LFE2-_dl_linux_resolve

