/* vi: set sw=4 ts=4: */
/*
 * Various assmbly language/system dependent  hacks that are required
 * so that we can minimize the amount of platform specific code.
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 */

/* Define this if the system uses RELOCA.  */
#undef ELF_USES_RELOCA
#include <elf.h>
/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE) \
{				\
  GOT_BASE[2] = (unsigned long) _dl_linux_resolve; \
  GOT_BASE[1] = (unsigned long) MODULE; \
}

static inline unsigned long arm_modulus(unsigned long m, unsigned long p)
{
	unsigned long i,t,inc;
	i=p; t=0;
	while(!(i&(1<<31))) {
		i<<=1;
		t++;
	}
	t--;
	for(inc=t;inc>2;inc--) {
		i=p<<inc;
		if(i&(1<<31))
			break;
		while(m>=i) {
			m-=i;
			i<<=1;
			if(i&(1<<31))
				break;
			if(i<p)
				break;
		}
	}
	while(m>=p) {
		m-=p;
	}
	return m;
}
#define do_rem(result, n, base) ((result) = arm_modulus(n, base))
#define do_div_10(result, remain) ((result) = (((result) - (remain)) / 2) * -(-1ul / 5ul))

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_ARM
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "ARM"

struct elf_resolve;
unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/* 4096 bytes alignment */
#define PAGE_ALIGN 0xfffff000
#define ADDR_ALIGN 0xfff
#define OFFS_ALIGN 0x7ffff000

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_ARM_JUMP_SLOT) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_ARM_COPY) * ELF_RTYPE_CLASS_COPY))

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline Elf32_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
	register Elf32_Addr *got asm ("r10");
	return *got;
}


/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
	extern void __dl_start asm ("_dl_start");
	Elf32_Addr got_addr = (Elf32_Addr) &__dl_start;
	Elf32_Addr pcrel_addr;
#if !defined __thumb__
	asm ("adr %0, _dl_start" : "=r" (pcrel_addr));
#else
	int tmp;
	/* The above adr will not work on thumb because it
	 * is negative.  The only safe way is to temporarily
	 * swap to arm.
	 */
	asm(   ".align	2\n"
	"	bx	pc\n"
	"	nop	\n"
	"	.arm	\n"
	"	adr	%0, _dl_start\n"
	"	.align	2\n"
	"	orr	%1, pc, #1\n"
	"	bx	%1\n"
	"	.force_thumb\n"
	: "=r" (pcrel_addr), "=&r" (tmp));
#endif
	return pcrel_addr - got_addr;
}

static inline void
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
