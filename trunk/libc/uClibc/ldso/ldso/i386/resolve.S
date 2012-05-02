/*
 * This function is _not_ called directly.  It is jumped to (so no return
 * address is on the stack) when attempting to use a symbol that has not yet
 * been resolved.  The first time a jump symbol (such as a function call inside
 * a shared library) is used (before it gets resolved) it will jump here to
 * _dl_linux_resolve.  When we get called the stack looks like this:
 *	reloc_entry
 *	tpnt
 *
 * This function saves all the registers, puts a copy of reloc_entry and tpnt
 * on the stack (as function arguments) then make the function call
 * _dl_linux_resolver(tpnt, reloc_entry).  _dl_linux_resolver() figures out
 * where the jump symbol is _really_ supposed to have jumped to and returns
 * that to us.  Once we have that, we overwrite tpnt with this fixed up
 * address. We then clean up after ourselves, put all the registers back how we
 * found them, then we jump to where the fixed up address, which is where the
 * jump symbol that got us here really wanted to jump to in the first place.
 * found them, then we jump to the fixed up address, which is where the jump
 * symbol that got us here really wanted to jump to in the first place.
 *  -Erik Andersen
 */

.text
.align 4

.globl _dl_linux_resolve
.type	_dl_linux_resolve,@function

_dl_linux_resolve:
	pusha				/* preserve all regs */
	lea	0x20(%esp),%eax		/* eax = tpnt and reloc_entry params */
	pushl	4(%eax)			/* push copy of reloc_entry param */
	pushl	(%eax)			/* push copy of tpnt param */

#ifdef __PIC__
	call	.L24
.L24:
	popl	%ebx
	addl	$_GLOBAL_OFFSET_TABLE_+[.-.L24],%ebx
	movl _dl_linux_resolver@GOT(%ebx),%ebx	/* eax = resolved func */
	call *%ebx
#else
	call _dl_linux_resolver
#endif
	movl	%eax,0x28(%esp)		/* store func addr over original
					 * tpnt param */
	addl	$0x8,%esp		/* remove copy parameters */
	popa				/* restore regs */
	ret	$4			/* jump to func removing original
					 * reloc_entry param from stack */
.LFE2:
	.size _dl_linux_resolve,.LFE2-_dl_linux_resolve
