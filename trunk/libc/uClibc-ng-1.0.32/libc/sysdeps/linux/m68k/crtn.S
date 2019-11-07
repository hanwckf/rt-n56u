#include "m68k_pic.S"

	.section .init
	FINI_GP
	unlk	%a6
	rts
	
	.section .fini
	FINI_GP
	unlk	%a6
	rts
