/*
 * Stolen from glibc-2.2.2 by Eddie C. Dost <ecd@atecom.com>
 */

	.text
	.globl	_dl_linux_resolver
	.globl	_dl_linux_resolve
	.type	_dl_linux_resolve, @function
	.balign	16
_dl_linux_resolve:
	mov.l	r2, @-r15	! see Note2 below
	mov.l	r3, @-r15
	mov.l	r4, @-r15
	mov.l	r5, @-r15
	mov.l	r6, @-r15
	mov.l	r7, @-r15
	mov.l	r12, @-r15
	movt	r3		! Save T flag
	mov.l	r3, @-r15

#ifdef HAVE_FPU
	sts.l	fpscr, @-r15
	mov	#8,r3
	swap.w	r3, r3
	lds	r3, fpscr
	fmov.s	fr11, @-r15
	fmov.s	fr10, @-r15
	fmov.s	fr9, @-r15
	fmov.s	fr8, @-r15
	fmov.s	fr7, @-r15
	fmov.s	fr6, @-r15
	fmov.s	fr5, @-r15
	fmov.s	fr4, @-r15
#endif
	sts.l	pr, @-r15
/* Note - The PLT entries have been "optimised" not to use r2.  r2 is used by
   GCC to return the address of large structures, so it should not be
   corrupted here.  This does mean however, that those PLTs does not conform
   to the SH PIC ABI.  That spec says that r0 contains the type of the PLT
   and r2 contains the GOT id.  The GNU Plt version stores the GOT id in r0 and
   ignores the type.  We can easily detect this difference however,
   since the type will always be 0 or 8, and the GOT ids will always be
   greater than or equal to 12.

   Found in binutils/bfd/elf32-sh.c by Stefan Allius <allius@atecom.com>

   Note2 - we also have to preserve r2 on the stack as the call into
   C code (_dl_linux_resolver) will use r2 as a scratch register and we
   need it for the address for returning structures,
   David McCullough <davidm@snapgear.com>.
 */
	mov     #8 ,r5
	cmp/gt  r5, r0
	bt      1f
	mov     r2, r0          ! link map address in r2 (SH PIC ABI)
1:
	mov     r0, r4          ! link map address in r0 (GNUs PLT)
	mova    .LG, r0
	mov.l   .LG, r5
	add     r5, r0
	mov.l   3f, r5
	mov.l   @(r0, r5),r5
	jsr     @r5
	 mov	r1, r5		! Reloc offset

	lds.l	@r15+, pr	! Get register content back

#ifdef HAVE_FPU
	fmov.s	@r15+, fr4
	fmov.s	@r15+, fr5
	fmov.s	@r15+, fr6
	fmov.s	@r15+, fr7
	fmov.s	@r15+, fr8
	fmov.s	@r15+, fr9
	fmov.s	@r15+, fr10
	fmov.s	@r15+, fr11
	lds.l	@r15+, fpscr
#endif

	mov.l	@r15+, r3
	shal	r3		! Load T flag
	mov.l	@r15+, r12
	mov.l	@r15+, r7
	mov.l	@r15+, r6
	mov.l	@r15+, r5
	mov.l	@r15+, r4
	mov.l	@r15+, r3
	jmp	@r0		! Jump to function address
	 mov.l	@r15+, r2	! see Note2 above

	.balign	4

3:
	.long   _dl_linux_resolver@GOT
.LG:
	.long	_GLOBAL_OFFSET_TABLE_
	.size	_dl_linux_resolve, . - _dl_linux_resolve

