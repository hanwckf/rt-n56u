	.file	"initfini.c"
	
#include <bits/arm_asm.h>
	.section .init
	.global	_init
	.type	_init, %function
#if defined __thumb__
	.align	1
	.thumb
	.thumb_func
_init:
	push	{r4-r7, lr}
#else
	.align	2
	.arm
_init:
	@ gcc 3.3.2 didn't create a stack frame, gcc 3.4.4 does -
	@ presumably 3.4.4 can put stuff into .init which requires
	@ the arguments to be saved.  This code is copied from 3.4.4
	mov	ip, sp
	stmdb	sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
	sub	fp, ip, #4
#endif


	.section .fini
	.global	_fini
	.type	_fini, %function
#if defined __thumb__
	.align	1
	.thumb
	.thumb_func
_fini:
	push	{r4-r7, lr}
#else
	.align	2
	.arm
_fini:
	mov	ip, sp
	stmdb	sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
	sub	fp, ip, #4
#endif


#if (defined __thumb__ || defined __THUMB_INTERWORK__) && (defined __ARM_ARCH_4T__ || defined __ARM_ARCH_5T__ || defined __ARM_ARCH_5TE__)
	@ To support thumb code it is currently necessary to have the _call_via_rX
	@ functions exposed to the linker for any program or shared library.  PLT
	@ references are inadequate - the PLT zaps ip and therefore breaks _call_via_ip
	@ (and the compiler does generate this).  It is simpler to put all the
	@ required code in here - it only amounts to 60 bytes overhead.
	@NOTE: it would be better to have the compiler generate this stuff as
	@ required...
	.section	".text"
	.align 0
	.force_thumb

.macro call_via register
	.global _call_via_\register
	.type	_call_via_\register, %function
	.weak	_call_via_\register
	.hidden	_call_via_\register
	.thumb_func
_call_via_\register:
	bx	\register
	nop
	.size	_call_via_\register, . - _call_via_\register
.endm

	@ and calls for the 15 general purpose registers (2 bytes each).
	call_via r0
	call_via r1
	call_via r2
	call_via r3
	call_via r4
	call_via r5
	call_via r6
	call_via r7
	call_via r8
	call_via r9
	call_via sl
	call_via fp
	call_via ip
	call_via sp
	call_via lr
#endif

	.ident	"GCC: (GNU) 3.3.2 20031005 (Debian prerelease)"
