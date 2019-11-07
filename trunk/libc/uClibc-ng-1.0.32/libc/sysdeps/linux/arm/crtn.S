	.file	"initfini.c"
	
#include <bits/arm_asm.h>
	.section .init
	.global	_init
	.type	_init, %function
#if defined __thumb__
	.align	1
	.thumb
	@ this will not work on ARMv4T, but lots of stuff
	@ in here won't work there anyway...
	pop	{r4-r7, pc}
#else
	.align	2
	.arm
	ldmdb	fp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
#endif
	
	.section .fini
	.global	_fini
	.type	_fini, %function
#if defined __thumb__
	.align	1
	.thumb
	pop	{r4-r7, pc}
#else
	.align	2
	.arm
	ldmdb	fp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
#endif
	
	@ In fact this is modified to 3.4.4
	.ident	"GCC: (GNU) 3.3.2 20031005 (Debian prerelease)"
