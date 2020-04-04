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

/* more info taken from glibc/sysdeps/x86_64/dl-trampoline.S */

.text

.global _dl_linux_resolve
.type   _dl_linux_resolve,%function

_dl_linux_resolve:
	subq $56,%rsp
	/* Preserve registers otherwise clobbered. */
	movq %rax,   (%rsp)
	movq %rcx,  8(%rsp)
	movq %rdx, 16(%rsp)
	movq %rsi, 24(%rsp)
	movq %rdi, 32(%rsp)
	movq %r8,  40(%rsp)
	movq %r9,  48(%rsp)

	movq 64(%rsp), %rsi  /* Copy args pushed by PLT in register. */
	movq %rsi, %r11      /* Multiply by 24 */
	addq %r11, %rsi
	addq %r11, %rsi
	shlq $3, %rsi
	movq 56(%rsp), %rdi  /* %rdi: link_map, %rsi: reloc_offset */
	call _dl_linux_resolver       /* Call resolver. */
	movq %rax, %r11      /* Save return value */

	/* Get register content back. */
	movq 48(%rsp), %r9
	movq 40(%rsp), %r8
	movq 32(%rsp), %rdi
	movq 24(%rsp), %rsi
	movq 16(%rsp), %rdx
	movq  8(%rsp), %rcx
	movq   (%rsp), %rax

	addq $72, %rsp       /* Adjust stack(PLT did 2 pushes) */
	jmp *%r11            /* Jump to function address. */

.size _dl_linux_resolve,.-_dl_linux_resolve
