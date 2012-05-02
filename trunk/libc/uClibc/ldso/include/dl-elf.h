/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Lesser General Public License version 2.1 or later.
 */

#ifndef LINUXELF_H
#define LINUXELF_H

#include <dl-string.h> /* before elf.h to get ELF_USES_RELOCA right */
#include <elf.h>
#include <link.h>

/* Forward declarations for stuff defined in ld_hash.h */
struct dyn_elf;
struct elf_resolve;

#include <dl-defs.h>
#ifdef __LDSO_CACHE_SUPPORT__
extern int _dl_map_cache(void);
extern int _dl_unmap_cache(void);
#else
static inline void _dl_map_cache(void) { }
static inline void _dl_unmap_cache(void) { }
#endif


/* Function prototypes for non-static stuff in readelflib1.c */
extern void _dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
	unsigned long rel_addr, unsigned long rel_size);
extern int _dl_parse_relocation_information(struct dyn_elf *rpnt,
	unsigned long rel_addr, unsigned long rel_size);
extern struct elf_resolve * _dl_load_shared_library(int secure,
	struct dyn_elf **rpnt, struct elf_resolve *tpnt, char *full_libname,
	int trace_loaded_objects);
extern struct elf_resolve * _dl_load_elf_shared_library(int secure,
	struct dyn_elf **rpnt, char *libname);
extern struct elf_resolve *_dl_check_if_named_library_is_loaded(const char *full_libname,
	int trace_loaded_objects);
extern int _dl_linux_resolve(void);
extern int _dl_fixup(struct dyn_elf *rpnt, int flag);
extern void _dl_protect_relro (struct elf_resolve *l);

/*
 * Bitsize related settings for things ElfW()
 * does not handle already
 */
#if __WORDSIZE == 64
# define ELF_ST_BIND(val) ELF64_ST_BIND(val)
# define ELF_ST_TYPE(val) ELF64_ST_TYPE(val)
# define ELF_R_SYM(i)     ELF64_R_SYM(i)
# define ELF_R_TYPE(i)    ELF64_R_TYPE(i)
# ifndef ELF_CLASS
#  define ELF_CLASS ELFCLASS64
# endif
#else
# define ELF_ST_BIND(val) ELF32_ST_BIND(val)
# define ELF_ST_TYPE(val) ELF32_ST_TYPE(val)
# define ELF_R_SYM(i)     ELF32_R_SYM(i)
# define ELF_R_TYPE(i)    ELF32_R_TYPE(i)
# ifndef ELF_CLASS
#  define ELF_CLASS ELFCLASS32
# endif
#endif

/*
 * Datatype of a relocation on this platform
 */
#ifdef ELF_USES_RELOCA
# define ELF_RELOC	ElfW(Rela)
# define DT_RELOC_TABLE_ADDR	DT_RELA
# define DT_RELOC_TABLE_SIZE	DT_RELASZ
# define DT_RELOCCOUNT		DT_RELACOUNT
# define UNSUPPORTED_RELOC_TYPE	DT_REL
# define UNSUPPORTED_RELOC_STR	"REL"
#else
# define ELF_RELOC	ElfW(Rel)
# define DT_RELOC_TABLE_ADDR	DT_REL
# define DT_RELOC_TABLE_SIZE	DT_RELSZ
# define DT_RELOCCOUNT		DT_RELCOUNT
# define UNSUPPORTED_RELOC_TYPE	DT_RELA
# define UNSUPPORTED_RELOC_STR	"RELA"
#endif

/* OS and/or GNU dynamic extensions */
#define OS_NUM 1
#define DT_RELCONT_IDX DT_NUM

#ifndef ARCH_DYNAMIC_INFO
  /* define in arch specific code, if needed */
# define ARCH_NUM 0
#endif

#define DYNAMIC_SIZE (DT_NUM+OS_NUM+ARCH_NUM)

extern void _dl_parse_dynamic_info(ElfW(Dyn) *dpnt, unsigned long dynamic_info[], void *debug_addr, ElfW(Addr) load_off);

static __always_inline
void __dl_parse_dynamic_info(ElfW(Dyn) *dpnt, unsigned long dynamic_info[], void *debug_addr, ElfW(Addr) load_off)
{
	for (; dpnt->d_tag; dpnt++) {
		if (dpnt->d_tag < DT_NUM) {
			dynamic_info[dpnt->d_tag] = dpnt->d_un.d_val;
#ifndef __mips__
			if (dpnt->d_tag == DT_DEBUG)
				dpnt->d_un.d_val = (unsigned long)debug_addr;
#endif
			if (dpnt->d_tag == DT_BIND_NOW)
				dynamic_info[DT_BIND_NOW] = 1;
			if (dpnt->d_tag == DT_FLAGS &&
			    (dpnt->d_un.d_val & DF_BIND_NOW))
				dynamic_info[DT_BIND_NOW] = 1;
			if (dpnt->d_tag == DT_TEXTREL)
				dynamic_info[DT_TEXTREL] = 1;
#ifdef __LDSO_RUNPATH__
			if (dpnt->d_tag == DT_RUNPATH)
				dynamic_info[DT_RPATH] = 0;
			if (dpnt->d_tag == DT_RPATH && dynamic_info[DT_RUNPATH])
				dynamic_info[DT_RPATH] = 0;
#endif
		} else if (dpnt->d_tag < DT_LOPROC) {
			if (dpnt->d_tag == DT_RELOCCOUNT)
				dynamic_info[DT_RELCONT_IDX] = dpnt->d_un.d_val;
			if (dpnt->d_tag == DT_FLAGS_1 &&
			    (dpnt->d_un.d_val & DF_1_NOW))
				dynamic_info[DT_BIND_NOW] = 1;
		}
#ifdef ARCH_DYNAMIC_INFO
		else {
			ARCH_DYNAMIC_INFO(dpnt, dynamic_info, debug_addr);
		}
#endif
	}
#define ADJUST_DYN_INFO(tag, load_off) \
	do { \
		if (dynamic_info[tag]) \
			dynamic_info[tag] += load_off; \
	} while(0)
	ADJUST_DYN_INFO(DT_HASH, load_off);
	ADJUST_DYN_INFO(DT_PLTGOT, load_off);
	ADJUST_DYN_INFO(DT_STRTAB, load_off);
	ADJUST_DYN_INFO(DT_SYMTAB, load_off);
	ADJUST_DYN_INFO(DT_RELOC_TABLE_ADDR, load_off);
	ADJUST_DYN_INFO(DT_JMPREL, load_off);
#undef ADJUST_DYN_INFO
}

/* Reloc type classes as returned by elf_machine_type_class().
   ELF_RTYPE_CLASS_PLT means this reloc should not be satisfied by
   some PLT symbol, ELF_RTYPE_CLASS_COPY means this reloc should not be
   satisfied by any symbol in the executable.  Some architectures do
   not support copy relocations.  In this case we define the macro to
   zero so that the code for handling them gets automatically optimized
   out.  */
#ifdef DL_NO_COPY_RELOCS
# define ELF_RTYPE_CLASS_COPY	(0x0)
#else
# define ELF_RTYPE_CLASS_COPY	(0x2)
#endif
#define ELF_RTYPE_CLASS_PLT	(0x1)


/* Convert between the Linux flags for page protections and the
   ones specified in the ELF standard. */
#define LXFLAGS(X) ( (((X) & PF_R) ? PROT_READ : 0) | \
		    (((X) & PF_W) ? PROT_WRITE : 0) | \
		    (((X) & PF_X) ? PROT_EXEC : 0))


#endif	/* LINUXELF_H */
