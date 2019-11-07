/*
 * Various assembly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 */

/* Define this if the system uses RELOCA.  */
#define ELF_USES_RELOCA
#include <elf.h>
/*
 * Initialization sequence for a GOT.
 */
#define INIT_GOT(GOT_BASE,MODULE) \
{				\
  GOT_BASE[2] = (unsigned long) _dl_linux_resolve; \
  GOT_BASE[1] = (unsigned long) (MODULE); \
}

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_SH
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "sh"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

static __always_inline unsigned int
_dl_urem(unsigned int n, unsigned int base)
{
  int res;

	__asm__ (""\
		"mov	#0, r0\n\t" \
		"div0u\n\t" \
		"" \
		"! get one bit from the msb of the numerator into the T\n\t" \
		"! bit and divide it by whats in %2.  Put the answer bit\n\t" \
		"! into the T bit so it can come out again at the bottom\n\t" \
		""				\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		""				\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		""				\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
 		""				\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1\n\t"
		: "=r" (res)
		: "0" (n), "r" (base)
		: "r0","cc");

	return n - (base * res);
}

#define do_rem(result, n, base)     ((result) = _dl_urem((n), (base)))

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
# define elf_machine_type_class(type) \
  ((((type) == R_SH_JMP_SLOT || (type) == R_SH_TLS_DTPMOD32		      \
     || (type) == R_SH_TLS_DTPOFF32 || (type) == R_SH_TLS_TPOFF32)	      \
    * ELF_RTYPE_CLASS_PLT)						      \
   | (((type) == R_SH_COPY) * ELF_RTYPE_CLASS_COPY))

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static __always_inline Elf32_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
	register Elf32_Addr *got;
	__asm__ ("mov r12,%0" :"=r" (got));
	return *got;
}

/* Return the run-time load address of the shared object.  */
static __always_inline Elf32_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
	Elf32_Addr addr;
	__asm__ ("mov.l 1f,r0\n\
        mov.l 3f,r2\n\
        add r12,r2\n\
        mov.l @(r0,r12),r0\n\
        bra 2f\n\
         sub r0,r2\n\
        .align 2\n\
        1: .long _dl_start@GOT\n\
        3: .long _dl_start@GOTOFF\n\
        2: mov r2,%0"
	     : "=r" (addr) : : "r0", "r1", "r2");
	return addr;
}

#define COPY_UNALIGNED_WORD(swp, twp, align) \
  { \
    void *__s = (swp), *__t = (twp); \
    unsigned char *__s1 = __s, *__t1 = __t; \
    unsigned short *__s2 = __s, *__t2 = __t; \
    unsigned long *__s4 = __s, *__t4 = __t; \
    switch ((align)) \
    { \
    case 0: \
      *__t4 = *__s4; \
      break; \
    case 2: \
      *__t2++ = *__s2++; \
      *__t2 = *__s2; \
      break; \
    default: \
      *__t1++ = *__s1++; \
      *__t1++ = *__s1++; \
      *__t1++ = *__s1++; \
      *__t1 = *__s1; \
      break; \
    } \
  }

static __always_inline void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	Elf32_Addr value;
	Elf32_Rela * rpnt = (void *)rel_addr;

	do {
		Elf32_Addr *const reloc_addr = (void *) (load_off + rpnt->r_offset);

		if (rpnt->r_addend)
			value = load_off + rpnt->r_addend;
		else {
			COPY_UNALIGNED_WORD (reloc_addr, &value, (int) reloc_addr & 3);
			value += load_off;
		}
		COPY_UNALIGNED_WORD (&value, reloc_addr, (int) reloc_addr & 3);
		rpnt++;
	} while (--relative_count);
#undef COPY_UNALIGNED_WORD
}
