/* elf reloc code for the or1k platform, based on glibc 2.3.6, dl-machine.h */

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
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

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

#define MAGIC1 EM_OR1K
#undef  MAGIC2
/* Used for error messages */
#define ELF_TARGET "or1k"

#define elf_machine_type_class(type) \
  (((type) == R_OR1K_JMP_SLOT) * ELF_RTYPE_CLASS_PLT \
   | ((type) == R_OR1K_COPY) * ELF_RTYPE_CLASS_COPY)

static inline Elf32_Addr *
or1k_get_got (void)
{
	Elf32_Addr *got;
	Elf32_Addr linkreg;
	__asm__("l.ori   %0, r9, 0\n"
		"l.jal	.LPC1\n"
#ifndef __OR1K_NODELAY__
		"l.nop\n"
#endif
	".LPC1:\n"
		"l.movhi	%1, gotpchi(_GLOBAL_OFFSET_TABLE_+(.-.LPC1))\n"
		"l.ori	%1, %1, gotpclo(_GLOBAL_OFFSET_TABLE_+(.-.LPC1))\n"
		"l.add	%1, %1, r9\n"
		"l.ori	r9, %0, 0\n"
		: "=r" (linkreg), "=r" (got));
	return got;
}

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT. */
static inline Elf32_Addr
elf_machine_dynamic (void)
{
  Elf32_Addr *got = or1k_get_got();
  return *got;
}


/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
  /* Compute the difference between the runtime address of _DYNAMIC as seen
     by a GOTOFF reference, and the link-time address found in the special
     unrelocated first GOT entry.  */
  Elf32_Addr dyn;
  Elf32_Addr *got = or1k_get_got();

  __asm__ __volatile__ (
    "l.movhi %0, gotoffhi(_DYNAMIC);"
    "l.ori %0, %0, gotofflo(_DYNAMIC);"
    "l.add %0, %0, %1;"
    : "=r"(dyn), "=r"(got)
    );
  return dyn - *got;
}



static __always_inline void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	Elf32_Rela * rpnt = (void *) rel_addr;
	--rpnt;
	do {
		Elf32_Addr *const reloc_addr = (void *) (load_off +
							 (++rpnt)->r_offset);

		*reloc_addr += load_off;
	} while (--relative_count);
}
