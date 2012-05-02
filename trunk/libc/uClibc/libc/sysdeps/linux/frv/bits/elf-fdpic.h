/* Copyright 2003, 2004 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

In addition to the permissions in the GNU Lesser General Public
License, the Free Software Foundation gives you unlimited
permission to link the compiled version of this file with other
programs, and to distribute those programs without any restriction
coming from the use of this file.  (The GNU Lesser General Public
License restrictions do apply in other respects; for example, they
cover modification of the file, and distribution when not linked
into another program.)

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef _BITS_ELF_FDPIC_H
#define _BITS_ELF_FDPIC_H

/* These data structures are described in the FDPIC ABI extension.
   The kernel passes a process a memory map, such that for every LOAD
   segment there is an elf32_fdpic_loadseg entry.  A pointer to an
   elf32_fdpic_loadmap is passed in GR8 at start-up, and a pointer to
   an additional such map is passed in GR9 for the interpreter, when
   there is one.  */

#include <elf.h>

/* This data structure represents a PT_LOAD segment.  */
struct elf32_fdpic_loadseg
{
  /* Core address to which the segment is mapped.  */
  Elf32_Addr addr;
  /* VMA recorded in the program header.  */
  Elf32_Addr p_vaddr;
  /* Size of this segment in memory.  */
  Elf32_Word p_memsz;
};

struct elf32_fdpic_loadmap {
  /* Protocol version number, must be zero.  */
  Elf32_Half version;
  /* Number of segments in this map.  */
  Elf32_Half nsegs;
  /* The actual memory map.  */
  struct elf32_fdpic_loadseg segs[/*nsegs*/];
};

struct elf32_fdpic_loadaddr {
  struct elf32_fdpic_loadmap *map;
  void *got_value;
};

/* Map a pointer's VMA to its corresponding address according to the
   load map.  */
inline static void *
__reloc_pointer (void *p,
		 const struct elf32_fdpic_loadmap *map)
{
  int c;

#if 0
  if (map->version != 0)
    /* Crash.  */
    ((void(*)())0)();
#endif

  /* No special provision is made for NULL.  We don't want NULL
     addresses to go through relocation, so they shouldn't be in
     .rofixup sections, and, if they're present in dynamic
     relocations, they shall be mapped to the NULL address without
     undergoing relocations.  */

  for (c = 0;
       /* Take advantage of the fact that the loadmap is ordered by
	  virtual addresses.  In general there will only be 2 entries,
	  so it's not profitable to do a binary search.  */
       c < map->nsegs && p >= (void*)map->segs[c].p_vaddr;
       c++)
    {
      /* This should be computed as part of the pointer comparison
	 above, but we want to use the carry in the comparison, so we
	 can't convert it to an integer type beforehand.  */
      unsigned long offset = p - (void*)map->segs[c].p_vaddr;
      /* We only check for one-past-the-end for the last segment,
	 assumed to be the data segment, because other cases are
	 ambiguous in the absence of padding between segments, and
	 rofixup already serves as padding between text and data.
	 Unfortunately, unless we special-case the last segment, we
	 fail to relocate the _end symbol.  */
      if (offset < map->segs[c].p_memsz
	  || (offset == map->segs[c].p_memsz && c + 1 == map->nsegs))
	return (char*)map->segs[c].addr + offset;
    }
	     
  /* We might want to crash instead.  */
  return (void*)-1;
}

# define __RELOC_POINTER(ptr, loadaddr) \
  (__reloc_pointer ((void*)(ptr), \
		    (loadaddr).map))

#endif /* _BITS_ELF_FDPIC_H */
