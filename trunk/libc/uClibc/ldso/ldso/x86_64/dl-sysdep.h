/* vi: set sw=4 ts=4: */
/* yoinked from glibc/sysdeps/x86_64/dl-machine.h */
/* Machine-dependent ELF dynamic relocation inline functions.  x86-64 version.
   Copyright (C) 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Define this if the system uses RELOCA.  */
#define ELF_USES_RELOCA
#include <elf.h>
#include <link.h>
/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE)							\
do {														\
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve;		\
	GOT_BASE[1] = (unsigned long) MODULE;					\
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_X86_64
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "x86_64"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/* 4096 bytes alignment */
#define PAGE_ALIGN 0xfffff000
#define ADDR_ALIGN 0xfff
#define OFFS_ALIGN 0x7ffff000

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_X86_64_JUMP_SLOT) * ELF_RTYPE_CLASS_PLT) \
   | (((type) == R_X86_64_COPY) * ELF_RTYPE_CLASS_COPY))

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static __always_inline Elf64_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
  Elf64_Addr addr;

  /* This works because we have our GOT address available in the small PIC
     model.  */
  addr = (Elf64_Addr) &_DYNAMIC;

  return addr;
}


/* Return the run-time load address of the shared object.  */
static __always_inline Elf64_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
  register Elf64_Addr addr, tmp;

  /* The easy way is just the same as on x86:
       leaq _dl_start, %0
       leaq _dl_start(%%rip), %1
       subq %0, %1
     but this does not work with binutils since we then have
     a R_X86_64_32S relocation in a shared lib.

     Instead we store the address of _dl_start in the data section
     and compare it with the current value that we can get via
     an RIP relative addressing mode.  */

  __asm__ ("movq 1f(%%rip), %1\n"
       "0:\tleaq _dl_start(%%rip), %0\n\t"
       "subq %1, %0\n\t"
       ".section\t.data\n"
       "1:\t.quad _dl_start\n\t"
       ".previous\n\t"
       : "=r" (addr), "=r" (tmp) : : "cc");

  return addr;
}

static __always_inline void
elf_machine_relative(Elf64_Addr load_off, const Elf64_Addr rel_addr,
                     Elf64_Word relative_count)
{
	Elf64_Rela *rpnt = (Elf64_Rela*)rel_addr;
	--rpnt;
	do {
		Elf64_Addr *const reloc_addr = (Elf64_Addr*)(load_off + (++rpnt)->r_offset);

		*reloc_addr = load_off + rpnt->r_addend;
	} while (--relative_count);
}
