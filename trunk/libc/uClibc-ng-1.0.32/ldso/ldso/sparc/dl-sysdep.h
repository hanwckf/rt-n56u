/*
 * Various assembly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 */

/* Define this if the system uses RELOCA.  */
#define ELF_USES_RELOCA
#include <elf.h>
/*
 * Initialization sequence for a GOT.  For the Sparc, this points to the
 * PLT, and we need to initialize a couple of the slots.  The PLT should
 * look like:
 *
 *		save %sp, -64, %sp
 *		call _dl_linux_resolve
 *		nop
 *		.word implementation_dependent
 */
#define INIT_GOT(GOT_BASE,MODULE) \
{				\
   GOT_BASE[0] = 0x9de3bfc0;  /* save %sp, -64, %sp */	\
   GOT_BASE[1] = 0x40000000 | (((unsigned int) _dl_linux_resolve - (unsigned int) GOT_BASE - 4) >> 2);	\
   GOT_BASE[2] = 0x01000000; /* nop */ 			\
   GOT_BASE[3] = (int) MODULE;					\
}

/* Here we define the magic numbers that this dynamic loader should accept
 * Note that SPARCV9 doesn't use EM_SPARCV9 since the userland is still 32-bit.
 */
#if defined(__sparc_v9__)
#define MAGIC1 EM_SPARC32PLUS
#else
#define MAGIC1 EM_SPARC
#endif

#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "sparc"

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

struct elf_resolve;
unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/*
 * Define this if you want a dynamic loader that works on Solaris.
 */

#ifndef COMPILE_ASM
/* Cheap modulo implementation, taken from arm/ld_sysdep.h. */
static __always_inline unsigned long
sparc_mod(unsigned long m, unsigned long p)
{
	unsigned long i, t, inc;

	i = p;
	t = 0;

	while (!(i & (1 << 31))) {
		i <<= 1;
		t++;
	}

	t--;

	for (inc = t; inc > 2; inc--) {
		i = p << inc;

		if (i & (1 << 31))
			break;

		while (m >= i) {
			m -= i;
			i <<= 1;
			if (i & (1 << 31))
				break;
			if (i < p)
				break;
		}
	}

	while (m >= p)
		m -= p;

	return m;
}

#define do_rem(result, n, base) ((result) = sparc_mod(n, base))
#endif

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_SPARC_JMP_SLOT || (type) == R_SPARC_TLS_DTPMOD32 \
     || (type) == R_SPARC_TLS_DTPOFF32 || (type) == R_SPARC_TLS_TPOFF32) \
    * ELF_RTYPE_CLASS_PLT)			      \
   | (((type) == R_SPARC_COPY) * ELF_RTYPE_CLASS_COPY))

/* The SPARC overlaps DT_RELA and DT_PLTREL.  */
#define ELF_MACHINE_PLTREL_OVERLAP 1

/* We have to do this because elf_machine_{dynamic,load_address} can be
   invoked from functions that have no GOT references, and thus the compiler
   has no obligation to load the PIC register.  */
#define LOAD_PIC_REG(PIC_REG)   \
do {    register Elf32_Addr pc __asm__("o7"); \
        __asm__("sethi %%hi(_GLOBAL_OFFSET_TABLE_-4), %1\n\t" \
              "call 1f\n\t" \
              "add %1, %%lo(_GLOBAL_OFFSET_TABLE_+4), %1\n" \
              "1:\tadd %1, %0, %1" \
              : "=r" (pc), "=r" (PIC_REG)); \
} while (0)

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static __always_inline Elf32_Addr
elf_machine_dynamic (void)
{
	register Elf32_Addr *got __asm__ ("%l7");

	LOAD_PIC_REG (got);

	return *got;
}

/* Return the run-time load address of the shared object.  */
static __always_inline Elf32_Addr
elf_machine_load_address (void)
{
	register Elf32_Addr *pc __asm__ ("%o7"), *got __asm__ ("%l7");

	__asm__ ("sethi %%hi(_GLOBAL_OFFSET_TABLE_-4), %1\n\t"
	       "call 1f\n\t"
	       " add %1, %%lo(_GLOBAL_OFFSET_TABLE_+4), %1\n\t"
	       "call _DYNAMIC\n\t"
	       "call _GLOBAL_OFFSET_TABLE_\n"
	       "1:\tadd %1, %0, %1\n\t" : "=r" (pc), "=r" (got));

	/* got is now l_addr + _GLOBAL_OFFSET_TABLE_
	 *got is _DYNAMIC
	 pc[2]*4 is l_addr + _DYNAMIC - (long)pc - 8
	 pc[3]*4 is l_addr + _GLOBAL_OFFSET_TABLE_ - (long)pc - 12  */
	return (Elf32_Addr) got - *got + (pc[2] - pc[3]) * 4 - 4;
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
