/* Copyright (C) 2010 Texas Instruments Incorporated
 * Contributed by Mark Salter <msalter@redhat.com>
 *
 * Borrowed heavily from frv arch:
 * Copyright (C) 2003, 2004 Red Hat, Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Figure out whether the given address is in one of the mapped
   segments.  */
static __always_inline int
__dl_addr_in_loadaddr (void *p, struct elf32_dsbt_loadaddr loadaddr)
{
	struct elf32_dsbt_loadmap *map = loadaddr.map;
	int c;

	for (c = 0; c < map->nsegs; c++)
		if ((void*)map->segs[c].addr <= p
		    && (char*)p < (char*)map->segs[c].addr + map->segs[c].p_memsz)
			return 1;

	return 0;
}

/* Figure out how many LOAD segments there are in the given headers,
   and allocate a block for the load map big enough for them.
   got_value will be properly initialized later on, with INIT_GOT.  */
static __always_inline int
__dl_init_loadaddr (struct elf32_dsbt_loadaddr *loadaddr, Elf32_Phdr *ppnt,
		    int pcnt)
{
	int count = 0, i;
	size_t size;

	for (i = 0; i < pcnt; i++)
		if (ppnt[i].p_type == PT_LOAD)
			count++;

	size = sizeof (struct elf32_dsbt_loadmap)
		+ sizeof (struct elf32_dsbt_loadseg) * count;
	loadaddr->map = _dl_malloc (size);
	if (! loadaddr->map)
		_dl_exit (-1);

	loadaddr->map->version = 0;
	loadaddr->map->nsegs = 0;

	return count;
}

/* Incrementally initialize a load map.  */
static __always_inline void
__dl_init_loadaddr_hdr (struct elf32_dsbt_loadaddr loadaddr, void *addr,
			Elf32_Phdr *phdr, int maxsegs)
{
	struct elf32_dsbt_loadseg *segdata;

	if (loadaddr.map->nsegs == maxsegs)
		_dl_exit (-1);

	segdata = &loadaddr.map->segs[loadaddr.map->nsegs++];
	segdata->addr = (Elf32_Addr) addr;
	segdata->p_vaddr = phdr->p_vaddr;
	segdata->p_memsz = phdr->p_memsz;

#if defined (__SUPPORT_LD_DEBUG__)
	{
		if (_dl_debug)
			_dl_dprintf(_dl_debug_file, "%i: mapped %x at %x, size %x\n",
				    loadaddr.map->nsegs-1,
				    segdata->p_vaddr, segdata->addr, segdata->p_memsz);
	}
#endif
}

/* Replace an existing entry in the load map.  */
static __always_inline void
__dl_update_loadaddr_hdr (struct elf32_dsbt_loadaddr loadaddr, void *addr,
			Elf32_Phdr *phdr)
{
 	struct elf32_dsbt_loadseg *segdata;
	void *oldaddr;
 	int i;

	for (i = 0; i < loadaddr.map->nsegs; i++)
		if (loadaddr.map->segs[i].p_vaddr == phdr->p_vaddr
		    && loadaddr.map->segs[i].p_memsz == phdr->p_memsz)
			break;
	if (i == loadaddr.map->nsegs)
		_dl_exit (-1);

	segdata = loadaddr.map->segs + i;
	oldaddr = (void *)segdata->addr;
	_dl_munmap (oldaddr, segdata->p_memsz);
	segdata->addr = (Elf32_Addr) addr;

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug)
		_dl_dprintf(_dl_debug_file, "%i: changed mapping %x at %x (old %x), size %x\n",
			    loadaddr.map->nsegs-1,
			    segdata->p_vaddr, segdata->addr, oldaddr, segdata->p_memsz);
#endif
}

static __always_inline void
__dl_loadaddr_unmap (struct elf32_dsbt_loadaddr loadaddr)
{
	int i;

	for (i = 0; i < loadaddr.map->nsegs; i++)
		_dl_munmap ((void*)loadaddr.map->segs[i].addr,
			    loadaddr.map->segs[i].p_memsz);

	/* _dl_unmap is only called for dlopen()ed libraries, for which
	   calling free() is safe, or before we've completed the initial
	   relocation, in which case calling free() is probably pointless,
	   but still safe.  */
	_dl_free (loadaddr.map);
}
