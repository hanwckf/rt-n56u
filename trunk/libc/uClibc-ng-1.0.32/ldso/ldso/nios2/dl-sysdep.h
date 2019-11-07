/* Use reloca */
#define ELF_USES_RELOCA

#include <elf.h>

/* Initialise the GOT */
#define INIT_GOT(GOT_BASE,MODULE)					\
do {									\
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve;		\
	GOT_BASE[1] = (unsigned long) MODULE;				\
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */

#define MAGIC1 EM_ALTERA_NIOS2
#undef  MAGIC2
/* Used for error messages */
#define ELF_TARGET "nios2"

struct elf_resolve;
unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_COPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type)				\
  ((((type) == R_NIOS2_JUMP_SLOT				\
     || (type) == R_NIOS2_TLS_DTPMOD				\
     || (type) == R_NIOS2_TLS_DTPREL				\
     || (type) == R_NIOS2_TLS_TPREL) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_NIOS2_COPY) * ELF_RTYPE_CLASS_COPY))

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  */
static inline Elf32_Addr
elf_machine_dynamic (void)
{
  Elf32_Addr *dynamic;
  int tmp;
  __asm__ ("nextpc\t%0\n\t"
       "1: movhi\t%1, %%hiadj(_GLOBAL_OFFSET_TABLE_ - 1b)\n\t"
       "addi\t%1, %1, %%lo(_GLOBAL_OFFSET_TABLE_ - 1b)\n\t"
       "add\t%0, %0, %1\n"
       : "=r" (dynamic), "=r" (tmp));
  return *dynamic;
}

/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
  Elf32_Addr result;
  int tmp;
  __asm__ ("nextpc\t%0\n\t"
       "1: movhi\t%1, %%hiadj(1b)\n\t"
       "addi\t%1, %1, %%lo(1b)\n\t"
       "sub\t%0, %0, %1\n"
       : "=r" (result), "=r" (tmp));
  return result;
}

static __always_inline void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	Elf32_Rel * rpnt = (void *) rel_addr;
	do {
		Elf32_Addr *const reloc_addr = (void *) (load_off + (rpnt)->r_offset);

		*reloc_addr += load_off;
	} while (--relative_count);
}

