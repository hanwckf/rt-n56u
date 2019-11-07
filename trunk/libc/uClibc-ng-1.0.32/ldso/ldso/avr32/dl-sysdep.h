/*
 * Various assembly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 *
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Define this if the system uses RELOCA. */
#define ELF_USES_RELOCA

#include <elf.h>

#define ARCH_NUM 1
#define DT_AVR32_GOTSZ_IDX	(DT_NUM + OS_NUM)

#define ARCH_DYNAMIC_INFO(dpnt, dynamic, debug_addr)			\
	do {								\
		if (dpnt->d_tag == DT_AVR32_GOTSZ)			\
			dynamic[DT_AVR32_GOTSZ_IDX] = dpnt->d_un.d_val;	\
	} while (0)

/* Initialization sequence for the application/library GOT. */
#define INIT_GOT(GOT_BASE,MODULE)					\
	do {								\
		unsigned long _i, _nr_got;				\
									\
		GOT_BASE[0] = (unsigned long) _dl_linux_resolve;	\
		GOT_BASE[1] = (unsigned long) MODULE;			\
									\
		/* Add load address displacement to all GOT entries */	\
		_nr_got = MODULE->dynamic_info[DT_AVR32_GOTSZ_IDX] / 4;	\
		for (_i = 2; _i < _nr_got; _i++)				\
			GOT_BASE[_i] += (unsigned long)MODULE->loadaddr;	\
	} while (0)

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_AVR32
#undef MAGIC2

/* Used for error messages */
#define ELF_TARGET "AVR32"

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

unsigned long _dl_linux_resolver(unsigned long got_offset, unsigned long *got);

#define elf_machine_type_class(type)				\
	((type == R_AVR32_JMP_SLOT) * ELF_RTYPE_CLASS_PLT)

/* AVR32 doesn't need any COPY relocs */
#define DL_NO_COPY_RELOCS

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static __always_inline Elf32_Addr
elf_machine_dynamic (void)
{
	register Elf32_Addr *got __asm__("r6");
	return *got;
}

extern char __dl_start[] __asm__("_dl_start");

/* Return the run-time load address of the shared object.  */
static __always_inline Elf32_Addr
elf_machine_load_address (void)
{
	Elf32_Addr got_addr = (Elf32_Addr) &__dl_start;
	Elf32_Addr pcrel_addr;

	__asm__("	lddpc	%0, 2f\n"
	       "1:	add	%0, pc\n"
	       "	rjmp	3f\n"
	       "	.align	2\n"
	       "2:	.long	_dl_start - 1b\n"
	       "3:\n"
	       : "=r"(pcrel_addr) : : "cc");

	return pcrel_addr - got_addr;
}

/*
 * Perform any RELATIVE relocations specified by DT_RELCOUNT.
 * Currently, we don't use that tag, but we might in the future as
 * this would reduce the startup time somewhat (although probably not by much).
 */
static __always_inline void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	Elf32_Rela *rpnt = (void *)rel_addr;

	do {
		Elf32_Addr *reloc_addr;
		reloc_addr = (void *)(load_off + (rpnt++)->r_offset);
		*reloc_addr = load_off + rpnt->r_addend;
	} while (--relative_count);
}
