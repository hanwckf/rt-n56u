     /* Copyright (C) 2003, 2004 Red Hat, Inc.
	Contributed by Alexandre Oliva <aoliva@redhat.com>
	Based on ../i386/dl-sysdep.h

This file is part of uClibc.

uClibc is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

uClibc is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with uClibc; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
USA.  */

/*
 * Various assembly language/system dependent  hacks that are required
 * so that we can minimize the amount of platform specific code.
 */

/*
 * Define this if the system uses RELOCA.
 */
#undef ELF_USES_RELOCA

/* JMPREL relocs are inside the DT_RELA table.  */
#define ELF_MACHINE_PLTREL_OVERLAP

#define DL_NO_COPY_RELOCS

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

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_CYGNUS_FRV
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "FR-V"

struct elf_resolve;

struct funcdesc_value
{
  void *entry_point;
  void *got_value;
} __attribute__((__aligned__(8)));


extern int _dl_linux_resolve(void) __attribute__((__visibility__("hidden")));

/* 16KiB page alignment.  Should perhaps be made dynamic using
   getpagesize(), based on AT_PAGESZ from auxvt?  */
#define PAGE_ALIGN 0xffffc000
#define ADDR_ALIGN 0x3fff
#define OFFS_ALIGN 0x7fffc000

struct funcdesc_ht;

/* We must force strings used early in the bootstrap into the data
   segment, such that they are referenced with GOTOFF instead of
   GPREL, because GPREL needs the GOT to have already been
   relocated.  */
#undef SEND_EARLY_STDERR
#define SEND_EARLY_STDERR(S) \
  do { static char __s[] = (S); SEND_STDERR (__s); } while (0)

#define DL_LOADADDR_TYPE struct elf32_fdpic_loadaddr

#define DL_RELOC_ADDR(ADDR, LOADADDR) \
  (__reloc_pointer ((void*)(ADDR), (LOADADDR).map))

#define DL_ADDR_TO_FUNC_PTR(ADDR, LOADADDR) \
  ((void(*)(void)) _dl_funcdesc_for ((void*)(ADDR), (LOADADDR).got_value))

#define _dl_stabilize_funcdesc(val) \
  ({ asm ("" : "+m" (*(val))); (val); })

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

/* Make sure we only load libraries that use the same number of
   general-purpose and floating-point registers the dynamic loader was
   compiled for.  */
#define DL_CHECK_REG_COUNT(flags) \
  (((flags & EF_FRV_GPR_MASK) == EF_FRV_GPR_32 ? __FRV_GPR__ == 32 : 1) \
   && ((flags & EF_FRV_GPR_MASK) == EF_FRV_GPR_64 ? __FRV_GPR__ == 64 : 1) \
   && ((flags & EF_FRV_FPR_MASK) == EF_FRV_FPR_32 ? __FRV_FPR__ == 32 : 1) \
   && ((flags & EF_FRV_FPR_MASK) == EF_FRV_FPR_64 ? __FRV_FPR__ == 64 : 1) \
   && ((flags & EF_FRV_FPR_MASK) == EF_FRV_FPR_NONE ? __FRV_FPR__ == 0 : 1))

/* We only support loading FDPIC independently-relocatable shared
   libraries.  It probably wouldn't be too hard to support loading
   shared libraries that require relocation by the same amount, but we
   don't know that they exist or would be useful, and the dynamic
   loader code could leak the whole-library map unless we keeping a
   bit more state for DL_LOADADDR_UNMAP and DL_LIB_UNMAP, so let's
   keep things simple for now.  */
#define DL_CHECK_LIB_TYPE(epnt, piclib, _dl_progname, libname) \
do \
{ \
  if (((epnt)->e_flags & EF_FRV_FDPIC) && ! ((epnt)->e_flags & EF_FRV_PIC)) \
    (piclib) = 2; \
  else \
    { \
      _dl_internal_error_number = LD_ERROR_NOTDYN; \
      _dl_dprintf(2, "%s: '%s' is not an FDPIC shared library" \
		  "\n", (_dl_progname), (libname)); \
      _dl_close(infile); \
      return NULL; \
    } \
\
  if (! DL_CHECK_REG_COUNT ((epnt)->e_flags)) \
    { \
      _dl_internal_error_number = LD_ERROR_NOTDYN; \
      _dl_dprintf(2, "%s: '%s' assumes different register counts" \
		  "\n", (_dl_progname), (libname)); \
      _dl_close(infile); \
    } \
} \
while (0)  

/* We want want to apply all relocations in the interpreter during
   bootstrap.  Because of this, we have to skip the interpreter
   relocations in _dl_parse_relocation_information(), see
   elfinterp.c.  */
#define DL_SKIP_BOOTSTRAP_RELOC(SYMTAB, INDEX, STRTAB) 0

#ifdef __NR_pread
#define _DL_PREAD(FD, BUF, SIZE, OFFSET) \
  (_dl_pread((FD), (BUF), (SIZE), (OFFSET)))
#endif

/* We want to return to dlsym() a function descriptor if the symbol
   turns out to be a function.  */
#define DL_FIND_HASH_VALUE(TPNT, TYPE_CLASS, SYM) \
  (((TYPE_CLASS) & ELF_RTYPE_CLASS_DLSYM) \
   && ELF32_ST_TYPE((SYM)->st_info) == STT_FUNC \
   ? _dl_funcdesc_for (DL_RELOC_ADDR ((SYM)->st_value, (TPNT)->loadaddr),    \
 		       (TPNT)->loadaddr.got_value)			     \
   : DL_RELOC_ADDR ((SYM)->st_value, (TPNT)->loadaddr))

#define DL_GET_READY_TO_RUN_EXTRA_PARMS \
  , struct elf32_fdpic_loadmap *dl_boot_progmap
#define DL_GET_READY_TO_RUN_EXTRA_ARGS \
  , dl_boot_progmap

	  


#ifdef __USE_GNU
# include <link.h>
#else
# define __USE_GNU
# include <link.h>
# undef __USE_GNU
#endif
