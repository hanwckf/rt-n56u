/* glibc's sysdeps/hppa/elf/initfini.c used for reference [PROLOG] */

	.section .init
	.align 4
	.globl _init
	.type _init,@function
_init:
	stw	%rp,-20(%sp)
	stwm	%r4,64(%sp)
	stw	%r19,-32(%sp)
	copy	%r19,%r4	/* delay slot */
	copy	%r4,%r19



	.section .fini
	.align 4
	.globl _fini
	.type _fini,@function
_fini:
	stw	%rp,-20(%sp)
	stwm	%r4,64(%sp)
	stw	%r19,-32(%sp)
	copy	%r19,%r4
