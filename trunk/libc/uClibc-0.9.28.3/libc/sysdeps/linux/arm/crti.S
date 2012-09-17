	.file	"initfini.c"
	
	.section .init
	.align	2
	.global	_init
	.type	_init, %function
_init:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	str	lr, [sp, #-4]!
	
	.align 2
	
	
	.section .fini
	.align	2
	.global	_fini
	.type	_fini, %function
_fini:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	str	lr, [sp, #-4]!
	.align 2
	
	
	.ident	"GCC: (GNU) 3.3.2 20031005 (Debian prerelease)"
