/* Copyright (C) 2003, 2004 Red Hat, Inc.
 * Contributed by Alexandre Oliva <aoliva@redhat.com>
 * Based on ../i386/dl-sysdep.h
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

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

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_CYGNUS_FRV
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "FR-V"

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

extern int _dl_linux_resolve(void) __attribute__((__visibility__("hidden")));

/* We must force strings used early in the bootstrap into the data
   segment, such that they are referenced with GOTOFF instead of
   GPREL, because GPREL needs the GOT to have already been
   relocated.  */
#undef SEND_EARLY_STDERR
#define SEND_EARLY_STDERR(S) \
  do { static char __s[] = (S); SEND_STDERR (__s); } while (0)

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

#include "../fdpic/dl-sysdep.h"

static __always_inline Elf32_Addr
elf_machine_load_address (void)
{
	return 0;
}

static __always_inline void
elf_machine_relative (DL_LOADADDR_TYPE load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	;
}
