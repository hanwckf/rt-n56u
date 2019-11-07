/* glibc's sysdeps/ia64/elf/initfini.c used for reference [EPILOG] */

	.text
	.section .init
	.proc _init#
_init:
	.prologue
	.save ar.pfs, r34
	.vframe r32
	.save rp, r33
	.body
	.regstk 0,2,0,0
	mov r12 = r32
	mov ar.pfs = r34
	mov b0 = r33
	br.ret.sptk.many b0
	.endp _init#



	.section .fini
	.proc _fini#
_fini:
	.prologue
	.save ar.pfs, r34
	.vframe r32
	.save rp, r33
	.body
	mov r12 = r32
	mov ar.pfs = r34
	mov b0 = r33
	br.ret.sptk.many b0
	.endp _fini#
