/* Copyright (C) 2003, 2004 Red Hat, Inc.
 * Contributed by Alexandre Oliva <aoliva@redhat.com>
 * Copyright (C) 2006-2011 Analog Devices, Inc.
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
#define MAGIC1 EM_BLACKFIN
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "BFIN"

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

extern int _dl_linux_resolve(void) __attribute__((__visibility__("hidden")));

#undef SEND_EARLY_STDERR
#define SEND_EARLY_STDERR(S)			\
    do {								\
	static const char __attribute__((section(".text"))) __s[] = (S); \
      const char *__p, *__scratch;					\
      __asm__ ("call 1f;\n1:\n\t"						\
	   "%1 = RETS;\n\t"						\
	   "%0 = [%3 + 1b@GOT17M4];\n\t"				\
	   "%1 = %1 - %0;\n\t"						\
	   "%1 = %1 + %2;\n\t"						\
	   : "=&d" (__scratch), "=&d" (__p)				\
	   : "d" (__s), "a" (dl_boot_got_pointer) : "RETS");				\
      SEND_STDERR (__p);						\
      {	int __t;							\
	  for (__t = 0; __t < 0x1000000; __t++) __asm__ __volatile__ ("");	} \
  } while (0)

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
  if (((epnt)->e_flags & EF_BFIN_FDPIC) && ! ((epnt)->e_flags & EF_BFIN_PIC)) \
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
} \
while (0)

#define DL_IS_SPECIAL_SEGMENT(EPNT, PPNT) \
  __dl_is_special_segment(EPNT, PPNT)
#define DL_MAP_SEGMENT(EPNT, PPNT, INFILE, FLAGS) \
  __dl_map_segment (EPNT, PPNT, INFILE, FLAGS)

#if defined(__BFIN_FDPIC__)
#include "../fdpic/dl-sysdep.h"

static __always_inline Elf32_Addr
elf_machine_load_address (void)
{
	/* this is never an issue on Blackfin systems, so screw it */
	return 0;
}

static __always_inline void
elf_machine_relative (DL_LOADADDR_TYPE load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	return;
}
#endif
