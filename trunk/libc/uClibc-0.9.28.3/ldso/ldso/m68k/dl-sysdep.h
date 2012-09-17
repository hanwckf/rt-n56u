/* vi: set sw=4 ts=4: */
/*
 * Various assmbly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code. 
 * Copyright (C) 2005 by Erik Andersen <andersen@codepoet.org>
 */

/* Define this if the system uses RELOCA.  */
#define ELF_USES_RELOCA
#include <elf.h>
/* Initialization sequence for a GOT.  */
#define INIT_GOT(GOT_BASE,MODULE) \
do { \
	GOT_BASE[2] = (int) _dl_linux_resolve; \
	GOT_BASE[1] = (int) (MODULE); \
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_68K
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "m68k"

struct elf_resolve;
extern unsigned int _dl_linux_resolver (struct elf_resolve *, int);

/* 4096 bytes alignment */
#define PAGE_ALIGN 0xfffff000
#define ADDR_ALIGN 0xfff
#define OFFS_ALIGN 0x7ffff000

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_68K_JMP_SLOT) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_68K_COPY) * ELF_RTYPE_CLASS_COPY))

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline Elf32_Addr
elf_machine_dynamic (void)
{
	register Elf32_Addr *got asm ("%a5");
	return *got;
}


/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
	Elf32_Addr addr;
	asm ("lea _dl_start(%%pc), %0\n\t"
	     "sub.l _dl_start@GOT.w(%%a5), %0"
	     : "=a" (addr));
	return addr;
}

static inline void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	 Elf32_Rela * rpnt = (void *)rel_addr;
	--rpnt;
	do {
		Elf32_Addr *const reloc_addr = (void *) (load_off + (++rpnt)->r_offset);

		*reloc_addr = load_off + rpnt->r_addend;
	} while (--relative_count);
}
