.file "initfini.c";
	.section .init
	.section .fini

.text;
.align 2
.global _dummy;
.type _dummy, STT_FUNC;
_dummy:
	P2 = R0;
	LINK 0;
	SP += -12;
	cc =P2==0;
	if cc jump L$L$1;
	SP += 12;
	UNLINK;
	jump (P2);
L$L$1:
	SP += 12;
	UNLINK;
	rts;
.align 2
.global __init;
.type __init, STT_FUNC;
__init:
	LINK 0;
	SP += -12;

	.align 2
