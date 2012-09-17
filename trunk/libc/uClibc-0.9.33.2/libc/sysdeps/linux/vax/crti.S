	.file	"initfini.c"
	.version	"01.01"
gcc2_compiled.:
__gnu_compiled_c:
	.section .init
	.align 1
.globl _init
	.type	 _init,@function
_init:
	.word 0x0000

	.align 1

	.section .fini
	.align 1
.globl _fini
	.type	 _fini,@function
_fini:
	.word 0x0000
	.align 1
	.ident	"GCC: (GNU) 2.95.2 19991024 (release) (Linux/VAX CVS)"
