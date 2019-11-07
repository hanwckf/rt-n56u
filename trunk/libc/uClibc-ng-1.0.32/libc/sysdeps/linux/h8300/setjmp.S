#include <jmpbuf-offsets.h>

#ifdef __H8300S__
	.h8300s
#else
	.h8300h
#endif
	.text

.global __sigsetjmp

__sigsetjmp:
	add.l	#JB_SIZE,er0
	mov.l	@sp,er1		; return PC
	mov.l	er1,@-er0
	mov.l	sp,@-er0
	mov.l	er6,@-er0
	mov.l	er5,@-er0
	mov.l	er4,@-er0
	sub.l	er0,er0
	rts
