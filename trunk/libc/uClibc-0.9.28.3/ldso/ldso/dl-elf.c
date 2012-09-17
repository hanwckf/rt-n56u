/* vi: set sw=4 ts=4: */
/*
 * This file contains the helper routines to load an ELF shared
 * library into memory and add the symbol table info to the chain.
 *
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@codepoet.org>
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald,
 *				David Engel, Hongjiu Lu and Mitch D'Souza
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the above contributors may not be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include "ldso.h"

#ifdef __LDSO_CACHE_SUPPORT__

static caddr_t _dl_cache_addr = NULL;
static size_t _dl_cache_size = 0;

int _dl_map_cache(void)
{
	int fd;
	struct stat st;
	header_t *header;
	libentry_t *libent;
	int i, strtabsize;

	if (_dl_cache_addr == (caddr_t) - 1)
		return -1;
	else if (_dl_cache_addr != NULL)
		return 0;

	if (_dl_stat(LDSO_CACHE, &st)
	    || (fd = _dl_open(LDSO_CACHE, O_RDONLY, 0)) < 0) {
		_dl_cache_addr = (caddr_t) - 1;	/* so we won't try again */
		return -1;
	}

	_dl_cache_size = st.st_size;
	_dl_cache_addr = (caddr_t) _dl_mmap(0, _dl_cache_size, PROT_READ, MAP_SHARED, fd, 0);
	_dl_close(fd);
	if (_dl_mmap_check_error(_dl_cache_addr)) {
		_dl_dprintf(2, "%s:%i: can't map '%s'\n",
				_dl_progname, __LINE__, LDSO_CACHE);
		return -1;
	}

	header = (header_t *) _dl_cache_addr;

	if (_dl_cache_size < sizeof(header_t) ||
			_dl_memcmp(header->magic, LDSO_CACHE_MAGIC, LDSO_CACHE_MAGIC_LEN)
			|| _dl_memcmp(header->version, LDSO_CACHE_VER, LDSO_CACHE_VER_LEN)
			|| _dl_cache_size <
			(sizeof(header_t) + header->nlibs * sizeof(libentry_t))
			|| _dl_cache_addr[_dl_cache_size - 1] != '\0')
	{
		_dl_dprintf(2, "%s: cache '%s' is corrupt\n", _dl_progname,
				LDSO_CACHE);
		goto fail;
	}

	strtabsize = _dl_cache_size - sizeof(header_t) -
		header->nlibs * sizeof(libentry_t);
	libent = (libentry_t *) & header[1];

	for (i = 0; i < header->nlibs; i++) {
		if (libent[i].sooffset >= strtabsize ||
				libent[i].liboffset >= strtabsize)
		{
			_dl_dprintf(2, "%s: cache '%s' is corrupt\n", _dl_progname, LDSO_CACHE);
			goto fail;
		}
	}

	return 0;

fail:
	_dl_munmap(_dl_cache_addr, _dl_cache_size);
	_dl_cache_addr = (caddr_t) - 1;
	return -1;
}

int _dl_unmap_cache(void)
{
	if (_dl_cache_addr == NULL || _dl_cache_addr == (caddr_t) - 1)
		return -1;

#if 1
	_dl_munmap(_dl_cache_addr, _dl_cache_size);
	_dl_cache_addr = NULL;
#endif

	return 0;
}
#endif


void 
_dl_protect_relro (struct elf_resolve *l)
{
	ElfW(Addr) start = ((l->loadaddr + l->relro_addr)
			    & PAGE_ALIGN);
	ElfW(Addr) end = ((l->loadaddr + l->relro_addr + l->relro_size)
			  & PAGE_ALIGN);
	_dl_if_debug_dprint("RELRO protecting %s:  start:%x, end:%x\n", l->libname, start, end);
	if (start != end &&
	    _dl_mprotect ((void *) start, end - start, PROT_READ) < 0) {
		_dl_dprintf(2, "%s: cannot apply additional memory protection after relocation", l->libname);
		_dl_exit(0);
	}
}

