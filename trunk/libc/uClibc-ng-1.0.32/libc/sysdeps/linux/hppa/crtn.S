/* glibc's sysdeps/hppa/elf/initfini.c used for reference [EPILOG] */

        .text
        .align 4
/* Here is the tail end of _init.  We put __gmon_start before this so
   that the assembler creates the .PARISC.unwind section for us, ie.
   with the right attributes.  */
	.section .init
	ldw	-84(%sp),%rp
	copy	%r4,%r19
	bv	%r0(%rp)
_end_init:
	ldwm	-64(%sp),%r4

/* Our very own unwind info, because the assembler can't handle
   functions split into two or more pieces.  */
	.section .PARISC.unwind
	.extern _init
	.word	_init, _end_init
	.byte	0x08, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08



	.section .fini
	ldw	-84(%sp),%rp
	copy	%r4,%r19
	bv	%r0(%rp)
_end_fini:
	ldwm	-64(%sp),%r4

	.section .PARISC.unwind
	.extern _fini
	.word	_fini, _end_fini
	.byte	0x08, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08
