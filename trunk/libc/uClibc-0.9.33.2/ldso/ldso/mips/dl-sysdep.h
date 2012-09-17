/* vi: set sw=8 ts=8: */

/*
 * Various assembly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 */

/* Define this if the system uses RELOCA.  */
#undef ELF_USES_RELOCA
#include <elf.h>

#ifdef __mips64	/* from glibc sysdeps/mips/elf/ldsodefs.h 1.4 */
/* The 64-bit MIPS ELF ABI uses an unusual reloc format.  Each
   relocation entry specifies up to three actual relocations, all at
   the same address.  The first relocation which required a symbol
   uses the symbol in the r_sym field.  The second relocation which
   requires a symbol uses the symbol in the r_ssym field.  If all
   three relocations require a symbol, the third one uses a zero
   value.

   We define these structures in internal headers because we're not
   sure we want to make them part of the ABI yet.  Eventually, some of
   this may move into elf/elf.h.  */

/* An entry in a 64 bit SHT_REL section.  */

typedef struct
{
  Elf32_Word    r_sym;		/* Symbol index */
  unsigned char r_ssym;		/* Special symbol for 2nd relocation */
  unsigned char r_type3;	/* 3rd relocation type */
  unsigned char r_type2;	/* 2nd relocation type */
  unsigned char r_type1;	/* 1st relocation type */
} _Elf64_Mips_R_Info;

typedef union
{
  Elf64_Xword	r_info_number;
  _Elf64_Mips_R_Info r_info_fields;
} _Elf64_Mips_R_Info_union;

typedef struct
{
  Elf64_Addr	r_offset;		/* Address */
  _Elf64_Mips_R_Info_union r_info;	/* Relocation type and symbol index */
} Elf64_Mips_Rel;

typedef struct
{
  Elf64_Addr	r_offset;		/* Address */
  _Elf64_Mips_R_Info_union r_info;	/* Relocation type and symbol index */
  Elf64_Sxword	r_addend;		/* Addend */
} Elf64_Mips_Rela;

#define ELF64_MIPS_R_SYM(i) \
  ((__extension__ (_Elf64_Mips_R_Info_union)(i)).r_info_fields.r_sym)
#define ELF64_MIPS_R_TYPE(i) \
  (((_Elf64_Mips_R_Info_union)(i)).r_info_fields.r_type1 \
   | ((Elf32_Word)(__extension__ (_Elf64_Mips_R_Info_union)(i) \
		   ).r_info_fields.r_type2 << 8) \
   | ((Elf32_Word)(__extension__ (_Elf64_Mips_R_Info_union)(i) \
		   ).r_info_fields.r_type3 << 16) \
   | ((Elf32_Word)(__extension__ (_Elf64_Mips_R_Info_union)(i) \
		   ).r_info_fields.r_ssym << 24))
#define ELF64_MIPS_R_INFO(sym, type) \
  (__extension__ (_Elf64_Mips_R_Info_union) \
   (__extension__ (_Elf64_Mips_R_Info) \
   { (sym), ELF64_MIPS_R_SSYM (type), \
       ELF64_MIPS_R_TYPE3 (type), \
       ELF64_MIPS_R_TYPE2 (type), \
       ELF64_MIPS_R_TYPE1 (type) \
   }).r_info_number)
/* These macros decompose the value returned by ELF64_MIPS_R_TYPE, and
   compose it back into a value that it can be used as an argument to
   ELF64_MIPS_R_INFO.  */
#define ELF64_MIPS_R_SSYM(i) (((i) >> 24) & 0xff)
#define ELF64_MIPS_R_TYPE3(i) (((i) >> 16) & 0xff)
#define ELF64_MIPS_R_TYPE2(i) (((i) >> 8) & 0xff)
#define ELF64_MIPS_R_TYPE1(i) ((i) & 0xff)
#define ELF64_MIPS_R_TYPEENC(type1, type2, type3, ssym) \
  ((type1) \
   | ((Elf32_Word)(type2) << 8) \
   | ((Elf32_Word)(type3) << 16) \
   | ((Elf32_Word)(ssym) << 24))

#undef ELF64_R_SYM
#define ELF64_R_SYM(i) ELF64_MIPS_R_SYM (i)
#undef ELF64_R_TYPE
#define ELF64_R_TYPE(i) ELF64_MIPS_R_TYPE (i)
#undef ELF64_R_INFO
#define ELF64_R_INFO(sym, type) ELF64_MIPS_R_INFO ((sym), (type))
#endif	/* __mips64 */

#include <link.h>

#define ARCH_NUM 4
#define DT_MIPS_GOTSYM_IDX	(DT_NUM + OS_NUM)
#define DT_MIPS_LOCAL_GOTNO_IDX	(DT_NUM + OS_NUM +1)
#define DT_MIPS_SYMTABNO_IDX	(DT_NUM + OS_NUM +2)
#define DT_MIPS_PLTGOT_IDX	(DT_NUM + OS_NUM +3)

#define ARCH_DYNAMIC_INFO(dpnt,  dynamic, debug_addr) \
do { \
if (dpnt->d_tag == DT_MIPS_GOTSYM) \
     dynamic[DT_MIPS_GOTSYM_IDX] = dpnt->d_un.d_val; \
else if (dpnt->d_tag == DT_MIPS_LOCAL_GOTNO) \
     dynamic[DT_MIPS_LOCAL_GOTNO_IDX] = dpnt->d_un.d_val; \
else if (dpnt->d_tag == DT_MIPS_SYMTABNO) \
     dynamic[DT_MIPS_SYMTABNO_IDX] = dpnt->d_un.d_val; \
else if (dpnt->d_tag == DT_MIPS_PLTGOT) \
     dynamic[DT_MIPS_PLTGOT_IDX] = dpnt->d_un.d_val; \
else if ((dpnt->d_tag == DT_MIPS_RLD_MAP) && (dpnt->d_un.d_ptr)) \
     *(ElfW(Addr) *)(dpnt->d_un.d_ptr) =  (ElfW(Addr)) debug_addr; \
} while (0)