/* This function's behavior must exactly match that
 * in uClibc/ldso/util/ldd.c */
static struct elf_resolve *
search_for_named_library(const char *name, int secure, const char *path_list,
	struct dyn_elf **rpnt)
{
	char *path, *path_n, *mylibname;
	struct elf_resolve *tpnt;
	int done;

	if (path_list==NULL)
		return NULL;

	/* We need a writable copy of this string, but we don't
	 * need this allocated permanently since we don't want
	 * to leak memory, so use alloca to put path on the stack */
	done = _dl_strlen(path_list);
	path = alloca(done + 1);

	/* another bit of local storage */
	mylibname = alloca(2050);

	/* gcc inlines alloca using a single instruction adjusting
	 * the stack pointer and no stack overflow check and thus
	 * no NULL error return.  No point leaving in dead code... */
#if 0
	if (!path || !mylibname) {
		_dl_dprintf(2, "Out of memory!\n");
		_dl_exit(0);
	}
#endif

	_dl_memcpy(path, path_list, done+1);

	/* Unlike ldd.c, don't bother to eliminate double //s */

	/* Replace colons with zeros in path_list */
	/* : at the beginning or end of path maps to CWD */
	/* :: anywhere maps CWD */
	/* "" maps to CWD */ 
	done = 0;
	path_n = path;
	do {
		if (*path == 0) {
			*path = ':';
			done = 1;
		}
		if (*path == ':') {
			*path = 0;
			if (*path_n)
				_dl_strcpy(mylibname, path_n);
			else
				_dl_strcpy(mylibname, "."); /* Assume current dir if empty path */
			_dl_strcat(mylibname, "/");
			_dl_strcat(mylibname, name);
			if ((tpnt = _dl_load_elf_shared_library(secure, rpnt, mylibname)) != NULL)
				return tpnt;
			path_n = path+1;
		}
		path++;
	} while (!done);
	return NULL;
}

/* Used to return error codes back to dlopen et. al.  */
unsigned long _dl_error_number;
unsigned long _dl_internal_error_number;

struct elf_resolve *_dl_load_shared_library(int secure, struct dyn_elf **rpnt,
	struct elf_resolve *tpnt, char *full_libname, int __attribute__((unused)) trace_loaded_objects)
{
	char *pnt;
	struct elf_resolve *tpnt1;
	char *libname;

	_dl_internal_error_number = 0;
	libname = full_libname;

	/* quick hack to ensure mylibname buffer doesn't overflow.  don't
	   allow full_libname or any directory to be longer than 1024. */
	if (_dl_strlen(full_libname) > 1024)
		goto goof;

	/* Skip over any initial initial './' and '/' stuff to
	 * get the short form libname with no path garbage */
	pnt = _dl_strrchr(libname, '/');
	if (pnt) {
		libname = pnt + 1;
	}

	_dl_if_debug_dprint("\tfind library='%s'; searching\n", libname);
	/* If the filename has any '/', try it straight and leave it at that.
	   For IBCS2 compatibility under linux, we substitute the string
	   /usr/i486-sysv4/lib for /usr/lib in library names. */

	if (libname != full_libname) {
		_dl_if_debug_dprint("\ttrying file='%s'\n", full_libname);
		tpnt1 = _dl_load_elf_shared_library(secure, rpnt, full_libname);
		if (tpnt1) {
			return tpnt1;
		}
	}

	/*
	 * The ABI specifies that RPATH is searched before LD_LIBRARY_PATH or
	 * the default path of /usr/lib.  Check in rpath directories.
	 */
#ifdef __LDSO_RUNPATH__
	pnt = (tpnt ? (char *) tpnt->dynamic_info[DT_RPATH] : NULL);
	if (pnt) {
		pnt += (unsigned long) tpnt->dynamic_info[DT_STRTAB];
		_dl_if_debug_dprint("\tsearching RPATH='%s'\n", pnt);
		if ((tpnt1 = search_for_named_library(libname, secure, pnt, rpnt)) != NULL)
			return tpnt1;
	}
#endif

