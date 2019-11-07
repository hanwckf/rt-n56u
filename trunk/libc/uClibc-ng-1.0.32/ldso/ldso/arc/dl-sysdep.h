/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include "elf.h"

/*
 * Define this if the system uses RELOCA.
 */
#define ELF_USES_RELOCA

/*
 * Dynamic Linking ABI for ARCompact ISA
 *
 *                      PLT
 *        --------------------------------
 *        |  ld r11, [pcl, off-to-GOT[1] |  0   (20 bytes)
 *        |                              |  4
 * plt0   |  ld r10, [pcl, off-to-GOT[2] |  8
 *        |                              | 12
 *        |  j [r10]                     | 16
 *        --------------------------------
 *        |    Base address of GOT       | 20
 *        --------------------------------
 *        |  ld r12, [pcl, off-to-GOT[3] | 24   (12 bytes each)
 * plt1   |                              |
 *        |  j_s.d  [r12]                | 32
 *        |  mov_s  r12, pcl             | 34
 *        --------------------------------
 *        |                              | 36
 *        ~                              ~
 *        ~                              ~
 *        |                              |
 *        --------------------------------
 *
 *             GOT
 *        --------------
 *        |    [0]     |
 *        --------------
 *        |    [1]     |  Module info - setup by ldso
 *        --------------
 *        |    [2]     |  resolver entry point
 *        --------------
 *        |    [3]     |
 *        |    ...     |  Runtime address for function symbols
 *        |    [f]     |
 *        --------------
 *        |    [f+1]   |
 *        |    ...     |  Runtime address for data symbols
 *        |    [last]  |
 *        --------------
 */

/*
 * Initialization sequence for a GOT.
 * Caller elf_resolve() seeds @GOT_BASE from DT_PLTGOT - which essentially is
 * pointer to first PLT entry. The actual GOT base is 5th word in PLT
 *
 */
#define INIT_GOT(GOT_BASE,MODULE)					\
do {									\
	unsigned long *__plt_base = (unsigned long *)GOT_BASE;		\
	GOT_BASE = (unsigned long *)(__plt_base[5] +			\
		                     (unsigned long)MODULE->loadaddr);	\
	GOT_BASE[1] = (unsigned long) MODULE;				\
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve;		\
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */
#ifdef __A7__
#define MAGIC1 EM_ARCOMPACT
#define ELF_TARGET "ARCompact"	/* For error messages */
#elif defined(__HS__)
#define MAGIC1 EM_ARCV2
#define ELF_TARGET "ARCv2"	/* For error messages */
#endif

#undef  MAGIC2


struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt,
					 unsigned int plt_pc);

extern unsigned __udivmodsi4(unsigned, unsigned) attribute_hidden;

#ifdef __A7__
/* using "C" causes an indirection via __umodsi3 -> __udivmodsi4 */
#define do_rem(result, n, base)  ((result) =				\
									\
	__builtin_constant_p (base) ? (n) % (unsigned) (base) :		\
	__extension__ ({						\
		register unsigned r1 __asm__ ("r1") = (base);		\
									\
		__asm__("bl.d @__udivmodsi4` mov r0,%1"			\
		: "=r" (r1)						\
	        : "r" (n), "r" (r1)					\
	        : "r0", "r2", "r3", "r4", "lp_count", "blink", "cc");	\
									\
		r1;							\
	})								\
)
#elif defined(__HS__)
/* ARCv2 has hardware assisted divide/mod */
#define do_rem(result, n, base)  ((result) = (n) % (unsigned) (base))
#endif

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable so PLT entries should not be allowed to define the value.

   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_ARC_JMP_SLOT || (type) == R_ARC_TLS_DTPMOD ||	\
     (type) == R_ARC_TLS_DTPOFF || (type) == R_ARC_TLS_TPOFF)	\
   * ELF_RTYPE_CLASS_PLT)					\
   | (((type) == R_ARC_COPY) * ELF_RTYPE_CLASS_COPY))

/*
 * Get build time address of .dynamic as setup in GOT[0]
 * This is called very early in _dl_start() so it has not been relocated to
 * runtime value
 */
static __always_inline Elf32_Addr elf_machine_dynamic(void)
{
	extern const Elf32_Addr _GLOBAL_OFFSET_TABLE_[] attribute_hidden;
	return _GLOBAL_OFFSET_TABLE_[0];
}

/* Return the run-time load address of the shared object.  */
static __always_inline Elf32_Addr elf_machine_load_address(void)
{
    /* To find the loadaddr we subtract the runtime addr of a non-local symbol
     * say _DYNAMIC from it's build-time addr.
     * N.B., gotpc loads get optimized by the linker if it finds the symbol
     * is resolved locally.
     * A more robust - and efficient - solution would be to use a symbol
     * set by the linker.  To make it actually save space, we'd have to
     * suppress the unwanted text relocation in the linked dso, though.
     * (I.e. in ldso.so.*, though it's just another dso as far as bfd/ld
     * are concerned.)
     */
	Elf32_Addr addr, tmp;
	__asm__ (
        "ld  %1, [pcl, _dl_start@gotpc] ;build addr of _DYNAMIC"   "\n"
        "add %0, pcl, _dl_start@pcl     ;runtime addr of _DYNAMIC" "\n"
        "sub %0, %0, %1                ;delta"                    "\n"
        : "=&r" (addr), "=r"(tmp)
    );
	return addr;
}

static __always_inline void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	 Elf32_Rela * rpnt = (void *) rel_addr;
	--rpnt;
	do {
		Elf32_Addr *const reloc_addr = (void *) (load_off + (++rpnt)->r_offset);
		*reloc_addr += load_off;
	} while (--relative_count);
}
