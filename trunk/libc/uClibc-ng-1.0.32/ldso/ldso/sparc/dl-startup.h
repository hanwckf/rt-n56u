/* Any assembly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.  See arm/boot1_arch.h for an example of what
 * can be done.
 */

__asm__ ("\
	.text\n\
	.global _start\n\
	.type   _start,%function\n\
	.hidden _start\n\
	.align 32\n\
	.register %g2, #scratch\n\
_start:\n\
	/* Allocate space for functions to drop their arguments. */\n\
	sub	%sp, 6*4, %sp\n\
	/* Pass pointer to argument block to _dl_start. */\n\
	call _dl_start\n\
	add    %sp, 22*4, %o0\n\
	/* FALTHRU */\n\
	.globl  _dl_start_user\n\
	.type   _dl_start_user, @function\n\
_dl_start_user:\n\
  /* Load the PIC register.  */\n\
1:	call    2f\n\
	sethi  %hi(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7\n\
2:	or  %l7, %lo(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7\n\
	add %l7, %o7, %l7\n\
  /* Save the user entry point address in %l0 */\n\
	mov %o0, %l0\n\
  /* See if we were run as a command with the executable file name as an\n\
	 extra leading argument.  If so, adjust the contents of the stack.  */\n\
	sethi   %hi(_dl_skip_args), %g2\n\
	or  %g2, %lo(_dl_skip_args), %g2\n\
	ld  [%l7+%g2], %i0\n\
	ld  [%i0], %i0\n\
	tst %i0\n\
  /* Pass our finalizer function to the user in %g1.  */\n\
	sethi	%hi(_dl_fini), %g1\n\
	or	%g1, %lo(_dl_fini), %g1\n\
	ld	[%l7+%g1], %g1\n\
  /* Jump to the user's entry point and deallocate the extra stack we got.  */\n\
	jmp %l0\n\
	 add    %sp, 6*4, %sp\n\
	.size   _dl_start_user, . - _dl_start_user\n\
	.previous\n\
");

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.  We assume that argc is stored
 * at the word just below the argvp that we return here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long *) ARGS) + 1)

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.
 */
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB) \
switch(ELF_R_TYPE((RELP)->r_info)) { \
	case R_SPARC_32: \
	case R_SPARC_GLOB_DAT: \
		*REL = SYMBOL + (RELP)->r_addend; \
		break; \
	case R_SPARC_JMP_SLOT: \
		REL[1] = 0x03000000 | ((SYMBOL >> 10) & 0x3fffff); \
		REL[2] = 0x81c06000 | (SYMBOL & 0x3ff); \
		break; \
	case R_SPARC_NONE: \
	case R_SPARC_WDISP30: \
		break; \
	case R_SPARC_RELATIVE: \
		*REL += (unsigned int) LOAD + (RELP)->r_addend; \
		break; \
	default: \
		_dl_exit(1); \
}
