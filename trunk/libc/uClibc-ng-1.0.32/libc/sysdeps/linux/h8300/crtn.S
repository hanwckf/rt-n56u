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
; #NO_APP
	mov.l	@er7+,er6
	rts
; #APP

	.section .fini
; #NO_APP
	.align 1
	.global _fini
; #NO_APP
	mov.l	@er7+,er6
	rts
; #APP

	.end
