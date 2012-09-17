/* vi: set sw=4 ts=4: */
/*
 * Various assembly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 */

/* Define this if the system uses RELOCA.  */
#undef ELF_USES_RELOCA
#include <elf.h>
/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE)							\
do {														\
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve;		\
	GOT_BASE[1] = (unsigned long) MODULE;					\
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_386
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "386"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_386_JMP_SLOT || (type) == R_386_TLS_DTPMOD32		      \
     || (type) == R_386_TLS_DTPOFF32 || (type) == R_386_TLS_TPOFF32	      \
     || (type) == R_386_TLS_TPOFF) * ELF_RTYPE_CLASS_PLT)				  \
   | (((type) == R_386_COPY) * ELF_RTYPE_CLASS_COPY))

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static __always_inline Elf32_Addr elf_machine_dynamic (void) attribute_unused;
static __always_inline Elf32_Addr
elf_machine_dynamic (void)
{
	register Elf32_Addr *got __asm__ ("%ebx");
	return *got;
}


/* Return the run-time load address of the shared object.  */
static __always_inline Elf32_Addr elf_machine_load_address (void) attribute_unused;
static __always_inline Elf32_Addr
elf_machine_load_address (void)
{
	/* It doesn't matter what variable this is, the reference never makes
	   it to assembly.  We need a dummy reference to some global variable
	   via the GOT to make sure the compiler initialized %ebx in time.  */
	Elf32_Addr addr;
	int tmp;
	__asm__ ("leal _dl_start@GOTOFF(%%ebx), %0\n"
	     "subl _dl_start@GOT(%%ebx), %0"
	     : "=r" (addr) : "m" (tmp) : "cc");
	return addr;
}

static __always_inline void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	Elf32_Rel * rpnt = (void *) rel_addr;
	--rpnt;
	do {
		Elf32_Addr *const reloc_addr = (void *) (load_off + (++rpnt)->r_offset);

		*reloc_addr += load_off;
	} while (--relative_count);
}
