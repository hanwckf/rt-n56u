/*
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Lesser General Public License version 2.1 or later.
 */

#ifndef _DL_ELF_H
#define _DL_ELF_H

#include <features.h>
#include <bits/wordsize.h>
#include <dl-string.h> /* before elf.h to get ELF_USES_RELOCA right */
#include <elf.h>
#include <link.h>
#include <dl-defs.h>
#include <dlfcn.h>

/* Forward declarations for stuff defined in dl-hash.h */
struct dyn_elf;
struct elf_resolve;
struct r_scope_elem;

#ifdef __LDSO_CACHE_SUPPORT__
extern int _dl_map_cache(void);
extern int _dl_unmap_cache(void);
#else
static __inline__ void _dl_map_cache(void) { }
static __inline__ void _dl_unmap_cache(void) { }
#endif

/* Function prototypes for non-static stuff in elfinterp.c */
extern void _dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
	unsigned long rel_addr, unsigned long rel_size);
extern int _dl_parse_relocation_information(struct dyn_elf *rpnt,
	struct r_scope_elem *scope, unsigned long rel_addr, unsigned long rel_size);
extern struct elf_resolve * _dl_load_shared_library(unsigned int rflags,
	struct dyn_elf **rpnt, struct elf_resolve *tpnt, char *full_libname,
	int trace_loaded_objects);
extern struct elf_resolve * _dl_load_elf_shared_library(unsigned int rflags,
	struct dyn_elf **rpnt, const char *libname);
extern int _dl_linux_resolve(void);
extern int _dl_fixup(struct dyn_elf *rpnt, struct r_scope_elem *scope, int flag);
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

#define OS_NUM_BASE 1			/* for DT_RELOCCOUNT */

#ifdef __LDSO_GNU_HASH_SUPPORT__
# define OS_NUM_GNU_HASH	1   /* for DT_GNU_HASH entry */
#else
# define OS_NUM_GNU_HASH	0
#endif

#ifdef __LDSO_PRELINK_SUPPORT__
# define OS_NUM_PRELINK		6   /* for DT_GNU_PRELINKED entry */
#else
# define OS_NUM_PRELINK	0
#endif

#define OS_NUM	  (OS_NUM_BASE + OS_NUM_GNU_HASH + OS_NUM_PRELINK)

#ifndef ARCH_DYNAMIC_INFO
  /* define in arch specific code, if needed */
# define ARCH_NUM 0
#endif

#define DYNAMIC_SIZE (DT_NUM + OS_NUM + ARCH_NUM)
/* Keep ARCH specific entries into dynamic section at the end of the array */
#define DT_RELCONT_IDX (DYNAMIC_SIZE - OS_NUM - ARCH_NUM)

#ifdef __LDSO_GNU_HASH_SUPPORT__
/* GNU hash comes just after the relocation count */
# define DT_GNU_HASH_IDX (DT_RELCONT_IDX + 1)
#else
# define DT_GNU_HASH_IDX DT_RELCONT_IDX
#endif

#ifdef __LDSO_PRELINK_SUPPORT__
/* GNU prelink comes just after the GNU hash if present */
#define DT_GNU_PRELINKED_IDX  (DT_GNU_HASH_IDX + 1)
#define DT_GNU_CONFLICT_IDX   (DT_GNU_HASH_IDX + 2)
#define DT_GNU_CONFLICTSZ_IDX (DT_GNU_HASH_IDX + 3)
#define DT_GNU_LIBLIST_IDX    (DT_GNU_HASH_IDX + 4)
#define DT_GNU_LIBLISTSZ_IDX  (DT_GNU_HASH_IDX + 5)
#define DT_CHECKSUM_IDX       (DT_GNU_HASH_IDX + 6)
#endif

extern unsigned int _dl_parse_dynamic_info(ElfW(Dyn) *dpnt, unsigned long dynamic_info[],
                                           void *debug_addr, DL_LOADADDR_TYPE load_off);

