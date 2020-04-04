/*
 * Meta can never use Elf32_Rel relocations.
 *
 * Copyright (C) 2013, Imagination Technologies Ltd.
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#define ELF_USES_RELOCA

#include <elf.h>

/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE)				\
{								\
	GOT_BASE[1] = (unsigned long) MODULE; 			\
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve; 	\
}

/* Maximum unsigned GOT [GS]ETD offset size, ie. 2^(11+2). */
#define GOT_REG_OFFSET 0x2000

/* Defined some magic numbers that this ld.so should accept. */
#define MAGIC1 EM_METAG
#undef  MAGIC2
#define ELF_TARGET "META"

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry);

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.

   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type)                                 \
  ((((type) == R_METAG_JMP_SLOT || (type) == R_METAG_TLS_DTPMOD      \
     || (type) == R_METAG_TLS_DTPOFF || (type) == R_METAG_TLS_TPOFF) \
     * ELF_RTYPE_CLASS_PLT)                                          \
     | (((type) == R_METAG_COPY) * ELF_RTYPE_CLASS_COPY))

static inline Elf32_Addr
elf_machine_dynamic(Elf32_Ehdr *header)
{
	Elf32_Addr *got;

	__asm__ ("MOV %0,A1LbP" : "=r" (got));

	if (header->e_ident[EI_ABIVERSION] >= 1) {
		/* GOT register offset was introduced with ABI v1 */
		got = (Elf32_Addr*)((void*)got - GOT_REG_OFFSET);
	}
	return *got;
}

#define DL_BOOT_COMPUTE_GOT(GOT) \
    ((GOT) = elf_machine_dynamic(header))

static inline Elf32_Addr
elf_machine_load_address(void)
{
	Elf32_Addr addr;
	__asm__ ("MOV  D1Ar1,A1LbP\n"
		 "ADDT D1Ar1,D1Ar1,#HI(__dl_start@GOTOFF)\n"
		 "ADD  D1Ar1,D1Ar1,#LO(__dl_start@GOTOFF)\n"
		 "ADDT D0Ar2,D0Ar2,#HI(__dl_start_addr@GOTOFF)\n"
		 "ADD  D0Ar2,D0Ar2,#LO(__dl_start_addr@GOTOFF)\n"
		 "GETD D0Ar2,[D0Ar2]\n"
		 "SUB  %0,D1Ar1,D0Ar2\n"
		 ".section .data\n"
		 "__dl_start_addr: .long __dl_start\n"
		 ".previous\n"
		 : "=d" (addr) : : "D1Ar1", "D0Ar2");
	return addr;
}

static inline void
elf_machine_relative(Elf32_Addr load_off, const Elf32_Addr rel_addr,
                     Elf32_Word relative_count)
{
	Elf32_Rela *rpnt = (void *)rel_addr;

	--rpnt;
	do {
		Elf32_Addr *const reloc_addr =
			(void *)(load_off + (++rpnt)->r_offset);

		*reloc_addr =  load_off + rpnt->r_addend;
	} while (--relative_count);
}

#define DL_MALLOC_ALIGN 8

#define HAVE_DL_INLINES_H

#define DL_IS_SPECIAL_SEGMENT(EPNT, PPNT) \
  __dl_is_special_segment(EPNT, PPNT)
#define DL_MAP_SEGMENT(EPNT, PPNT, INFILE, FLAGS) \
  __dl_map_segment (EPNT, PPNT, INFILE, FLAGS)

#define DL_CHECK_LIB_TYPE(epnt, piclib, _dl_progname, libname) \
do \
{ \
  ElfW(Phdr) *ppnt_; \
  char *header_ = (char *)epnt;				   \
  ppnt_ = (ElfW(Phdr) *)(intptr_t) & header_[epnt->e_phoff]; \
  if (ppnt_->p_vaddr >= 0x80000000 && \
      ppnt_->p_vaddr < 0x82060000) \
    (piclib) = 2; \
  if (ppnt_->p_vaddr >= 0xe0200000 && \
      ppnt_->p_vaddr < 0xe0260000) \
    (piclib) = 2; \
} \
while (0)

#define _DL_PREAD(FD, BUF, SIZE, OFFSET) \
  (_dl_pread((FD), (BUF), (SIZE), (OFFSET)))
