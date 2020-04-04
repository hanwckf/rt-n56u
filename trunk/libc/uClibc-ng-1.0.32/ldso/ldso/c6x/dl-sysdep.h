/* Copyright (C) 2010 Texas Instruments Incorporated
 * Contributed by Mark Salter <msalter@redhat.com>
 *
 * Borrowed heavily from frv arch:
 * Copyright (C) 2003, 2004 Red Hat, Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <bits/elf-dsbt.h>

/*
 * Define this if the system uses RELOCA.
 */
#define ELF_USES_RELOCA 1

/* JMPREL relocs are inside the DT_RELA table.  */
/* Actually looks like a linker bug sets DT_JMPREL anyway */
#define ELF_MACHINE_PLTREL_OVERLAP 1

#undef DL_NO_COPY_RELOCS

#define HAVE_DL_INLINES_H


/*
 * Various assembly language/system dependent  hacks that are required
 * so that we can minimize the amount of platform specific code.
 */

/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE) \
{ \
  GOT_BASE[0] = (unsigned long) _dl_linux_resolve; \
  GOT_BASE[1] = (unsigned long) MODULE; \
}

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_TI_C6000
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "C6000"

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

struct elf_resolve;

extern int _dl_linux_resolve(void) attribute_hidden;

struct funcdesc_ht;
struct elf32_dsbt_loadaddr;

/* Current toolchains access constant strings via unrelocated GOT
   entries. Fortunately, we have enough in place to just call the
   relocation function early on. */
#undef SEND_EARLY_STDERR
#define SEND_EARLY_STDERR(S) \
  do { char *__p = __reloc_pointer((S), dl_boot_ldsomap?:dl_boot_progmap);\
	  SEND_STDERR (__p); } while (0)

#define DL_LOADADDR_TYPE struct elf32_dsbt_loadaddr

#define DL_RELOC_ADDR(LOADADDR, ADDR) \
  ((ElfW(Addr))__reloc_pointer ((void*)(ADDR), (LOADADDR).map))

#define DL_INIT_LOADADDR_BOOT(LOADADDR, BASEADDR) \
	do {						  		\
	    struct elf32_dsbt_loadmap *map;				\
	    map = dl_boot_ldsomap ?: dl_boot_progmap;			\
	    if (map->version != 0) {			  		\
	        SEND_EARLY_STDERR ("Invalid loadmap version number\n"); \
                _dl_exit(-1);						\
	    }								\
	    if (map->nsegs < 2) {					\
	        SEND_EARLY_STDERR ("Invalid segment count in loadmap\n"); \
                _dl_exit(-1);						\
	    }								\
	    (LOADADDR).map = map;					\
	} while(0)

#define DL_INIT_LOADADDR_PROG(LOADADDR, BASEADDR) \
	do {						  		\
	    if (dl_boot_progmap->version != 0) {	  		\
	        SEND_EARLY_STDERR ("Invalid loadmap version number\n"); \
                _dl_exit(-1);						\
	    }								\
	    if (dl_boot_progmap->nsegs < 2) {				\
	        SEND_EARLY_STDERR ("Invalid segment count in loadmap\n"); \
                _dl_exit(-1);						\
	    }								\
	    (LOADADDR).map = dl_boot_progmap;				\
	} while(0)

#define DL_INIT_LOADADDR_EXTRA_DECLS \
  int dl_init_loadaddr_load_count;

#define DL_INIT_LOADADDR(LOADADDR, BASEADDR, PHDR, PHDRCNT) \
  (dl_init_loadaddr_load_count = \
     __dl_init_loadaddr (&(LOADADDR), (PHDR), (PHDRCNT)))

#define DL_INIT_LOADADDR_HDR(LOADADDR, ADDR, PHDR) \
  (__dl_init_loadaddr_hdr ((LOADADDR), (ADDR), (PHDR), \
			   dl_init_loadaddr_load_count))

#define DL_UPDATE_LOADADDR_HDR(LOADADDR, ADDR, PHDR) \
  (__dl_update_loadaddr_hdr ((LOADADDR), (ADDR), (PHDR)))

#define DL_LOADADDR_UNMAP(LOADADDR, LEN) \
  (__dl_loadaddr_unmap ((LOADADDR)))

#define DL_LIB_UNMAP(LIB, LEN) \
  (__dl_loadaddr_unmap ((LIB)->loadaddr))

#define DL_LOADADDR_BASE(LOADADDR) \
  ((LOADADDR).map)

