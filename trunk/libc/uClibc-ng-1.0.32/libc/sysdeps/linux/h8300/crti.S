#ifdef __H8300H__
	.h8300h
#endif
#ifdef __H8300S__
	.h8300s
#endif
	.file	"initfini.c"
; #APP

	.section .init
; #NO_APP
	.align 1
	.global _init
_init:
	mov.l	er6,@-er7
	mov.l	er7,er6
; #APP
	.align 1

	.section .fini
; #NO_APP
	.align 1
	.global _fini
_fini:
	mov.l	er6,@-er7
	mov.l	er7,er6
; #APP
	.align 1

	.end
