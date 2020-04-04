/* Copyright (C) 2003, 2004 Red Hat, Inc.
 * Contributed by Alexandre Oliva <aoliva@redhat.com>
 * Copyright (C) 2006-2011 Analog Devices, Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <inline-hashtab.h>

static __always_inline void htab_delete(struct funcdesc_ht *htab);

/* Initialize a DL_LOADADDR_TYPE given a got pointer and a complete load map. */
static __always_inline void
__dl_init_loadaddr_map(struct elf32_fdpic_loadaddr *loadaddr, Elf32_Addr dl_boot_got_pointer,
                       struct elf32_fdpic_loadmap *map)
{
	if (map->version != 0) {
		SEND_EARLY_STDERR("Invalid loadmap version number\n");
		_dl_exit(-1);
	}
	if (map->nsegs == 0) {
		SEND_EARLY_STDERR("Invalid segment count in loadmap\n");
		_dl_exit(-1);
	}
	loadaddr->got_value = (void *)dl_boot_got_pointer;
	loadaddr->map = map;
}

/*
 * Figure out how many LOAD segments there are in the given headers,
 * and allocate a block for the load map big enough for them.
 * got_value will be properly initialized later on, with INIT_GOT.
 */
static __always_inline int
__dl_init_loadaddr(struct elf32_fdpic_loadaddr *loadaddr, Elf32_Phdr *ppnt,
                   int pcnt)
{
	int count = 0, i;
	size_t size;

	for (i = 0; i < pcnt; i++)
		if (ppnt[i].p_type == PT_LOAD)
			count++;

	loadaddr->got_value = 0;

	size = sizeof(struct elf32_fdpic_loadmap) +
		(sizeof(struct elf32_fdpic_loadseg) * count);
	loadaddr->map = _dl_malloc(size);
	if (!loadaddr->map)
		_dl_exit(-1);

	loadaddr->map->version = 0;
	loadaddr->map->nsegs = 0;

	return count;
}

/* Incrementally initialize a load map. */
static __always_inline void
__dl_init_loadaddr_hdr(struct elf32_fdpic_loadaddr loadaddr, void *addr,
                       Elf32_Phdr *phdr, int maxsegs)
{
	struct elf32_fdpic_loadseg *segdata;

	if (loadaddr.map->nsegs == maxsegs)
		_dl_exit(-1);

	segdata = &loadaddr.map->segs[loadaddr.map->nsegs++];
	segdata->addr = (Elf32_Addr)addr;
	segdata->p_vaddr = phdr->p_vaddr;
	segdata->p_memsz = phdr->p_memsz;

#if defined(__SUPPORT_LD_DEBUG__)
	if (_dl_debug)
		_dl_dprintf(_dl_debug_file, "%i: mapped %x at %x, size %x\n",
			loadaddr.map->nsegs - 1,
			segdata->p_vaddr, segdata->addr, segdata->p_memsz);
#endif
}

/* Replace an existing entry in the load map. */
static __always_inline void
__dl_update_loadaddr_hdr(struct elf32_fdpic_loadaddr loadaddr, void *addr,
                         Elf32_Phdr *phdr)
{
	struct elf32_fdpic_loadseg *segdata;
	void *oldaddr;
	int i;

	for (i = 0; i < loadaddr.map->nsegs; i++)
		if (loadaddr.map->segs[i].p_vaddr == phdr->p_vaddr &&
		    loadaddr.map->segs[i].p_memsz == phdr->p_memsz)
			break;
	if (i == loadaddr.map->nsegs)
		_dl_exit(-1);

	segdata = loadaddr.map->segs + i;
	oldaddr = (void *)segdata->addr;
	_dl_munmap(oldaddr, segdata->p_memsz);
	segdata->addr = (Elf32_Addr)addr;

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug)
		_dl_dprintf(_dl_debug_file, "%i: changed mapping %x at %x (old %x), size %x\n",
			loadaddr.map->nsegs - 1,
			segdata->p_vaddr, segdata->addr, oldaddr, segdata->p_memsz);
#endif
}


#ifndef __dl_loadaddr_unmap
static __always_inline void
__dl_loadaddr_unmap(struct elf32_fdpic_loadaddr loadaddr,
                    struct funcdesc_ht *funcdesc_ht)
{
	int i;

	for (i = 0; i < loadaddr.map->nsegs; i++)
		_dl_munmap((void *)loadaddr.map->segs[i].addr,
			loadaddr.map->segs[i].p_memsz);

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
#endif

/* Figure out whether the given address is in one of the mapped segments. */
static __always_inline int
__dl_addr_in_loadaddr(void *p, struct elf32_fdpic_loadaddr loadaddr)
{
	struct elf32_fdpic_loadmap *map = loadaddr.map;
	int c;

	for (c = 0; c < map->nsegs; c++)
		if ((void *)map->segs[c].addr <= p &&
		    (char *)p < (char *)map->segs[c].addr + map->segs[c].p_memsz)
			return 1;

	return 0;
}

static int
hash_pointer(void *p)
{
	return (int) ((long)p >> 3);
}

static int
eq_pointer(void *p, void *q)
{
	struct funcdesc_value *entry = p;

	return entry->entry_point == q;
}

static __always_inline void *
_dl_funcdesc_for (void *entry_point, void *got_value)
{
	struct elf_resolve *tpnt = ((void**)got_value)[2];
	struct funcdesc_ht *ht = tpnt->funcdesc_ht;
	struct funcdesc_value **entry;

	_dl_assert(got_value == tpnt->loadaddr.got_value);

	if (!ht) {
		ht = htab_create();
		if (!ht)
			return (void*)-1;
		tpnt->funcdesc_ht = ht;
	}

	entry = htab_find_slot(ht, entry_point, 1, hash_pointer, eq_pointer);

	if (entry == NULL)
		_dl_exit(1);

	if (*entry) {
		_dl_assert((*entry)->entry_point == entry_point);
		return _dl_stabilize_funcdesc(*entry);
	}

	*entry = _dl_malloc(sizeof(**entry));
	(*entry)->entry_point = entry_point;
	(*entry)->got_value = got_value;

	return _dl_stabilize_funcdesc(*entry);
}

static __always_inline void const *
_dl_lookup_address(void const *address)
{
	struct elf_resolve *rpnt;
	struct funcdesc_value const *fd;

	/* Make sure we don't make assumptions about its alignment.  */
	__asm__ ("" : "+r" (address));

	if ((Elf32_Addr)address & 7)
		/* It's not a function descriptor.  */
		return address;

	fd = address;

	for (rpnt = _dl_loaded_modules; rpnt; rpnt = rpnt->next) {
		if (!rpnt->funcdesc_ht)
			continue;

		if (fd->got_value != rpnt->loadaddr.got_value)
			continue;

		address = htab_find_slot(rpnt->funcdesc_ht, (void *)fd->entry_point, 0,
				hash_pointer, eq_pointer);

		if (address && *(struct funcdesc_value *const*)address == fd) {
			address = (*(struct funcdesc_value *const*)address)->entry_point;
			break;
		} else
			address = fd;
	}

	return address;
}