#define DL_ADDR_IN_LOADADDR(ADDR, TPNT, TFROM) \
  (! (TFROM) && __dl_addr_in_loadaddr ((void*)(ADDR), (TPNT)->loadaddr))


/* We only support loading DSBT relocatable shared libraries.
   It probably wouldn't be too hard to support loading statically
   linked executables that require relocation.*/
#define DL_CHECK_LIB_TYPE(epnt, piclib, _dl_progname, libname) \
do \
{ \
    (piclib) = 2; \
} \
while (0)

/* We want want to apply all relocations in the interpreter during
   bootstrap.  Because of this, we have to skip the interpreter
   relocations in _dl_parse_relocation_information(), see
   elfinterp.c.  */
#define DL_SKIP_BOOTSTRAP_RELOC(SYMTAB, INDEX, STRTAB) 0

#ifdef __NR_pread64
#define _DL_PREAD(FD, BUF, SIZE, OFFSET) \
  (_dl_pread((FD), (BUF), (SIZE), (OFFSET)))
#endif

#define DL_GET_READY_TO_RUN_EXTRA_PARMS \
  , struct elf32_dsbt_loadmap *dl_boot_progmap \
  , struct elf32_dsbt_loadmap *dl_boot_ldsomap
#define DL_GET_READY_TO_RUN_EXTRA_ARGS \
  , dl_boot_progmap \
  , dl_boot_ldsomap


/*
 * C6X doesn't really need the GOT here.
 * The GOT is placed just past the DSBT table, so we could find it by
 * using the DSBT register + table size found in the dynamic section.
 *
 *	do {						  		\
 *		unsigned long *ldso_dsbt;				\
 *		ElfW(Dyn) *d = dl_boot_ldso_dyn_pointer;		\
 *		while (d->d_tag != DT_NULL) {				\
 *			if (d->d_tag == DT_C6000_DSBT_SIZE)	{	\
 *				__asm__ (" MV .S2 B14,%0\n"		\
 *				     : "=b" (ldso_dsbt));		\
 *				(GOT) = ldso_dsbt + d->d_un.d_val;	\
 *				break;					\
 *			}						\
 *			d++;						\
 *		}							\
 *	} while(0)
 *
 * Instead, just point it to the DSBT table to avoid unused variable warning.
 */
#define DL_BOOT_COMPUTE_GOT(GOT) \
	__asm__ (" MV .S2 B14,%0\n" : "=b" (GOT))

#define DL_BOOT_COMPUTE_DYN(dpnt, got, load_addr) \
  ((dpnt) = dl_boot_ldso_dyn_pointer)

/* Define this to declare the library offset. */
#define DL_DEF_LIB_OFFSET

/* Define this to get the library offset. */
#define DL_GET_LIB_OFFSET() 0

/* Define this to set the library offset. */
#define DL_SET_LIB_OFFSET(offset)

/* Define this to get the real object's runtime address. */
#define DL_GET_RUN_ADDR(loadaddr, mapaddr) (loadaddr)

#ifdef __USE_GNU
# include <link.h>
#else
# define __USE_GNU
# include <link.h>
# undef __USE_GNU
#endif

/* we need this for __LDSO_STANDALONE_SUPPORT__ */
#define elf_machine_load_address() \
	(dl_boot_ldsomap ?: dl_boot_progmap)->segs[0].addr

static __always_inline void
elf_machine_relative (DL_LOADADDR_TYPE load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
}

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_C6000_JUMP_SLOT) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_C6000_COPY) * ELF_RTYPE_CLASS_COPY))

#define ARCH_NUM 3
#define DT_DSBT_BASE_IDX	(DT_NUM + OS_NUM)
#define DT_DSBT_SIZE_IDX	(DT_NUM + OS_NUM + 1)
#define DT_DSBT_INDEX_IDX	(DT_NUM + OS_NUM + 2)

#define ARCH_DYNAMIC_INFO(dpnt,  dynamic, debug_addr) \
do { \
if (dpnt->d_tag == DT_C6000_DSBT_BASE) \
     dynamic[DT_DSBT_BASE_IDX] = dpnt->d_un.d_val; \
else if (dpnt->d_tag == DT_C6000_DSBT_SIZE) \
     dynamic[DT_DSBT_SIZE_IDX] = dpnt->d_un.d_val; \
else if (dpnt->d_tag == DT_C6000_DSBT_INDEX) \
     dynamic[DT_DSBT_INDEX_IDX] = dpnt->d_un.d_val; \
} while (0)
