/*
 * Architecture specific code used by dl-startup.c
 *
 * Copyright (C) 2005-2007 Atmel Corporation
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* This is the library loader's main entry point. Let _dl_boot2 do its
 * initializations and jump to the application's entry point
 * afterwards. */
__asm__("	.text\n"
	"	.global	_start\n"
	"	.type	_start,@function\n"
	"	.hidden	_start\n"
	"_start:\n"
	/* All arguments are on the stack initially */
	"	mov	r12, sp\n"
	"	rcall	_dl_start\n"
	/* Returns user entry point in r12. Save it. */
	"	mov	r0, r12\n"
	/* We're PIC, so get the Global Offset Table */
	"	lddpc	r6, .L_GOT\n"
	".L_RGOT:\n"
	"	rsub	r6, pc\n"
	/* Adjust argc and argv according to _dl_skip_args */
	"	ld.w	r1, r6[_dl_skip_args@got]\n"
	"	ld.w	r1, r1[0]\n"
	"	ld.w	r2, sp++\n"
	"	sub	r2, r1\n"
	"	add	sp, sp, r1 << 2\n"
	"	st.w	--sp, r2\n"
	/* Load the finalizer function */
	"	ld.w	r12, r6[_dl_fini@got]\n"
	/* Jump to the user's entry point */
	"	mov	pc, r0\n\n"

	"	.align	2\n"
	".L_GOT:"
	"	.long	.L_RGOT - _GLOBAL_OFFSET_TABLE_\n"
	"	.size	_start, . - _start\n"
	"	.previous\n");

/* Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here. */
#define GET_ARGV(ARGVP, ARGS) ARGVP = ((unsigned long *)ARGS + 1)


/* We can't call functions before the GOT has been initialized */
#define NO_FUNCS_BEFORE_BOOTSTRAP

/*
 * Relocate the GOT during dynamic loader bootstrap.  This will add
 * the load address to all entries in the GOT, which is necessary
 * because the linker doesn't generate R_AVR32_RELATIVE relocs for the
 * GOT.
 */
static __always_inline
void PERFORM_BOOTSTRAP_GOT(struct elf_resolve *tpnt)
{
	Elf32_Addr i, nr_got;
	register Elf32_Addr *__r6 __asm__("r6");
	Elf32_Addr *got = __r6;

	nr_got = tpnt->dynamic_info[DT_AVR32_GOTSZ_IDX] / sizeof(*got);
	for (i = 2; i < nr_got; i++)
		got[i] += tpnt->loadaddr;
}

#define PERFORM_BOOTSTRAP_GOT(tpnt) PERFORM_BOOTSTRAP_GOT(tpnt)

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
			     unsigned long symbol_addr,
			     unsigned long load_addr, Elf32_Sym *symtab)
{
	switch(ELF_R_TYPE(rpnt->r_info)) {
	case R_AVR32_NONE:
		break;
	case R_AVR32_GLOB_DAT:
	case R_AVR32_JMP_SLOT:
		*reloc_addr = symbol_addr;
		break;
	case R_AVR32_RELATIVE:
		SEND_STDERR_DEBUG("Applying RELATIVE relocation: ");
		SEND_ADDRESS_STDERR_DEBUG(load_addr, 0);
		SEND_STDERR_DEBUG(" + ");
		SEND_ADDRESS_STDERR_DEBUG(rpnt->r_addend, 1);
		*reloc_addr = load_addr + rpnt->r_addend;
		break;
	default:
		SEND_STDERR("BOOTSTRAP_RELOC: unhandled reloc_type ");
		SEND_NUMBER_STDERR(ELF_R_TYPE(rpnt->r_info), 1);
		SEND_STDERR("REL, SYMBOL, LOAD: ");
		SEND_ADDRESS_STDERR(reloc_addr, 0);
		SEND_STDERR(", ");
		SEND_ADDRESS_STDERR(symbol_addr, 0);
		SEND_STDERR(", ");
		SEND_ADDRESS_STDERR(load_addr, 1);
		_dl_exit(1);
	}
}
