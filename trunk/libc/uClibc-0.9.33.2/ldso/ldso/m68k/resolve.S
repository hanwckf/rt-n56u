/*
 * These are various helper routines that are needed to run an ELF image.
 */

.text
.even

.globl _dl_linux_resolve
	.type	_dl_linux_resolve,@function
_dl_linux_resolve:
	# Save %a0 (struct return address) and %a1.
	move.l %a0, -(%sp)
	move.l %a1, -(%sp)
	# Call the real address resolver.
	jbsr _dl_linux_resolver
	# Restore register %a0 and %a1.
	move.l (%sp)+, %a1
	move.l (%sp)+, %a0
	# Pop parameters
	addq.l #8, %sp
	# Call real function.
#if defined __mcoldfire__
	move.l %d0,-(%sp)
	rts
#else
	jmp (%d0)
#endif
.size _dl_linux_resolve,.-_dl_linux_resolve
