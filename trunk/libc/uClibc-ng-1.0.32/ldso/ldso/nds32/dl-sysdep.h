/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Define this if the system uses RELOCA.  */
#define ELF_USES_RELOCA
#include <elf.h>
/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE) \
{				\
  GOT_BASE[2] = (unsigned long) _dl_linux_resolve; \
  GOT_BASE[1] = (unsigned long) MODULE; \
}

static __inline__ unsigned long nds32_modulus(unsigned long m, unsigned long p)
{
	unsigned long i,t,inc;
	i=p; t=0;
	while (!(i&(1<<31))) {
		i<<=1;
		t++;
	}
	t--;
	for (inc=t;inc>2;inc--) {
		i=p<<inc;
		if (i&(1<<31))
			break;
		while (m>=i) {
			m-=i;
			i<<=1;
			if (i&(1<<31))
				break;
			if (i<p)
				break;
		}
	}
	while (m>=p) {
		m-=p;
	}
	return m;
}
#define do_rem(result, n, base) ((result) = nds32_modulus(n, base))

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_NDS32
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "NDS32"

struct elf_resolve;
unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_NDS32_JMP_SLOT || (type) == R_NDS32_TLS_TPOFF \
      || (type) == R_NDS32_TLS_DESC) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_NDS32_COPY) * ELF_RTYPE_CLASS_COPY))

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  We used to use the PIC register to do this
   without a constant pool reference, but GCC 4.2 will use a pseudo-register
   for the PIC base, so it may not be in r10.  */
static __inline__ Elf32_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
	Elf32_Addr link_addr;
	__asm__ ( "l.w %0, _GLOBAL_OFFSET_TABLE_@GOTOFF": "=r" (link_addr) );
	return link_addr;
}

/* Return the run-time load address of the shared object.  */
static __inline__ Elf32_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
  /* It doesn't matter what variable this is, the reference never makes
     it to assembly.  We need a dummy reference to some global variable
     via the GOT to make sure the compiler initialized %ebx in time.  */

	Elf32_Addr addr;
	__asm__ ("la	%0, _DYNAMIC@GOTOFF\n" : "=r" (addr) );
	return addr - elf_machine_dynamic();
}

static __inline__ void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	 Elf32_Rela * rpnt = (void *) rel_addr;
	--rpnt;
	do {
		Elf32_Addr *const reloc_addr = (void *) (load_off + (++rpnt)->r_offset);

		*reloc_addr = load_off + rpnt->r_addend;
	} while (--relative_count);
}
