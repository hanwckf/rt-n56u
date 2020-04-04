/*
 * Various assembly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 * Copyright (C) 2005 by Erik Andersen <andersen@codepoet.org>
 */

/* Define this if the system uses RELOCA.  */
#define ELF_USES_RELOCA
#include <elf.h>
/* Initialization sequence for a GOT.  */
#define INIT_GOT(GOT_BASE,MODULE) \
do { \
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve; \
	GOT_BASE[1] = (unsigned long) (MODULE); \
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_68K
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "m68k"

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

struct elf_resolve;
extern unsigned long _dl_linux_resolver (struct elf_resolve *, int);

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.
   ELF_RTYPE_CLASS_COPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_68K_JMP_SLOT	     \
     || (type) == R_68K_TLS_DTPMOD32 \
     || (type) == R_68K_TLS_DTPREL32 \
     || (type) == R_68K_TLS_TPREL32) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_68K_COPY) * ELF_RTYPE_CLASS_COPY))

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static __always_inline Elf32_Addr
elf_machine_dynamic (void)
{
	Elf32_Addr got;

	__asm__ ("move.l _DYNAMIC@GOT.w(%%a5), %0"
			: "=a" (got));
	return got;
}

#ifdef __mcoldfire__
#define PCREL_OP(OP, SRC, DST, TMP, PC) \
  "move.l #" SRC " - ., " TMP "\n\t" OP " (-8, " PC ", " TMP "), " DST
#else
#define PCREL_OP(OP, SRC, DST, TMP, PC) \
  OP " " SRC "(" PC "), " DST
#endif

/* Return the run-time load address of the shared object.  */
static __always_inline Elf32_Addr
elf_machine_load_address (void)
{
	Elf32_Addr addr;
	__asm__ (PCREL_OP ("lea", "_dl_start", "%0", "%0", "%%pc") "\n\t"
			"sub.l _dl_start@GOT.w(%%a5), %0"
			: "=a" (addr));
	return addr;
}

static __always_inline void
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