	/* Check in LD_{ELF_}LIBRARY_PATH, if specified and allowed */
	if (_dl_library_path) {
		_dl_if_debug_dprint("\tsearching LD_LIBRARY_PATH='%s'\n", _dl_library_path);
		if ((tpnt1 = search_for_named_library(libname, secure, _dl_library_path, rpnt)) != NULL)
		{
			return tpnt1;
		}
	}

	/*
	 * The ABI specifies that RUNPATH is searched after LD_LIBRARY_PATH.
	 */
#ifdef __LDSO_RUNPATH__
	pnt = (tpnt ? (char *)tpnt->dynamic_info[DT_RUNPATH] : NULL);
	if (pnt) {
		pnt += (unsigned long) tpnt->dynamic_info[DT_STRTAB];
		_dl_if_debug_dprint("\tsearching RUNPATH='%s'\n", pnt);
		if ((tpnt1 = search_for_named_library(libname, secure, pnt, rpnt)) != NULL)
			return tpnt1;
	}
#endif

	/*
	 * Where should the cache be searched?  There is no such concept in the
	 * ABI, so we have some flexibility here.  For now, search it before
	 * the hard coded paths that follow (i.e before /lib and /usr/lib).
	 */
#ifdef __LDSO_CACHE_SUPPORT__
	if (_dl_cache_addr != NULL && _dl_cache_addr != (caddr_t) - 1) {
		int i;
		header_t *header = (header_t *) _dl_cache_addr;
		libentry_t *libent = (libentry_t *) & header[1];
		char *strs = (char *) &libent[header->nlibs];

		_dl_if_debug_dprint("\tsearching cache='%s'\n", LDSO_CACHE);
		for (i = 0; i < header->nlibs; i++) {
			if ((libent[i].flags == LIB_ELF ||
						libent[i].flags == LIB_ELF_LIBC0 ||
						libent[i].flags == LIB_ELF_LIBC5) &&
					_dl_strcmp(libname, strs + libent[i].sooffset) == 0 &&
					(tpnt1 = _dl_load_elf_shared_library(secure,
														 rpnt, strs + libent[i].liboffset)))
				return tpnt1;
		}
	}
#endif

	/* Look for libraries wherever the shared library loader
	 * was installed */
	_dl_if_debug_dprint("\tsearching ldso dir='%s'\n", _dl_ldsopath);
	if ((tpnt1 = search_for_named_library(libname, secure, _dl_ldsopath, rpnt)) != NULL)
	{
		return tpnt1;
	}


	/* Lastly, search the standard list of paths for the library.
	   This list must exactly match the list in uClibc/ldso/util/ldd.c */
	_dl_if_debug_dprint("\tsearching full lib path list\n");
	if ((tpnt1 = search_for_named_library(libname, secure,
					UCLIBC_RUNTIME_PREFIX "lib:"
					UCLIBC_RUNTIME_PREFIX "usr/lib"
#ifndef __LDSO_CACHE_SUPPORT__
					":" UCLIBC_RUNTIME_PREFIX "usr/X11R6/lib"
#endif
					, rpnt)
		) != NULL)
	{
		return tpnt1;
	}

goof:
	/* Well, we shot our wad on that one.  All we can do now is punt */
	if (_dl_internal_error_number)
		_dl_error_number = _dl_internal_error_number;
	else
		_dl_error_number = LD_ERROR_NOFILE;
	_dl_if_debug_dprint("Bummer: could not find '%s'!\n", libname);
	return NULL;
}


/*
 * Read one ELF library into memory, mmap it into the correct locations and
 * add the symbol info to the symbol chain.  Perform any relocations that
 * are required.
 */

