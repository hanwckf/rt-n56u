/*
 * Architecture specific code used by dl-startup.c
 */

/* This code fixes the stack pointer so that the dynamic linker
 * can find argc, argv and auxvt (Auxillary Vector Table).  */
#ifdef __arch_v32

__asm__(""					\
"	.text\n"			\
"	.globl _start\n"		\
"	.type _start,@function\n"	\
"_start:\n"				\
"	move.d	$sp,$r10\n"		\
"	lapc	_dl_start,$r9\n"	\
"	jsr	$r9\n"			\
"	nop\n"				\
"	moveq	0,$r8\n"		\
"	jump	$r10\n"			\
"	move	$r8,$srp\n"		\
"	.size _start,.-_start\n"	\
"	.previous\n"			\
);

#else

__asm__(""					\
"	.text\n"			\
"	.globl _start\n"		\
"	.type _start,@function\n"	\
"_start:\n"				\
"	move.d	$sp,$r10\n"		\
"	move.d	$pc,$r9\n"		\
"	add.d	_dl_start - ., $r9\n"	\
"	jsr	$r9\n"			\
"	moveq	0,$r8\n"		\
"	move	$r8,$srp\n"		\
"	jump	$r10\n"			\
"	.size _start,.-_start\n"	\
"	.previous\n"			\
);

#endif /* __arch_v32 */

/* Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.  */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long *) ARGS)+1)


/* We can't call functions earlier in the dl startup process */
#define NO_FUNCS_BEFORE_BOOTSTRAP


/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, Elf32_Sym *symtab)
{
	switch (ELF_R_TYPE(rpnt->r_info)) {
		case R_CRIS_GLOB_DAT:
		case R_CRIS_JUMP_SLOT:
		case R_CRIS_32:
			*reloc_addr = symbol_addr;
			break;
		case R_CRIS_16_PCREL:
			*(short *) *reloc_addr = symbol_addr + rpnt->r_addend - *reloc_addr - 2;
			break;
		case R_CRIS_32_PCREL:
			*reloc_addr = symbol_addr + rpnt->r_addend - *reloc_addr - 4;
			break;
		case R_CRIS_NONE:
			break;
		case R_CRIS_RELATIVE:
			*reloc_addr = load_addr + rpnt->r_addend;
			break;
		default:
			_dl_exit(1);
			break;
	}
}
