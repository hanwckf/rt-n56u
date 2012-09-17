/* Any assmbly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.
 */

asm("" \
"	.section .text..SHmedia32,\"ax\"\n"				\
"	.globl _start\n"						\
"	.type _start, @function\n"					\
"	.align 5\n"							\
"_start:\n"								\
"	! Set r12 to point to GOT\n"					\
"	movi	(((datalabel _GLOBAL_OFFSET_TABLE_-(.LZZZ3-.)) >> 16) & 0xffff), r12\n"	\
"	shori	((datalabel _GLOBAL_OFFSET_TABLE_-(.LZZZ3-.)) & 0xffff), r12\n"		\
".LZZZ3:\n"								\
"	ptrel/u	r12, tr0\n"						\
"	gettr	tr0, r12	! GOT address\n"			\
"	add	r18, r63, r11	! save return address - needed?\n"	\
"	add	r15, r63, r2	! arg = stack pointer\n"		\
"	pt	_dl_start, tr0	! should work even if PIC\n"		\
"	blink	tr0, r18	! call _dl_start - user EP is in r2\n"	\
"	add	r2, r63, r28\n"						\
"	movi	(((_dl_fini@GOT) >> 16) & 0xffff), r1\n"		\
"	shori	((_dl_fini@GOT) & 0xffff), r1\n"			\
"	ldx.l	r1, r12, r2\n"						\
"	add	r11, r63, r18\n"					\
"	ptabs/l r28, tr0\n"						\
"	blink	tr0, r63\n"						\
"	.size	_start,.-_start\n"
"	.previous\n"
);

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long *)ARGS)+1)

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.  RELP is the relocation that we
 * are performing, REL is the pointer to the address we are relocating.
 * SYMBOL is the symbol involved in the relocation, and LOAD is the
 * load address.
 */

#include <elf.h>

#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB)		\
	const unsigned int r_type = ELF32_R_TYPE((RELP)->r_info);	\
	int lsb = !!((SYMTAB)->st_other & STO_SH5_ISA32);		\
									\
	switch (r_type)	{						\
	case R_SH_REL32:						\
		*(REL)  = (SYMBOL) + (RELP)->r_addend			\
			    - (unsigned long)(REL);			\
		break;							\
	case R_SH_DIR32:						\
	case R_SH_GLOB_DAT:						\
	case R_SH_JMP_SLOT:						\
		*(REL)  = ((SYMBOL) + (RELP)->r_addend) | lsb;		\
		break;							\
	case R_SH_RELATIVE:						\
		*(REL)  = (LOAD) + (RELP)->r_addend;			\
		break;							\
	case R_SH_RELATIVE_LOW16:					\
	case R_SH_RELATIVE_MEDLOW16:					\
	{								\
		unsigned long word, value;				\
									\
		word = (unsigned long)(REL) & ~0x3fffc00;		\
		value = (LOAD) + (RELP)->r_addend;			\
									\
		if (r_type == R_SH_RELATIVE_MEDLOW16)			\
			value >>= 16;					\
									\
		word |= (value & 0xffff) << 10;				\
		*(REL)	= word;						\
		break;							\
	}								\
	case R_SH_IMM_LOW16:						\
	case R_SH_IMM_MEDLOW16:						\
	{								\
		unsigned long word, value;				\
									\
		word = (unsigned long)(REL) & ~0x3fffc00;		\
		value = ((SYMBOL) + (RELP)->r_addend) | lsb;		\
									\
		if (r_type == R_SH_IMM_MEDLOW16)			\
			value >>= 16;					\
									\
		word |= (value & 0xffff) << 10;				\
		*(REL)	= word;						\
		break;							\
	}								\
	case R_SH_IMM_LOW16_PCREL:					\
	case R_SH_IMM_MEDLOW16_PCREL:					\
	{								\
		unsigned long word, value;				\
									\
		word = (unsigned long)(REL) & ~0x3fffc00;		\
		value = (SYMBOL) + (RELP)->r_addend			\
			  - (unsigned long)(REL);			\
									\
		if (r_type == R_SH_IMM_MEDLOW16_PCREL)			\
			value >>= 16;					\
									\
		word |= (value & 0xffff) << 10;				\
		*(REL)	= word;						\
		break;							\
	}								\
	case R_SH_NONE:							\
		break;							\
	default:							\
		_dl_exit(1);						\
	}