#define ARCH_SKIP_RELOC(type_class, sym) \
     ((sym)->st_shndx == SHN_UNDEF && !((sym)->st_other & STO_MIPS_PLT))

/* Initialization sequence for the application/library GOT.  */
#define INIT_GOT(GOT_BASE,MODULE)						\
do {										\
	unsigned long idx;							\
	unsigned long *pltgot;							\
										\
	/* Check if this is the dynamic linker itself */			\
	if (MODULE->libtype == program_interpreter)				\
		continue;							\
										\
	/* Fill in first two GOT entries according to the ABI */		\
	GOT_BASE[0] = (unsigned long) _dl_runtime_resolve;			\
	GOT_BASE[1] = (unsigned long) MODULE;					\
										\
	pltgot = (unsigned long *) MODULE->dynamic_info[DT_MIPS_PLTGOT_IDX];	\
	if (pltgot) {								\
		pltgot[0] = (unsigned long) _dl_runtime_pltresolve;		\
		pltgot[1] = (unsigned long) MODULE;				\
	}									\
										\
	/* Add load address displacement to all local GOT entries */		\
	idx = 2;									\
	while (idx < MODULE->dynamic_info[DT_MIPS_LOCAL_GOTNO_IDX])		\
		GOT_BASE[idx++] += (unsigned long) MODULE->loadaddr;		\
										\
} while (0)


/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_MIPS
#define MAGIC2 EM_MIPS_RS3_LE


/* Used for error messages */
#define ELF_TARGET "MIPS"

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

unsigned long __dl_runtime_resolve(unsigned long sym_index,
	unsigned long old_gpreg);

struct elf_resolve;
unsigned long __dl_runtime_pltresolve(struct elf_resolve *tpnt,
	int reloc_entry);

void _dl_perform_mips_global_got_relocations(struct elf_resolve *tpnt, int lazy);

/* 4096 bytes alignment */
#if _MIPS_SIM == _MIPS_SIM_ABI64
#define OFFS_ALIGN (0x10000000000UL-0x1000)
#endif	/* O32 || N32 */

#if defined USE_TLS
# if _MIPS_SIM == _MIPS_SIM_ABI64
# define elf_machine_type_class(type) 					\
  ((((type) == R_MIPS_JUMP_SLOT || (type) == R_MIPS_TLS_DTPMOD64	\
     || (type) == R_MIPS_TLS_DTPREL64 || (type) == R_MIPS_TLS_TPREL64)	\
    * ELF_RTYPE_CLASS_PLT)						\
   | (((type) == R_MIPS_COPY) * ELF_RTYPE_CLASS_COPY))
# else
# define elf_machine_type_class(type)					\
  ((((type) == R_MIPS_JUMP_SLOT || (type) == R_MIPS_TLS_DTPMOD32	\
     || (type) == R_MIPS_TLS_DTPREL32 || (type) == R_MIPS_TLS_TPREL32)	\
    * ELF_RTYPE_CLASS_PLT)						\
   | (((type) == R_MIPS_COPY) * ELF_RTYPE_CLASS_COPY))
# endif /* _MIPS_SIM == _MIPS_SIM_ABI64 */
#else
#define elf_machine_type_class(type)					\
  ((((type) == R_MIPS_JUMP_SLOT) * ELF_RTYPE_CLASS_PLT)			\
   | (((type) == R_MIPS_COPY) * ELF_RTYPE_CLASS_COPY))
#endif /* USE_TLS */

#define OFFSET_GP_GOT 0x7ff0

static __always_inline ElfW(Addr) *
elf_mips_got_from_gpreg (ElfW(Addr) gpreg)
{
	/* FIXME: the offset of gp from GOT may be system-dependent. */
	return (ElfW(Addr) *) (gpreg - OFFSET_GP_GOT);
}

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  We assume its $gp points to the primary GOT.  */
static __always_inline ElfW(Addr)
elf_machine_dynamic (void)
{
	register ElfW(Addr) gp __asm__ ("$28");
	return *elf_mips_got_from_gpreg (gp);
}

#define STRINGXP(X) __STRING(X)
#define STRINGXV(X) STRINGV_(X)
#define STRINGV_(...) # __VA_ARGS__
#if _MIPS_SIM == _MIPS_SIM_ABI64
#define PTR_LA               dla
#define PTR_SUBU     dsubu
#else
#define PTR_LA               la
#define PTR_SUBU     subu
#endif

/* Return the run-time load address of the shared object.  */
static __always_inline ElfW(Addr)
elf_machine_load_address (void)
{
	ElfW(Addr) addr;
	__asm__ ("        .set noreorder\n"
	     "        " STRINGXP (PTR_LA) " %0, 0f\n"
	     "        bltzal $0, 0f\n"
	     "        nop\n"
	     "0:      " STRINGXP (PTR_SUBU) " %0, $31, %0\n"
	     "        .set reorder\n"
	     :        "=r" (addr)
	     :        /* No inputs */
	     :        "$31");
	return addr;
}

static __always_inline void
elf_machine_relative (ElfW(Addr) load_off, const ElfW(Addr) rel_addr,
		      ElfW(Word) relative_count)
{
	/* No RELATIVE relocs in MIPS? */
}

#ifdef __mips64
#define DL_MALLOC_ALIGN 8	/* N64/N32 needs 8 byte alignment */
#endif
