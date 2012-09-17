/* vi: set sw=8 ts=8: */
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
#define INIT_GOT(GOT_BASE,MODULE)				\
{								\
	GOT_BASE[2] = (unsigned long)_dl_linux_resolve;		\
	GOT_BASE[1] = (unsigned long)(MODULE);			\
}

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_SH
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "sh64"

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_SH_JMP_SLOT) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_SH_COPY) * ELF_RTYPE_CLASS_COPY))

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static __always_inline Elf32_Addr elf_machine_dynamic(void)
{
	register Elf32_Addr *got;

	/*
	 * The toolchain adds 32768 to the GOT address, we compensate for
	 * that in the movi/sub pair.
	 *
	 * XXX: If this is cleaned up in the toolchain, we can end up
	 * saving 2 instructions and subsequently free up r1 from the
	 * clobber list..
	 */
	__asm__ (
		"movi\t(((datalabel _GLOBAL_OFFSET_TABLE_-(.LZZZ1-.)) >> 16) & 0xffff), r2\n\t"
		"shori\t((datalabel _GLOBAL_OFFSET_TABLE_-(.LZZZ1-.)) & 0xffff), r2\n\t"
		".LZZZ1:\tptrel/u r2, tr0\n\t"
		"movi\t32768, r1\n\t"
		"gettr\ttr0, r2\n\t"
		"sub\tr2, r1, %0\n\t"
		: "=r" (got)
		: /* no inputs */
		: "r1", "r2", "tr0"
	);

	return *got;
}

/* Return the run-time load address of the shared object.  */
static __always_inline Elf32_Addr elf_machine_load_address(void)
{
	Elf32_Addr addr;

	__asm__ (
		"movi\t(((datalabel _GLOBAL_OFFSET_TABLE_-(.LZZZ2-.)) >> 16) & 0xffff), r0\n\t"
		"shori\t((datalabel _GLOBAL_OFFSET_TABLE_-(.LZZZ2-.)) & 0xffff), r0\n\t"
		".LZZZ2:\tptrel/u r0, tr0\n\t"
		"movi\t(((_dl_start@GOTOFF) >> 16) & 0xffff), r2\n\t"
		"shori\t((_dl_start@GOTOFF) & 0xffff), r2\n\t"
		"gettr\ttr0, r0\n\t"
		"add\tr2, r0, r2\n\t"
		"movi\t(((_dl_start@GOT) >> 16) & 0xffff), r1\n\t"
		"shori\t((_dl_start@GOT) & 0xffff), r1\n\t"
		"ldx.l\tr1, r0, r1\n\t"
		"sub\tr2, r1, %0\n\t"
		: "=r" (addr)
		: /* no inputs */
		: "r0", "r1", "r2", "tr0"
	);

	return addr;
}

/*
 * XXX: As we don't need to worry about r25 clobbering, we could probably
 * get away with inlining {st,ld}{x,}.l and friends here instead and
 * forego gcc's idea of code generation.
 */
#define COPY_UNALIGNED_WORD(swp, twp, align)		\
{							\
	void *__s = (swp), *__t = (twp);		\
	unsigned char *__s1 = __s, *__t1 = __t;		\
	unsigned short *__s2 = __s, *__t2 = __t;	\
	unsigned long *__s4 = __s, *__t4 = __t;		\
							\
	switch ((align)) {				\
	case 0:						\
		*__t4 = *__s4;				\
		break;					\
	case 2:						\
		*__t2++ = *__s2++;			\
		*__t2 = *__s2;				\
		break;					\
	default:					\
		*__t1++ = *__s1++;			\
		*__t1++ = *__s1++;			\
		*__t1++ = *__s1++;			\
		*__t1 = *__s1;				\
		break;					\
	}						\
}

static __always_inline void
elf_machine_relative(Elf32_Addr load_off, const Elf32_Addr rel_addr,
		     Elf32_Word relative_count)
{
	Elf32_Addr value, word;
	Elf32_Rela *rpnt = (void *)rel_addr;
	int reloc_type = ELF_R_TYPE(rpnt->r_info);

	do {
		Elf32_Addr *const reloc_addr =
			(void *)(load_off + rpnt->r_offset);
		int align = (int)reloc_addr & 3;

		switch (reloc_type) {
		case R_SH_RELATIVE_LOW16:
			COPY_UNALIGNED_WORD(reloc_addr, &word, align);
			word &= ~0x3fffc00;
			value = (rpnt->r_addend + load_off);
			word |= (value & 0xffff) << 10;
			COPY_UNALIGNED_WORD(&word, reloc_addr, align);
			break;
		case R_SH_RELATIVE_MEDLOW16:
			COPY_UNALIGNED_WORD(reloc_addr, &word, align);
			word &= ~0x3fffc00;
			value = (rpnt->r_addend + load_off) >> 16;
			word |= (value & 0xffff) << 10;
			COPY_UNALIGNED_WORD(&word, reloc_addr, align);
			break;
		default:
			if (rpnt->r_addend) {
				value = load_off + rpnt->r_addend;
			} else {
				COPY_UNALIGNED_WORD(reloc_addr, &value, align);
				value += load_off;
			}

			COPY_UNALIGNED_WORD(&value, reloc_addr, align);
			break;
		}

		rpnt++;
	} while (--relative_count);
#undef COPY_UNALIGNED_WORD
}