struct elf_resolve *_dl_load_elf_shared_library(int secure,
	struct dyn_elf **rpnt, char *libname)
{
	ElfW(Ehdr) *epnt;
	unsigned long dynamic_addr = 0;
	ElfW(Dyn) *dpnt;
	struct elf_resolve *tpnt;
	ElfW(Phdr) *ppnt;
	char *status, *header;
	unsigned long dynamic_info[DYNAMIC_SIZE];
	unsigned long *lpnt;
	unsigned long libaddr;
	unsigned long minvma = 0xffffffff, maxvma = 0;
	int i, flags, piclib, infile;
	ElfW(Addr) relro_addr = 0;
	size_t relro_size = 0;
		struct stat st;

	libaddr = 0;
	infile = _dl_open(libname, O_RDONLY, 0);
	if (infile < 0) {
		_dl_internal_error_number = LD_ERROR_NOFILE;
		return NULL;
	}

	if (_dl_fstat(infile, &st) < 0) {
		_dl_internal_error_number = LD_ERROR_NOFILE;
		_dl_close(infile);
		return NULL;
	}
	/* If we are in secure mode (i.e. a setu/gid binary using LD_PRELOAD),
	   we don't load the library if it isn't setuid. */
	if (secure)
		if (!(st.st_mode & S_ISUID)) {
			_dl_close(infile);
			return NULL;
		}