static __always_inline
unsigned int __dl_parse_dynamic_info(ElfW(Dyn) *dpnt, unsigned long dynamic_info[],
                                     void *debug_addr, DL_LOADADDR_TYPE load_off)
{
	unsigned int rtld_flags = 0;

	for (; dpnt->d_tag; dpnt++) {
		if (dpnt->d_tag < DT_NUM) {
			dynamic_info[dpnt->d_tag] = dpnt->d_un.d_val;
#ifndef __mips__
			/* we disable for mips because normally this page is readonly
			 * and modifying the value here needlessly dirties a page.
			 * see this post for more info:
			 * http://uclibc.org/lists/uclibc/2006-April/015224.html */
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
			if (dpnt->d_tag == DT_FLAGS_1) {
				if (dpnt->d_un.d_val & DF_1_NOW)
					dynamic_info[DT_BIND_NOW] = 1;
				if (dpnt->d_un.d_val & DF_1_NODELETE)
					rtld_flags |= RTLD_NODELETE;
			}
#ifdef __LDSO_GNU_HASH_SUPPORT__
			if (dpnt->d_tag == DT_GNU_HASH)
				dynamic_info[DT_GNU_HASH_IDX] = dpnt->d_un.d_ptr;
#endif
#ifdef __LDSO_PRELINK_SUPPORT__
			if (dpnt->d_tag == DT_GNU_PRELINKED)
				dynamic_info[DT_GNU_PRELINKED_IDX] = dpnt->d_un.d_val;
			if (dpnt->d_tag == DT_GNU_CONFLICT)
				dynamic_info[DT_GNU_CONFLICT_IDX] = dpnt->d_un.d_ptr;
			if (dpnt->d_tag == DT_GNU_CONFLICTSZ)
				dynamic_info[DT_GNU_CONFLICTSZ_IDX] = dpnt->d_un.d_val;
			if (dpnt->d_tag == DT_GNU_LIBLIST)
				dynamic_info[DT_GNU_LIBLIST_IDX] = dpnt->d_un.d_ptr;
			if (dpnt->d_tag == DT_GNU_LIBLISTSZ)
				dynamic_info[DT_GNU_LIBLISTSZ_IDX] = dpnt->d_un.d_val;
			if (dpnt->d_tag == DT_CHECKSUM)
				dynamic_info[DT_CHECKSUM_IDX] = dpnt->d_un.d_val;
#endif
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
			dynamic_info[tag] = (unsigned long) DL_RELOC_ADDR(load_off, dynamic_info[tag]); \
	} while (0)
	/* Don't adjust .dynamic unnecessarily.  For FDPIC targets,
	   we'd have to walk all the loadsegs to find out if it was
	   actually unnecessary, so skip this optimization.  */
#if !defined __FRV_FDPIC__ && !defined __BFIN_FDPIC__ && !defined __DSBT__ && !defined __FDPIC__
	if (load_off != 0)
#endif
	{
		ADJUST_DYN_INFO(DT_HASH, load_off);
		ADJUST_DYN_INFO(DT_PLTGOT, load_off);
		ADJUST_DYN_INFO(DT_STRTAB, load_off);
		ADJUST_DYN_INFO(DT_SYMTAB, load_off);
		ADJUST_DYN_INFO(DT_RELOC_TABLE_ADDR, load_off);
		ADJUST_DYN_INFO(DT_JMPREL, load_off);
#ifdef __LDSO_GNU_HASH_SUPPORT__
		ADJUST_DYN_INFO(DT_GNU_HASH_IDX, load_off);
#endif
	}
#ifdef __DSBT__
	/* Get the mapped address of the DSBT base.  */
	ADJUST_DYN_INFO(DT_DSBT_BASE_IDX, load_off);
#endif
#undef ADJUST_DYN_INFO
	return rtld_flags;
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

/* dlsym() calls _dl_find_hash with this value, that enables
   DL_FIND_HASH_VALUE to return something different than the symbol
   itself, e.g., a function descriptor.  */
#define ELF_RTYPE_CLASS_DLSYM 0x80000000


/* Convert between the Linux flags for page protections and the
   ones specified in the ELF standard. */
#define LXFLAGS(X) ( (((X) & PF_R) ? PROT_READ : 0) | \
		    (((X) & PF_W) ? PROT_WRITE : 0) | \
		    (((X) & PF_X) ? PROT_EXEC : 0))


#endif /* _DL_ELF_H */
