/* vi: set sw=4 ts=4: */
/*
 * Architecture specific code used by dl-startup.c
 * Copyright (C) 2005 by Erik Andersen <andersen@codepoet.org>
 */

asm ("\
	.text\n\
	.globl _start\n\
	.type _start,@function\n\
_start:\n\
	move.l %sp, -(%sp)\n\
	jbsr _dl_start\n\
	addq.l #4, %sp\n\
	/* FALLTHRU */\n\
\n\
	.globl _dl_start_user\n\
.type _dl_start_user,@function\n\
_dl_start_user:\n\
	# Save the user entry point address in %a4.\n\
	move.l %d0, %a4\n\
	# See if we were run as a command with the executable file\n\
	# name as an extra leading argument.\n\
	move.l _dl_skip_args(%pc), %d0\n\
	# Pop the original argument count\n\
	move.l (%sp)+, %d1\n\
	# Subtract _dl_skip_args from it.\n\
	sub.l %d0, %d1\n\
	# Adjust the stack pointer to skip _dl_skip_args words.\n\
	lea (%sp, %d0*4), %sp\n\
	# Push back the modified argument count.\n\
	move.l %d1, -(%sp)\n\
	# Pass our finalizer function to the user in %a1.\n\
	lea _dl_fini(%pc), %a1\n\
	# Initialize %fp with the stack pointer.\n\
	move.l %sp, %fp\n\
	# Jump to the user's entry point.\n\
	jmp (%a4)\n\
	.size _dl_start_user, . - _dl_start_user\n\
	.previous");

/* Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.  */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long *) ARGS) + 1)

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, Elf32_Sym *symtab)
{
	switch (ELF32_R_TYPE(rpnt->r_info))
	{
		case R_68K_8:
			*(char *) reloc_addr = symbol_addr + rpnt->r_addend;
			break;
		case R_68K_16:
			*(short *) reloc_addr = symbol_addr + rpnt->r_addend;
			break;
		case R_68K_32:
			*reloc_addr = symbol_addr + rpnt->r_addend;
			break;
		case R_68K_PC8:
			*(char *) reloc_addr = (symbol_addr + rpnt->r_addend
			                       - (unsigned int) reloc_addr);
			break;
		case R_68K_PC16:
			*(short *) reloc_addr = (symbol_addr + rpnt->r_addend
			                        - (unsigned int) reloc_addr);
			break;
		case R_68K_PC32:
			*reloc_addr = (symbol_addr + rpnt->r_addend
			              - (unsigned int) reloc_addr);
			break;
		case R_68K_GLOB_DAT:
		case R_68K_JMP_SLOT:
			*reloc_addr = symbol_addr;
			break;
		case R_68K_RELATIVE:
			*reloc_addr = ((unsigned int) load_addr +
			              (rpnt->r_addend ? : *reloc_addr));
			break;
		default:
			_dl_exit (1);
	}
}
