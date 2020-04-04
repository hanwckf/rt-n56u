/*
   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#define ELF_USES_RELOCA

#include <elf.h>

/* Initialise the GOT */
#define INIT_GOT(GOT_BASE,MODULE)				\
do {								\
	GOT_BASE[1] = (unsigned long) MODULE;			\
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve;	\
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_MICROBLAZE
#undef  MAGIC2
/* Used for error messages */
#define ELF_TARGET "microblaze"

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

struct elf_resolve;
unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

#define elf_machine_type_class(type) \
  (((type) == R_MICROBLAZE_JUMP_SLOT || \
    (type) == R_MICROBLAZE_TLSDTPREL32 || \
    (type) == R_MICROBLAZE_TLSDTPMOD32 || \
    (type) == R_MICROBLAZE_TLSTPREL32) \
    * ELF_RTYPE_CLASS_PLT \
   | ((type) == R_MICROBLAZE_COPY) * ELF_RTYPE_CLASS_COPY)

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static __always_inline Elf32_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
  Elf32_Addr got_entry_0;
  __asm__ __volatile__(
    "lwi %0,r20,0"
    :"=r"(got_entry_0)
    );
  return got_entry_0;
}

/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
  /* Compute the difference between the runtime address of _DYNAMIC as seen
     by a GOTOFF reference, and the link-time address found in the special
     unrelocated first GOT entry.  */

  Elf32_Addr dyn;
  __asm__ __volatile__ (
    "addik %0,r20,_DYNAMIC@GOTOFF"
    : "=r"(dyn)
    );
  return dyn - elf_machine_dynamic ();
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
