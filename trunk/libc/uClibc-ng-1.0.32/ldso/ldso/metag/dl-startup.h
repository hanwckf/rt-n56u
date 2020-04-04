/*
 * Copyright (C) 2013 Imagination Technologies Ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

/*
 * This code fixes the stack pointer so that the dynamic linker
 * can find argc, argv and auxvt (Auxillary Vector Table).
 */

__asm__ (
"       .text\n"
"       .global __start\n"
"       .type   __start,@function\n"
"	.hidden __start\n"
"_start:\n"
"__start:\n"
"       MSETL   [A0StP++],D0Ar4,D0Ar2\n"
"       MOV     D1Ar1,D0Ar2\n"
"       CALLR   D1RtP,__dl_start\n"
"       GETL    D0Ar2,D1Ar1,[A0StP+#-(1*8)]\n"
"       GETL    D0Ar4,D1Ar3,[A0StP+#-(2*8)]\n"
"       ADDT    A1LbP,CPC1,#HI(__GLOBAL_OFFSET_TABLE__)\n"
"       ADD     A1LbP,A1LbP,#LO(__GLOBAL_OFFSET_TABLE__+4)\n"
"       ADDT    A1LbP,A1LbP,#HI(__dl_fini@GOTOFF)\n"
"       ADD     A1LbP,A1LbP,#LO(__dl_fini@GOTOFF)\n"
"       MOV     D0Ar4, A1LbP\n"
"       SUB     A0StP,A0StP,#(2*8)\n"
"       MOV     PC,D0Re0\n"
"       .size __start,.-__start\n"
"       .previous\n"
);


/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.
 */

#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long *) ARGS))


/* Handle relocation of the symbols in the dynamic loader. */
static inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, Elf32_Sym *symtab)
{
	switch (ELF32_R_TYPE(rpnt->r_info)) {
		case R_METAG_GLOB_DAT:
		case R_METAG_JMP_SLOT:
		case R_METAG_ADDR32:
			*reloc_addr = symbol_addr;
			break;
		case R_METAG_RELATIVE:
			*reloc_addr = load_addr + rpnt->r_addend;
			break;
		case R_METAG_RELBRANCH:
			*reloc_addr = symbol_addr + rpnt->r_addend - *reloc_addr - 4;
			break;
		case R_METAG_NONE:
			break;
		default:
			_dl_exit(1);
			break;
	}
}
