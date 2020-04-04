/* Copyright (C) 2003, 2004 Red Hat, Inc.
 * Contributed by Alexandre Oliva <aoliva@redhat.com>
 * Copyright (C) 2006-2011 Analog Devices, Inc.
 * Based on ../i386/dl-sysdep.h
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define HAVE_DL_INLINES_H

/*
 * Initialization sequence for a GOT.  Copy the resolver function
 * descriptor and the pointer to the elf_resolve/link_map data
 * structure.  Initialize the got_value in the module while at that.
 */
#define INIT_GOT(GOT_BASE,MODULE) \
{				\
  (MODULE)->loadaddr.got_value = (GOT_BASE); \
  GOT_BASE[0] = ((unsigned long *)&_dl_linux_resolve)[0]; \
  GOT_BASE[1] = ((unsigned long *)&_dl_linux_resolve)[1]; \
  GOT_BASE[2] = (unsigned long) MODULE; \
}

struct elf_resolve;

struct funcdesc_value
{
  void *entry_point;
  void *got_value;
} __attribute__((__aligned__(8)));

struct funcdesc_ht;

#define DL_LOADADDR_TYPE struct elf32_fdpic_loadaddr

#define DL_RELOC_ADDR(LOADADDR, ADDR) \
    ((ElfW(Addr))__reloc_pointer ((void*)(ADDR), (LOADADDR).map))

#define DL_ADDR_TO_FUNC_PTR(ADDR, LOADADDR) \
  ((void(*)(void)) _dl_funcdesc_for ((void*)(ADDR), (LOADADDR).got_value))

#define _dl_stabilize_funcdesc(val) \
  ({ __asm__ ("" : "+m" (*(val))); (val); })

#define DL_CALL_FUNC_AT_ADDR(ADDR, LOADADDR, SIGNATURE, ...) \
  ({ struct funcdesc_value fd = { (void*)(ADDR), (LOADADDR).got_value }; \
     void (*pf)(void) = (void*) _dl_stabilize_funcdesc (&fd); \
     (* SIGNATURE pf)(__VA_ARGS__); })

#define DL_INIT_LOADADDR_BOOT(LOADADDR, BASEADDR) \
  (__dl_init_loadaddr_map (&(LOADADDR), dl_boot_got_pointer, \
			   dl_boot_ldsomap ?: dl_boot_progmap))

#define DL_INIT_LOADADDR_PROG(LOADADDR, BASEADDR) \
  (__dl_init_loadaddr_map (&(LOADADDR), 0, dl_boot_progmap))

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
  (__dl_loadaddr_unmap ((LOADADDR), (NULL)))
#define DL_LIB_UNMAP(LIB, LEN) \
  (__dl_loadaddr_unmap ((LIB)->loadaddr, (LIB)->funcdesc_ht))
#define DL_LOADADDR_BASE(LOADADDR) \
  ((LOADADDR).got_value)

/* This is called from dladdr(), such that we map a function
   descriptor's address to the function's entry point before trying to
   find in which library it's defined.  */
#define DL_LOOKUP_ADDRESS(ADDRESS) (_dl_lookup_address (ADDRESS))

#define DL_ADDR_IN_LOADADDR(ADDR, TPNT, TFROM) \
  (! (TFROM) && __dl_addr_in_loadaddr ((void*)(ADDR), (TPNT)->loadaddr))

/*
 * Compute the GOT address.  On several platforms, we use assembly
 * here.  on FDPIC, there's no way to compute the GOT address,
 * since the offset between text and data is not fixed, so we arrange
 * for the ldso assembly entry point to pass this value as an argument
 * to _dl_start.  */
#define DL_BOOT_COMPUTE_GOT(got) ((got) = dl_boot_got_pointer)

#define DL_BOOT_COMPUTE_DYN(dpnt, got, load_addr) \
  ((dpnt) = dl_boot_ldso_dyn_pointer)

/* We want want to apply all relocations in the interpreter during
   bootstrap.  Because of this, we have to skip the interpreter
   relocations in _dl_parse_relocation_information(), see
   elfinterp.c.  */
#define DL_SKIP_BOOTSTRAP_RELOC(SYMTAB, INDEX, STRTAB) 0

#if defined(__NR_pread) || defined(__NR_pread64)
#define _DL_PREAD(FD, BUF, SIZE, OFFSET) \
  (_dl_pread((FD), (BUF), (SIZE), (OFFSET)))
#endif

/* We want to return to dlsym() a function descriptor if the symbol
   turns out to be a function.  */
#define DL_FIND_HASH_VALUE(TPNT, TYPE_CLASS, SYM) \
  (((TYPE_CLASS) & ELF_RTYPE_CLASS_DLSYM) \
   && ELF32_ST_TYPE((SYM)->st_info) == STT_FUNC \
   ? _dl_funcdesc_for ((void *)DL_RELOC_ADDR ((TPNT)->loadaddr, (SYM)->st_value), \
 		       (TPNT)->loadaddr.got_value)			     \
   : DL_RELOC_ADDR ((TPNT)->loadaddr, (SYM)->st_value))

#define DL_GET_READY_TO_RUN_EXTRA_PARMS \
    , struct elf32_fdpic_loadmap *dl_boot_progmap, Elf32_Addr dl_boot_got_pointer
#define DL_GET_READY_TO_RUN_EXTRA_ARGS \
    , dl_boot_progmap, dl_boot_got_pointer

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
