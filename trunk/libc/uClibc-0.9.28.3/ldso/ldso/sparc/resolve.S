/*
 * These are various helper routines that are needed to run an ELF image.
 */

.text
.align 16

.globl _dl_linux_resolve
.type  _dl_linux_resolve,#function
_dl_linux_resolve:
	/*
	 * Call the resolver - pass the address of the PLT so that we can
	 * figure out which module we are in.
	 */
	mov %o7,%o1
	call _dl_linux_resolver
	mov %g1,%o0

	jmpl %o0,%o7
	restore
.LFE2:
	.size _dl_linux_resolve,.LFE2-_dl_linux_resolve