	/* Check if file is already loaded */
	for (tpnt = _dl_loaded_modules; tpnt; tpnt = tpnt->next) {
		if(tpnt->st_dev == st.st_dev && tpnt->st_ino == st.st_ino) {
			/* Already loaded */
			tpnt->usage_count++;
			_dl_close(infile);
			return tpnt;
		}
	}
	header = _dl_mmap((void *) 0, _dl_pagesize, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (_dl_mmap_check_error(header)) {
		_dl_dprintf(2, "%s:%i: can't map '%s'\n", _dl_progname, __LINE__, libname);
		_dl_internal_error_number = LD_ERROR_MMAP_FAILED;
		_dl_close(infile);
		return NULL;
	}

	_dl_read(infile, header, _dl_pagesize);
	epnt = (ElfW(Ehdr) *) (intptr_t) header;
	if (epnt->e_ident[0] != 0x7f ||
			epnt->e_ident[1] != 'E' ||
			epnt->e_ident[2] != 'L' ||
			epnt->e_ident[3] != 'F')
	{
		_dl_dprintf(2, "%s: '%s' is not an ELF file\n", _dl_progname,
				libname);
		_dl_internal_error_number = LD_ERROR_NOTELF;
		_dl_close(infile);
		_dl_munmap(header, _dl_pagesize);
		return NULL;
	}

	if ((epnt->e_type != ET_DYN) || (epnt->e_machine != MAGIC1
#ifdef MAGIC2
				&& epnt->e_machine != MAGIC2
#endif
				))
	{
		_dl_internal_error_number =
			(epnt->e_type != ET_DYN ? LD_ERROR_NOTDYN : LD_ERROR_NOTMAGIC);
		_dl_dprintf(2, "%s: '%s' is not an ELF executable for " ELF_TARGET
				"\n", _dl_progname, libname);
		_dl_close(infile);
		_dl_munmap(header, _dl_pagesize);
		return NULL;
	}

	ppnt = (ElfW(Phdr) *)(intptr_t) & header[epnt->e_phoff];

	piclib = 1;
	for (i = 0; i < epnt->e_phnum; i++) {

		if (ppnt->p_type == PT_DYNAMIC) {
			if (dynamic_addr)
				_dl_dprintf(2, "%s: '%s' has more than one dynamic section\n",
						_dl_progname, libname);
			dynamic_addr = ppnt->p_vaddr;
		}

		if (ppnt->p_type == PT_LOAD) {
			/* See if this is a PIC library. */
			if (i == 0 && ppnt->p_vaddr > 0x1000000) {
				piclib = 0;
				minvma = ppnt->p_vaddr;
			}
			if (piclib && ppnt->p_vaddr < minvma) {
				minvma = ppnt->p_vaddr;
			}
			if (((unsigned long) ppnt->p_vaddr + ppnt->p_memsz) > maxvma) {
				maxvma = ppnt->p_vaddr + ppnt->p_memsz;
			}
		}
		ppnt++;
	}

	maxvma = (maxvma + ADDR_ALIGN) & PAGE_ALIGN;
	minvma = minvma & ~0xffffU;

	flags = MAP_PRIVATE /*| MAP_DENYWRITE */ ;
	if (!piclib)
		flags |= MAP_FIXED;

	status = (char *) _dl_mmap((char *) (piclib ? 0 : minvma),
			maxvma - minvma, PROT_NONE, flags | MAP_ANONYMOUS, -1, 0);
	if (_dl_mmap_check_error(status)) {
		_dl_dprintf(2, "%s:%i: can't map '%s'\n", _dl_progname, __LINE__, libname);
		_dl_internal_error_number = LD_ERROR_MMAP_FAILED;
		_dl_close(infile);
		_dl_munmap(header, _dl_pagesize);
		return NULL;
	}
	libaddr = (unsigned long) status;
	flags |= MAP_FIXED;

	/* Get the memory to store the library */
	ppnt = (ElfW(Phdr) *)(intptr_t) & header[epnt->e_phoff];

	for (i = 0; i < epnt->e_phnum; i++) {
		if (ppnt->p_type == PT_GNU_RELRO) {
			relro_addr = ppnt->p_vaddr;
			relro_size = ppnt->p_memsz;
		}
		if (ppnt->p_type == PT_LOAD) {

			/* See if this is a PIC library. */
			if (i == 0 && ppnt->p_vaddr > 0x1000000) {
				piclib = 0;
				/* flags |= MAP_FIXED; */
			}



			if (ppnt->p_flags & PF_W) {
				unsigned long map_size;
				char *cpnt;

				status = (char *) _dl_mmap((char *) ((piclib ? libaddr : 0) +
							(ppnt->p_vaddr & PAGE_ALIGN)), (ppnt->p_vaddr & ADDR_ALIGN)
						+ ppnt->p_filesz, LXFLAGS(ppnt->p_flags), flags, infile,
						ppnt->p_offset & OFFS_ALIGN);

				if (_dl_mmap_check_error(status)) {
					_dl_dprintf(2, "%s:%i: can't map '%s'\n",
							_dl_progname, __LINE__, libname);
					_dl_internal_error_number = LD_ERROR_MMAP_FAILED;
					_dl_munmap((char *) libaddr, maxvma - minvma);
					_dl_close(infile);
					_dl_munmap(header, _dl_pagesize);
					return NULL;
				}

				/* Pad the last page with zeroes. */
				cpnt = (char *) (status + (ppnt->p_vaddr & ADDR_ALIGN) +
						ppnt->p_filesz);
				while (((unsigned long) cpnt) & ADDR_ALIGN)
					*cpnt++ = 0;

				/* I am not quite sure if this is completely
				 * correct to do or not, but the basic way that
				 * we handle bss segments is that we mmap
				 * /dev/zero if there are any pages left over
				 * that are not mapped as part of the file */

				map_size = (ppnt->p_vaddr + ppnt->p_filesz + ADDR_ALIGN) & PAGE_ALIGN;

				if (map_size < ppnt->p_vaddr + ppnt->p_memsz)
					status = (char *) _dl_mmap((char *) map_size +
							(piclib ? libaddr : 0),
							ppnt->p_vaddr + ppnt->p_memsz - map_size,
							LXFLAGS(ppnt->p_flags), flags | MAP_ANONYMOUS, -1, 0);
			} else
				status = (char *) _dl_mmap((char *) (ppnt->p_vaddr & PAGE_ALIGN)
						+ (piclib ? libaddr : 0), (ppnt->p_vaddr & ADDR_ALIGN) +
						ppnt->p_filesz, LXFLAGS(ppnt->p_flags), flags,
						infile, ppnt->p_offset & OFFS_ALIGN);
			if (_dl_mmap_check_error(status)) {
				_dl_dprintf(2, "%s:%i: can't map '%s'\n", _dl_progname, __LINE__, libname);
				_dl_internal_error_number = LD_ERROR_MMAP_FAILED;
				_dl_munmap((char *) libaddr, maxvma - minvma);
				_dl_close(infile);
				_dl_munmap(header, _dl_pagesize);
				return NULL;
			}

			/* if(libaddr == 0 && piclib) {
			   libaddr = (unsigned long) status;
			   flags |= MAP_FIXED;
			   } */
		}
		ppnt++;
	}
	_dl_close(infile);

	/* For a non-PIC library, the addresses are all absolute */
	if (piclib) {
		dynamic_addr += (unsigned long) libaddr;
	}

	/*
	 * OK, the ELF library is now loaded into VM in the correct locations
	 * The next step is to go through and do the dynamic linking (if needed).
	 */

	/* Start by scanning the dynamic section to get all of the pointers */

	if (!dynamic_addr) {
		_dl_internal_error_number = LD_ERROR_NODYNAMIC;
		_dl_dprintf(2, "%s: '%s' is missing a dynamic section\n",
				_dl_progname, libname);
		_dl_munmap(header, _dl_pagesize);
		return NULL;
	}

	dpnt = (ElfW(Dyn) *) dynamic_addr;
	_dl_memset(dynamic_info, 0, sizeof(dynamic_info));
	_dl_parse_dynamic_info(dpnt, dynamic_info, NULL, libaddr);
	/* If the TEXTREL is set, this means that we need to make the pages
	   writable before we perform relocations.  Do this now. They get set
	   back again later. */

	if (dynamic_info[DT_TEXTREL]) {
#ifndef __FORCE_SHAREABLE_TEXT_SEGMENTS__
		ppnt = (ElfW(Phdr) *)(intptr_t) & header[epnt->e_phoff];
		for (i = 0; i < epnt->e_phnum; i++, ppnt++) {
			if (ppnt->p_type == PT_LOAD && !(ppnt->p_flags & PF_W))
				_dl_mprotect((void *) ((piclib ? libaddr : 0) +
							(ppnt->p_vaddr & PAGE_ALIGN)),
						(ppnt->p_vaddr & ADDR_ALIGN) + (unsigned long) ppnt->p_filesz,
						PROT_READ | PROT_WRITE | PROT_EXEC);
		}
#else
		_dl_dprintf(_dl_debug_file, "Can't modify %s's text section. Use GCC option -fPIC for shared objects, please.\n",libname);
		_dl_exit(1);
#endif
	}

	tpnt = _dl_add_elf_hash_table(libname, (char *) libaddr, dynamic_info,
			dynamic_addr, 0);
	tpnt->relro_addr = relro_addr;
	tpnt->relro_size = relro_size;
	tpnt->st_dev = st.st_dev;
	tpnt->st_ino = st.st_ino;
	tpnt->ppnt = (ElfW(Phdr) *)(intptr_t) (tpnt->loadaddr + epnt->e_phoff);
	tpnt->n_phent = epnt->e_phnum;

	/*
	 * Add this object into the symbol chain
	 */
	if (*rpnt) {
		(*rpnt)->next = (struct dyn_elf *) _dl_malloc(sizeof(struct dyn_elf));
		_dl_memset((*rpnt)->next, 0, sizeof(struct dyn_elf));
		(*rpnt)->next->prev = (*rpnt);
		*rpnt = (*rpnt)->next;
		(*rpnt)->dyn = tpnt;
		tpnt->symbol_scope = _dl_symbol_tables;
	}
	tpnt->usage_count++;
	tpnt->libtype = elf_lib;

	/*
	 * OK, the next thing we need to do is to insert the dynamic linker into
	 * the proper entry in the GOT so that the PLT symbols can be properly
	 * resolved.
	 */

	lpnt = (unsigned long *) dynamic_info[DT_PLTGOT];

	if (lpnt) {
		lpnt = (unsigned long *) (dynamic_info[DT_PLTGOT]);
		INIT_GOT(lpnt, tpnt);
	}

	_dl_if_debug_dprint("\n\tfile='%s';  generating link map\n", libname);
	_dl_if_debug_dprint("\t\tdynamic: %x  base: %x\n", dynamic_addr, libaddr);
	_dl_if_debug_dprint("\t\t  entry: %x  phdr: %x  phnum: %x\n\n",
			epnt->e_entry + libaddr, tpnt->ppnt, tpnt->n_phent);

	_dl_munmap(header, _dl_pagesize);

	return tpnt;
}

/* now_flag must be RTLD_NOW or zero */
int _dl_fixup(struct dyn_elf *rpnt, int now_flag)
{
	int goof = 0;
	struct elf_resolve *tpnt;
	ElfW(Word) reloc_size, relative_count;
	ElfW(Addr) reloc_addr;

	if (rpnt->next)
		goof = _dl_fixup(rpnt->next, now_flag);
	if (goof)
		return goof;
	tpnt = rpnt->dyn;

	if(!(tpnt->init_flag & RELOCS_DONE)) 
		_dl_if_debug_dprint("relocation processing: %s\n", tpnt->libname);

	if (unlikely(tpnt->dynamic_info[UNSUPPORTED_RELOC_TYPE])) {
		_dl_if_debug_dprint("%s: can't handle %s relocation records\n",
				_dl_progname, UNSUPPORTED_RELOC_STR);
		goof++;
		return goof;
	}

	reloc_size = tpnt->dynamic_info[DT_RELOC_TABLE_SIZE];
/* On some machines, notably SPARC & PPC, DT_REL* includes DT_JMPREL in its
   range.  Note that according to the ELF spec, this is completely legal! */
#ifdef ELF_MACHINE_PLTREL_OVERLAP
	reloc_size -= tpnt->dynamic_info [DT_PLTRELSZ];
#endif
	if (tpnt->dynamic_info[DT_RELOC_TABLE_ADDR] &&
	    !(tpnt->init_flag & RELOCS_DONE)) {
		reloc_addr = tpnt->dynamic_info[DT_RELOC_TABLE_ADDR];
		relative_count = tpnt->dynamic_info[DT_RELCONT_IDX];
		if (relative_count) { /* Optimize the XX_RELATIVE relocations if possible */
			reloc_size -= relative_count * sizeof(ELF_RELOC);
			elf_machine_relative(tpnt->loadaddr, reloc_addr, relative_count);
			reloc_addr += relative_count * sizeof(ELF_RELOC);
		}
		goof += _dl_parse_relocation_information(rpnt,
				reloc_addr,
				reloc_size);
		tpnt->init_flag |= RELOCS_DONE;
	}
	if (tpnt->dynamic_info[DT_BIND_NOW])
		now_flag = RTLD_NOW;
	if (tpnt->dynamic_info[DT_JMPREL] &&
	    (!(tpnt->init_flag & JMP_RELOCS_DONE) ||
	     (now_flag && !(tpnt->rtld_flags & now_flag)))) {
		tpnt->rtld_flags |= now_flag; 
		if (!(tpnt->rtld_flags & RTLD_NOW)) {
			_dl_parse_lazy_relocation_information(rpnt,
					tpnt->dynamic_info[DT_JMPREL],
					tpnt->dynamic_info [DT_PLTRELSZ]);
		} else {
			goof += _dl_parse_relocation_information(rpnt,
					tpnt->dynamic_info[DT_JMPREL],
					tpnt->dynamic_info[DT_PLTRELSZ]);
		}
		tpnt->init_flag |= JMP_RELOCS_DONE;
	}
	return goof;
}

/* Minimal printf which handles only %s, %d, and %x */
void _dl_dprintf(int fd, const char *fmt, ...)
{
#if __WORDSIZE > 32
	long int num;
#else
	int num;
#endif
	va_list args;
	char *start, *ptr, *string;
	static char *buf;

	if (!fmt)
		return;

	buf = _dl_mmap((void *) 0, _dl_pagesize, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (_dl_mmap_check_error(buf)) {
		_dl_write(fd, "mmap of a spare page failed!\n", 29);
		_dl_exit(20);
	}

	start = ptr = buf;

	if (_dl_strlen(fmt) >= (_dl_pagesize - 1)) {
		_dl_write(fd, "overflow\n", 11);
		_dl_exit(20);
	}

	_dl_strcpy(buf, fmt);
	va_start(args, fmt);

	while (start) {
		while (*ptr != '%' && *ptr) {
			ptr++;
		}

		if (*ptr == '%') {
			*ptr++ = '\0';
			_dl_write(fd, start, _dl_strlen(start));

			switch (*ptr++) {
				case 's':
					string = va_arg(args, char *);

					if (!string)
						_dl_write(fd, "(null)", 6);
					else
						_dl_write(fd, string, _dl_strlen(string));
					break;

				case 'i':
				case 'd':
					{
						char tmp[22];
#if __WORDSIZE > 32
						num = va_arg(args, long int);
#else
						num = va_arg(args, int);
#endif
						string = _dl_simple_ltoa(tmp, num);
						_dl_write(fd, string, _dl_strlen(string));
						break;
					}
				case 'x':
				case 'X':
					{
						char tmp[22];
#if __WORDSIZE > 32
						num = va_arg(args, long int);
#else
						num = va_arg(args, int);
#endif
						string = _dl_simple_ltoahex(tmp, num);
						_dl_write(fd, string, _dl_strlen(string));
						break;
					}
				default:
					_dl_write(fd, "(null)", 6);
					break;
			}

			start = ptr;
		} else {
			_dl_write(fd, start, _dl_strlen(start));
			start = NULL;
		}
	}
	_dl_munmap(buf, _dl_pagesize);
	return;
}

char *_dl_strdup(const char *string)
{
	char *retval;
	int len;

	len = _dl_strlen(string);
	retval = _dl_malloc(len + 1);
	_dl_strcpy(retval, string);
	return retval;
}

void _dl_parse_dynamic_info(ElfW(Dyn) *dpnt, unsigned long dynamic_info[], void *debug_addr, ElfW(Addr) load_off)
{
	__dl_parse_dynamic_info(dpnt, dynamic_info, debug_addr, load_off);
}

/* we want this in ldso.so and libdl.a but nowhere else */
#ifdef __USE_GNU
#if ! defined SHARED || (! defined PIC && ! defined __PIC__)
int
__dl_iterate_phdr (int (*callback) (struct dl_phdr_info *info, size_t size, void *data), void *data)
{
	struct elf_resolve *l;
	struct dl_phdr_info info;
	int ret = 0;

	for (l = _dl_loaded_modules; l != NULL; l = l->next) {
		info.dlpi_addr = l->loadaddr;
		info.dlpi_name = l->libname;
		info.dlpi_phdr = l->ppnt;
		info.dlpi_phnum = l->n_phent;
		ret = callback (&info, sizeof (struct dl_phdr_info), data);
		if (ret)
			break;
	}
	return ret;
}
strong_alias(__dl_iterate_phdr, dl_iterate_phdr)
#endif
#endif
