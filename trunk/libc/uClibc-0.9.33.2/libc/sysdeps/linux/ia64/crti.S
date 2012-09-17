/* glibc's sysdeps/ia64/elf/initfini.c used for reference [PROLOG] */

	.text
	.section .init
	.global _init#
	.proc _init#
_init:
	.prologue
	.save ar.pfs, r34
	alloc r34 = ar.pfs, 0, 3, 0, 0
	.vframe r32
	mov r32 = r12
	.save rp, r33
	mov r33 = b0
	.body
	adds r12 = -16, r12
	;;
	.endp _init#



	.section .fini
	.global _fini#
	.proc _fini#
_fini:
	.prologue
	.save ar.pfs, r34
	alloc r34 = ar.pfs, 0, 3, 0, 0
	.vframe r32
	mov r32 = r12
	.save rp, r33
	mov r33 = b0
	.body
	adds r12 = -16, r12
	;;
	.endp _fini#
