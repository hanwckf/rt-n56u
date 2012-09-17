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
	.global __init
__init:
	mov.l	er6,@-er7
	mov.l	er7,er6
; #APP
	.align 1

	.section .fini
; #NO_APP
	.align 1
	.global __fini
__fini:
	mov.l	er6,@-er7
	mov.l	er7,er6
; #APP
	.align 1

	.end
