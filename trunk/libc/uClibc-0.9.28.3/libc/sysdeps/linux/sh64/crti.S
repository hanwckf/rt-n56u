	.file	"initfini.c"
	.section	.text..SHmedia32,"ax"
	.little
	
	.section .init
	.hidden  _init
	.align 2
	.global	_init
	.type	_init, @function
_init:
	addi.l	r15, -16, r15
	st.l	r15, 4, r12
	movi	(((datalabel _GLOBAL_OFFSET_TABLE_-(.LPCS0-.)) >> 16) & 65535), r12
	shori	((datalabel _GLOBAL_OFFSET_TABLE_-(.LPCS0-.)) & 65535), r12
	.LPCS0: ptrel/u	r12, tr0
	st.l	r15, 8, r14
	st.l	r15, 12, r18
	add.l	r15, r63, r14
	gettr	tr0, r12
	
	.align 2
	
	
	.section .fini
	.hidden  _fini
	.align 2
	.global	_fini
	.type	_fini, @function
_fini:
	addi.l	r15, -16, r15
	st.l	r15, 4, r12
	movi	(((datalabel _GLOBAL_OFFSET_TABLE_-(.LPCS1-.)) >> 16) & 65535), r12
	shori	((datalabel _GLOBAL_OFFSET_TABLE_-(.LPCS1-.)) & 65535), r12
	.LPCS1: ptrel/u	r12, tr0
	st.l	r15, 8, r14
	st.l	r15, 12, r18
	add.l	r15, r63, r14
	gettr	tr0, r12
	.align 2
	
	
	.ident	"GCC: (GNU) 3.3.2"
