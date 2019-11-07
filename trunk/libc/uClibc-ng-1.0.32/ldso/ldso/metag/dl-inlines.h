/*
 * Copyright (C) 2013, Imagination Technologies Ltd.
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

static __always_inline int
__dl_is_special_segment (Elf32_Ehdr *epnt,
			 Elf32_Phdr *ppnt)
{
  if (ppnt->p_type != PT_LOAD &&
      ppnt->p_type != PT_DYNAMIC)
    return 0;

  if (ppnt->p_vaddr >= 0x80000000 &&
      ppnt->p_vaddr < 0x82060000)
    return 1;

  if (ppnt->p_vaddr >= 0xe0200000 &&
      ppnt->p_vaddr < 0xe0260000)
    return 1;

  return 0;
}

static __always_inline char *
__dl_map_segment (Elf32_Ehdr *epnt,
		  Elf32_Phdr *ppnt,
		  int infile,
		  int flags)
{
  char *addr = (char *)ppnt->p_vaddr;

  if (_DL_PREAD (infile, addr, ppnt->p_filesz, ppnt->p_offset) != ppnt->p_filesz) {
    return 0;
  }

  return addr;
}
