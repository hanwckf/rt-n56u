#ifdef __H8300S__
	.h8300s
#else
	.h8300h
#endif
	.text
	
.global ___longjmp

___longjmp:
	mov.l	er1,er1
	bne	1f
	sub.l	er1,er1
	inc.l	#1,er1
1:
	mov.l	@er0+,er4
	mov.l	@er0+,er5
	mov.l	@er0+,er6
	mov.l	@er0+,sp
	mov.l	@er0+,er3	; return PC
	adds	#4,sp		; adjust return stack
	jmp	@er3
