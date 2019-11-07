/* Copyright (C) 2003, 2004 Red Hat, Inc.
 * Contributed by Alexandre Oliva <aoliva@redhat.com>
 * Copyright (C) 2006-2011 Analog Devices, Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <bfin_sram.h>

#define __dl_loadaddr_unmap __dl_loadaddr_unmap

#include "../fdpic/dl-inlines.h"

static __always_inline void
__dl_loadaddr_unmap(struct elf32_fdpic_loadaddr loadaddr,
                    struct funcdesc_ht *funcdesc_ht)
{
	int i;

	for (i = 0; i < loadaddr.map->nsegs; i++) {
		struct elf32_fdpic_loadseg *segdata;
		ssize_t offs;
		segdata = loadaddr.map->segs + i;

		/* FIXME:
		 * A more cleaner way is to add type for struct elf32_fdpic_loadseg,
		 * and release the memory according to the type.
		 * Currently, we hardcode the memory address of L1 SRAM.
		 */
		if ((segdata->addr & 0xff800000) == 0xff800000) {
			_dl_sram_free((void *)segdata->addr);
			continue;
		}

		offs = (segdata->p_vaddr & ADDR_ALIGN);
		_dl_munmap((void*)segdata->addr - offs,
			segdata->p_memsz + offs);
	  }

	/*
	 * _dl_unmap is only called for dlopen()ed libraries, for which
	 * calling free() is safe, or before we've completed the initial
	 * relocation, in which case calling free() is probably pointless,
	 * but still safe.
	 */
	_dl_free(loadaddr.map);
	if (funcdesc_ht)
		htab_delete(funcdesc_ht);
}

static __always_inline int
__dl_is_special_segment(Elf32_Ehdr *epnt, Elf32_Phdr *ppnt)
{
	if (ppnt->p_type != PT_LOAD)
		return 0;

	/* Allow read-only executable segments to be loaded into L1 inst */
	if ((epnt->e_flags & EF_BFIN_CODE_IN_L1) &&
	    !(ppnt->p_flags & PF_W) && (ppnt->p_flags & PF_X))
		return 1;

	/* Allow writable non-executable segments to be loaded into L1 data */
	if ((epnt->e_flags & EF_BFIN_DATA_IN_L1) &&
	    (ppnt->p_flags & PF_W) && !(ppnt->p_flags & PF_X))
		return 1;

	/*
	 * These L1 memory addresses are also used in GNU ld and linux kernel.
	 * They need to be kept synchronized.
	 */
	switch (ppnt->p_vaddr) {
	case 0xff700000:
	case 0xff800000:
	case 0xff900000:
	case 0xffa00000:
	case 0xfeb00000:
	case 0xfec00000:
		return 1;
	default:
		return 0;
	}
}

static __always_inline char *
__dl_map_segment(Elf32_Ehdr *epnt, Elf32_Phdr *ppnt, int infile, int flags)
{
	void *addr;
	unsigned long sram_flags = 0;

	/* Handle L1 inst mappings */
	if (((epnt->e_flags & EF_BFIN_CODE_IN_L1) || ppnt->p_vaddr == 0xffa00000) &&
	    !(ppnt->p_flags & PF_W) && (ppnt->p_flags & PF_X))
	{
		size_t size = (ppnt->p_vaddr & ADDR_ALIGN) + ppnt->p_filesz;
		void *status = _dl_mmap(NULL, size, LXFLAGS(ppnt->p_flags),
			flags | MAP_EXECUTABLE | MAP_DENYWRITE,
			infile, ppnt->p_offset & OFFS_ALIGN);
		if (_dl_mmap_check_error(status))
			return NULL;

		addr = _dl_sram_alloc(ppnt->p_filesz, L1_INST_SRAM);
		if (addr)
			_dl_dma_memcpy(addr, status + (ppnt->p_vaddr & ADDR_ALIGN), ppnt->p_filesz);
		else
			_dl_dprintf(2, "%s:%s: sram allocation %#x failed\n",
				_dl_progname, __func__, ppnt->p_vaddr);

		_dl_munmap(status, size);
		return addr;
	}

	/* Handle L1 data mappings */
	if (((epnt->e_flags & EF_BFIN_DATA_IN_L1) ||
	     ppnt->p_vaddr == 0xff700000 ||
	     ppnt->p_vaddr == 0xff800000 ||
	     ppnt->p_vaddr == 0xff900000) &&
	    (ppnt->p_flags & PF_W) && !(ppnt->p_flags & PF_X))
	{
		switch (ppnt->p_vaddr) {
		case 0xff800000: sram_flags = L1_DATA_A_SRAM; break;
		case 0xff900000: sram_flags = L1_DATA_B_SRAM; break;
		default:         sram_flags = L1_DATA_SRAM;   break;
		}
	}

	/* Handle L2 mappings */
	if (ppnt->p_vaddr == 0xfeb00000 || ppnt->p_vaddr == 0xfec00000)
		sram_flags = L2_SRAM;

	if (sram_flags) {
		addr = _dl_sram_alloc(ppnt->p_memsz, sram_flags);
		if (addr) {
			if (_DL_PREAD(infile, addr, ppnt->p_filesz, ppnt->p_offset) != ppnt->p_filesz) {
				_dl_sram_free(addr);
				return NULL;
			}
			if (ppnt->p_filesz < ppnt->p_memsz)
				_dl_memset(addr + ppnt->p_filesz, 0, ppnt->p_memsz - ppnt->p_filesz);
		} else
			_dl_dprintf(2, "%s:%s: sram allocation %#x failed\n",
				_dl_progname, __func__, ppnt->p_vaddr);
		return addr;
	}

	return 0;
}
