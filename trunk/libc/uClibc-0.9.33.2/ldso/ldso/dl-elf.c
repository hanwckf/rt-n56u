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

	if (_dl_cache_addr == MAP_FAILED)
		return -1;
	else if (_dl_cache_addr != NULL)
		return 0;

	if (_dl_stat(LDSO_CACHE, &st)
	    || (fd = _dl_open(LDSO_CACHE, O_RDONLY|O_CLOEXEC, 0)) < 0) {
		_dl_cache_addr = MAP_FAILED;	/* so we won't try again */
		return -1;
	}

	_dl_cache_size = st.st_size;
	_dl_cache_addr = _dl_mmap(0, _dl_cache_size, PROT_READ, LDSO_CACHE_MMAP_FLAGS, fd, 0);
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
	_dl_cache_addr = MAP_FAILED;
	return -1;
}

int _dl_unmap_cache(void)
{
	if (_dl_cache_addr == NULL || _dl_cache_addr == MAP_FAILED)
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
	ElfW(Addr) base = (ElfW(Addr)) DL_RELOC_ADDR(l->loadaddr, l->relro_addr);
	ElfW(Addr) start = (base & PAGE_ALIGN);
	ElfW(Addr) end = ((base + l->relro_size) & PAGE_ALIGN);
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
search_for_named_library(const char *name, unsigned rflags, const char *path_list,
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
			if ((tpnt = _dl_load_elf_shared_library(rflags, rpnt, mylibname)) != NULL)
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

struct elf_resolve *_dl_load_shared_library(unsigned rflags, struct dyn_elf **rpnt,
	struct elf_resolve *tpnt, char *full_libname, int attribute_unused trace_loaded_objects)
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
		tpnt1 = _dl_load_elf_shared_library(rflags, rpnt, full_libname);
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
		if ((tpnt1 = search_for_named_library(libname, rflags, pnt, rpnt)) != NULL)
			return tpnt1;
	}
#endif

#ifdef __LDSO_LD_LIBRARY_PATH__
	/* Check in LD_{ELF_}LIBRARY_PATH, if specified and allowed */
	if (_dl_library_path) {
		_dl_if_debug_dprint("\tsearching LD_LIBRARY_PATH='%s'\n", _dl_library_path);
		if ((tpnt1 = search_for_named_library(libname, rflags, _dl_library_path, rpnt)) != NULL)
		{
			return tpnt1;
		}
	}
#endif
	/*
	 * The ABI specifies that RUNPATH is searched after LD_LIBRARY_PATH.
	 */
#ifdef __LDSO_RUNPATH__
	pnt = (tpnt ? (char *)tpnt->dynamic_info[DT_RUNPATH] : NULL);
	if (pnt) {
		pnt += (unsigned long) tpnt->dynamic_info[DT_STRTAB];
		_dl_if_debug_dprint("\tsearching RUNPATH='%s'\n", pnt);
		if ((tpnt1 = search_for_named_library(libname, rflags, pnt, rpnt)) != NULL)
			return tpnt1;
	}
#endif

	/*
	 * Where should the cache be searched?  There is no such concept in the
	 * ABI, so we have some flexibility here.  For now, search it before
	 * the hard coded paths that follow (i.e before /lib and /usr/lib).
	 */
#ifdef __LDSO_CACHE_SUPPORT__
	if (_dl_cache_addr != NULL && _dl_cache_addr != MAP_FAILED) {
		int i;
		header_t *header = (header_t *) _dl_cache_addr;
		libentry_t *libent = (libentry_t *) & header[1];
		char *strs = (char *) &libent[header->nlibs];

		_dl_if_debug_dprint("\tsearching cache='%s'\n", LDSO_CACHE);
		for (i = 0; i < header->nlibs; i++) {
			if ((libent[i].flags == LIB_ELF
			     || libent[i].flags == LIB_ELF_LIBC0
			     ||	libent[i].flags == LIB_ELF_LIBC5)
			 && _dl_strcmp(libname, strs + libent[i].sooffset) == 0
			 && (tpnt1 = _dl_load_elf_shared_library(rflags, rpnt, strs + libent[i].liboffset))
			) {
				return tpnt1;
			}
		}
	}
#endif
#if defined SHARED && defined __LDSO_SEARCH_INTERP_PATH__
	/* Look for libraries wherever the shared library loader
	 * was installed */
	_dl_if_debug_dprint("\tsearching ldso dir='%s'\n", _dl_ldsopath);
	tpnt1 = search_for_named_library(libname, rflags, _dl_ldsopath, rpnt);
	if (tpnt1 != NULL)
		return tpnt1;
#endif
	/* Lastly, search the standard list of paths for the library.
	   This list must exactly match the list in uClibc/ldso/util/ldd.c */
	_dl_if_debug_dprint("\tsearching full lib path list\n");
	tpnt1 = search_for_named_library(libname, rflags,
					UCLIBC_RUNTIME_PREFIX "lib:"
					UCLIBC_RUNTIME_PREFIX "usr/lib"
#ifndef __LDSO_CACHE_SUPPORT__
					":" UCLIBC_RUNTIME_PREFIX "usr/X11R6/lib"
#endif
					, rpnt);
	if (tpnt1 != NULL)
		return tpnt1;

goof:
	/* Well, we shot our wad on that one.  All we can do now is punt */
	if (_dl_internal_error_number)
		_dl_error_number = _dl_internal_error_number;
	else
		_dl_error_number = LD_ERROR_NOFILE;
	_dl_if_debug_dprint("Bummer: could not find '%s'!\n", libname);
	return NULL;
}

/* Define the _dl_library_offset for the architectures that need it */
DL_DEF_LIB_OFFSET;

/*
 * Make a writeable mapping of a segment, regardless of whether PF_W is
 * set or not.
 */
static void *
map_writeable (int infile, ElfW(Phdr) *ppnt, int piclib, int flags,
	       unsigned long libaddr)
{
	int prot_flags = ppnt->p_flags | PF_W;
	char *status, *retval;
	char *tryaddr;
	ssize_t size;
	unsigned long map_size;
	char *cpnt;
	char *piclib2map = NULL;

	if (piclib == 2 &&
	    /* We might be able to avoid this call if memsz doesn't
	       require an additional page, but this would require mmap
	       to always return page-aligned addresses and a whole
	       number of pages allocated.  Unfortunately on uClinux
	       may return misaligned addresses and may allocate
	       partial pages, so we may end up doing unnecessary mmap
	       calls.

	       This is what we could do if we knew mmap would always
	       return aligned pages:

	       ((ppnt->p_vaddr + ppnt->p_filesz + ADDR_ALIGN) &
	       PAGE_ALIGN) < ppnt->p_vaddr + ppnt->p_memsz)

	       Instead, we have to do this:  */
	    ppnt->p_filesz < ppnt->p_memsz)
	{
		piclib2map = (char *)
			_dl_mmap(0, (ppnt->p_vaddr & ADDR_ALIGN) + ppnt->p_memsz,
				 LXFLAGS(prot_flags), flags | MAP_ANONYMOUS, -1, 0);
		if (_dl_mmap_check_error(piclib2map))
			return 0;
	}

	tryaddr = piclib == 2 ? piclib2map
		: ((char *) (piclib ? libaddr : DL_GET_LIB_OFFSET()) +
		   (ppnt->p_vaddr & PAGE_ALIGN));

	size = (ppnt->p_vaddr & ADDR_ALIGN) + ppnt->p_filesz;

	/* For !MMU, mmap to fixed address will fail.
	   So instead of desperately call mmap and fail,
	   we set status to MAP_FAILED to save a call
	   to mmap ().  */
#ifndef __ARCH_USE_MMU__
	if (piclib2map == 0)
#endif
		status = (char *) _dl_mmap
			(tryaddr, size, LXFLAGS(prot_flags),
			 flags | (piclib2map ? MAP_FIXED : 0),
			 infile, ppnt->p_offset & OFFS_ALIGN);
#ifndef __ARCH_USE_MMU__
	else
		status = MAP_FAILED;
#endif
#ifdef _DL_PREAD
	if (_dl_mmap_check_error(status) && piclib2map
	    && (_DL_PREAD (infile, tryaddr, size,
			   ppnt->p_offset & OFFS_ALIGN) == size))
		status = tryaddr;
#endif
	if (_dl_mmap_check_error(status) || (tryaddr && tryaddr != status))
		return 0;

	if (piclib2map)
		retval = piclib2map;
	else
		retval = status;

	/* Now we want to allocate and zero-out any data from the end
	   of the region we mapped in from the file (filesz) to the
	   end of the loadable segment (memsz).  We may need
	   additional pages for memsz, that we map in below, and we
	   can count on the kernel to zero them out, but we have to
	   zero out stuff in the last page that we mapped in from the
	   file.  However, we can't assume to have actually obtained
	   full pages from the kernel, since we didn't ask for them,
	   and uClibc may not give us full pages for small
	   allocations.  So only zero out up to memsz or the end of
	   the page, whichever comes first.  */

	/* CPNT is the beginning of the memsz portion not backed by
	   filesz.  */
	cpnt = (char *) (status + size);

	/* MAP_SIZE is the address of the
	   beginning of the next page.  */
	map_size = (ppnt->p_vaddr + ppnt->p_filesz
		    + ADDR_ALIGN) & PAGE_ALIGN;

	_dl_memset (cpnt, 0,
		    MIN (map_size
			 - (ppnt->p_vaddr
			    + ppnt->p_filesz),
			 ppnt->p_memsz
			 - ppnt->p_filesz));

	if (map_size < ppnt->p_vaddr + ppnt->p_memsz && !piclib2map) {
		tryaddr = map_size + (char*)(piclib ? libaddr : 0);
		status = (char *) _dl_mmap(tryaddr,
					   ppnt->p_vaddr + ppnt->p_memsz - map_size,
					   LXFLAGS(prot_flags),
					   flags | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
		if (_dl_mmap_check_error(status) || tryaddr != status)
			return NULL;
	}
	return retval;
}

/*
 * Read one ELF library into memory, mmap it into the correct locations and
 * add the symbol info to the symbol chain.  Perform any relocations that
 * are required.
 */

struct elf_resolve *_dl_load_elf_shared_library(unsigned rflags,
	struct dyn_elf **rpnt, const char *libname)
{
	ElfW(Ehdr) *epnt;
	unsigned long dynamic_addr = 0;
	ElfW(Dyn) *dpnt;
	struct elf_resolve *tpnt;
	ElfW(Phdr) *ppnt;
#if defined(USE_TLS) && USE_TLS
	ElfW(Phdr) *tlsppnt = NULL;
#endif
	char *status, *header;
	unsigned long dynamic_info[DYNAMIC_SIZE];
	unsigned long *lpnt;
	unsigned long libaddr;
	unsigned long minvma = 0xffffffff, maxvma = 0;
	unsigned int rtld_flags;
	int i, flags, piclib, infile;
	ElfW(Addr) relro_addr = 0;
	size_t relro_size = 0;
	struct stat st;
	uint32_t *p32;
	DL_LOADADDR_TYPE lib_loadaddr;
	DL_INIT_LOADADDR_EXTRA_DECLS

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
	/* If we are in secure mode (i.e. a setuid/gid binary using LD_PRELOAD),
	   we don't load the library if it isn't setuid. */
	if (rflags & DL_RESOLVE_SECURE) {
		if (!(st.st_mode & S_ISUID)) {
			_dl_close(infile);
			return NULL;
		}
	}

	/* Check if file is already loaded */
	for (tpnt = _dl_loaded_modules; tpnt; tpnt = tpnt->next) {
		if (tpnt->st_dev == st.st_dev && tpnt->st_ino == st.st_ino) {
			/* Already loaded */
			tpnt->usage_count++;
			_dl_close(infile);
			return tpnt;
		}
	}
	if (rflags & DL_RESOLVE_NOLOAD) {
		_dl_close(infile);
		return NULL;
	}
	header = _dl_mmap((void *) 0, _dl_pagesize, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZE, -1, 0);
	if (_dl_mmap_check_error(header)) {
		_dl_dprintf(2, "%s:%i: can't map '%s'\n", _dl_progname, __LINE__, libname);
		_dl_internal_error_number = LD_ERROR_MMAP_FAILED;
		_dl_close(infile);
		return NULL;
	}

	_dl_read(infile, header, _dl_pagesize);
	epnt = (ElfW(Ehdr) *) (intptr_t) header;
	p32 = (uint32_t*)&epnt->e_ident;
	if (*p32 != ELFMAG_U32) {
		_dl_dprintf(2, "%s: '%s' is not an ELF file\n", _dl_progname,
				libname);
		_dl_internal_error_number = LD_ERROR_NOTELF;
		_dl_close(infile);
		_dl_munmap(header, _dl_pagesize);
		return NULL;
	}

	if ((epnt->e_type != ET_DYN
#ifdef __LDSO_STANDALONE_SUPPORT__
		&& epnt->e_type != ET_EXEC
#endif
		) || (epnt->e_machine != MAGIC1
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
			if (minvma == 0xffffffff && ppnt->p_vaddr > 0x1000000) {
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
		if (ppnt->p_type == PT_TLS) {
#if defined(USE_TLS) && USE_TLS
			if (ppnt->p_memsz == 0)
				/* Nothing to do for an empty segment.  */
				continue;
			else
				/* Save for after 'tpnt' is actually allocated. */
				tlsppnt = ppnt;
#else
			/*
			 * Yup, the user was an idiot and tried to sneak in a library with
			 * TLS in it and we don't support it. Let's fall on our own sword
			 * and scream at the luser while we die.
			 */
			_dl_dprintf(2, "%s: '%s' library contains unsupported TLS\n",
				_dl_progname, libname);
			_dl_internal_error_number = LD_ERROR_TLS_FAILED;
			_dl_close(infile);
			_dl_munmap(header, _dl_pagesize);
			return NULL;
#endif
		}
		ppnt++;
	}

#ifdef __LDSO_STANDALONE_SUPPORT__
	if (epnt->e_type == ET_EXEC)
		piclib = 0;
#endif

	DL_CHECK_LIB_TYPE (epnt, piclib, _dl_progname, libname);

	maxvma = (maxvma + ADDR_ALIGN) & PAGE_ALIGN;
	minvma = minvma & ~ADDR_ALIGN;

	flags = MAP_PRIVATE /*| MAP_DENYWRITE */ ;

	if (piclib == 0 || piclib == 1) {
		status = (char *) _dl_mmap((char *) (piclib ? 0 : minvma),
				maxvma - minvma, PROT_NONE, flags | MAP_ANONYMOUS, -1, 0);
		if (_dl_mmap_check_error(status)) {
		cant_map:
			_dl_dprintf(2, "%s:%i: can't map '%s'\n", _dl_progname, __LINE__, libname);
			_dl_internal_error_number = LD_ERROR_MMAP_FAILED;
			_dl_close(infile);
			_dl_munmap(header, _dl_pagesize);
			return NULL;
		}
		libaddr = (unsigned long) status;
		flags |= MAP_FIXED;
	}

	/* Get the memory to store the library */
	ppnt = (ElfW(Phdr) *)(intptr_t) & header[epnt->e_phoff];

	DL_INIT_LOADADDR(lib_loadaddr, libaddr - minvma, ppnt, epnt->e_phnum);
	/* Set _dl_library_offset to lib_loadaddr or 0. */
	DL_SET_LIB_OFFSET(lib_loadaddr);

	for (i = 0; i < epnt->e_phnum; i++) {
		if (DL_IS_SPECIAL_SEGMENT (epnt, ppnt)) {
			char *addr;

			addr = DL_MAP_SEGMENT (epnt, ppnt, infile, flags);
			if (addr == NULL) {
			cant_map1:
				DL_LOADADDR_UNMAP (lib_loadaddr, maxvma - minvma);
				goto cant_map;
			}

			DL_INIT_LOADADDR_HDR (lib_loadaddr, addr, ppnt);
			ppnt++;
			continue;
		}
		if (ppnt->p_type == PT_GNU_RELRO) {
			relro_addr = ppnt->p_vaddr;
			relro_size = ppnt->p_memsz;
		}
		if (ppnt->p_type == PT_LOAD) {
			char *tryaddr;
			ssize_t size;

			if (ppnt->p_flags & PF_W) {
				status = map_writeable (infile, ppnt, piclib, flags, libaddr);
				if (status == NULL)
					goto cant_map1;
			} else {
				tryaddr = (piclib == 2 ? 0
					   : (char *) (ppnt->p_vaddr & PAGE_ALIGN)
					   + (piclib ? libaddr : DL_GET_LIB_OFFSET()));
				size = (ppnt->p_vaddr & ADDR_ALIGN) + ppnt->p_filesz;
				status = (char *) _dl_mmap
					   (tryaddr, size, LXFLAGS(ppnt->p_flags),
					    flags | (piclib == 2 ? MAP_EXECUTABLE
						     | MAP_DENYWRITE : 0),
					    infile, ppnt->p_offset & OFFS_ALIGN);
				if (_dl_mmap_check_error(status)
				    || (tryaddr && tryaddr != status))
				  goto cant_map1;
			}
			DL_INIT_LOADADDR_HDR(lib_loadaddr,
					     status + (ppnt->p_vaddr & ADDR_ALIGN),
					     ppnt);

			/* if (libaddr == 0 && piclib) {
			   libaddr = (unsigned long) status;
			   flags |= MAP_FIXED;
			   } */
		}
		ppnt++;
	}

	/*
	 * The dynamic_addr must be take into acount lib_loadaddr value, to note
	 * it is zero when the SO has been mapped to the elf's physical addr
	 */
#ifdef __LDSO_PRELINK_SUPPORT__
	if (DL_GET_LIB_OFFSET()) {
#else
	if (piclib) {
#endif
		dynamic_addr = (unsigned long) DL_RELOC_ADDR(lib_loadaddr, dynamic_addr);
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
		_dl_close(infile);
		return NULL;
	}

	dpnt = (ElfW(Dyn) *) dynamic_addr;
	_dl_memset(dynamic_info, 0, sizeof(dynamic_info));
	rtld_flags = _dl_parse_dynamic_info(dpnt, dynamic_info, NULL, lib_loadaddr);
	/* If the TEXTREL is set, this means that we need to make the pages
	   writable before we perform relocations.  Do this now. They get set
	   back again later. */

	if (dynamic_info[DT_TEXTREL]) {
#ifndef __FORCE_SHAREABLE_TEXT_SEGMENTS__
		ppnt = (ElfW(Phdr) *)(intptr_t) & header[epnt->e_phoff];
		for (i = 0; i < epnt->e_phnum; i++, ppnt++) {
			if (ppnt->p_type == PT_LOAD && !(ppnt->p_flags & PF_W)) {
#ifdef __ARCH_USE_MMU__
				_dl_mprotect((void *) ((piclib ? libaddr : DL_GET_LIB_OFFSET()) +
							(ppnt->p_vaddr & PAGE_ALIGN)),
						(ppnt->p_vaddr & ADDR_ALIGN) + (unsigned long) ppnt->p_filesz,
						PROT_READ | PROT_WRITE | PROT_EXEC);
#else
				void *new_addr;
				new_addr = map_writeable (infile, ppnt, piclib, flags, libaddr);
				if (!new_addr) {
					_dl_dprintf(_dl_debug_file, "Can't modify %s's text section.",
						    libname);
					_dl_exit(1);
				}
				DL_UPDATE_LOADADDR_HDR(lib_loadaddr,
						       new_addr + (ppnt->p_vaddr & ADDR_ALIGN),
						       ppnt);
				/* This has invalidated all pointers into the previously readonly segment.
				   Update any them to point into the remapped segment.  */
				_dl_parse_dynamic_info(dpnt, dynamic_info, NULL, lib_loadaddr);
#endif
			}
		}
#else
		_dl_dprintf(_dl_debug_file, "Can't modify %s's text section."
			" Use GCC option -fPIC for shared objects, please.\n",
			libname);
		_dl_exit(1);
#endif
	}

	_dl_close(infile);

	tpnt = _dl_add_elf_hash_table(libname, lib_loadaddr, dynamic_info,
			dynamic_addr, 0);
	tpnt->mapaddr = libaddr;
	tpnt->relro_addr = relro_addr;
	tpnt->relro_size = relro_size;
	tpnt->st_dev = st.st_dev;
	tpnt->st_ino = st.st_ino;
	tpnt->ppnt = (ElfW(Phdr) *)
		DL_RELOC_ADDR(DL_GET_RUN_ADDR(tpnt->loadaddr, tpnt->mapaddr),
		epnt->e_phoff);
	tpnt->n_phent = epnt->e_phnum;
	tpnt->rtld_flags |= rtld_flags;
#ifdef __LDSO_STANDALONE_SUPPORT__
	tpnt->l_entry = epnt->e_entry;
#endif

#if defined(USE_TLS) && USE_TLS
	if (tlsppnt) {
		_dl_debug_early("Found TLS header for %s\n", libname);
# if NO_TLS_OFFSET != 0
		tpnt->l_tls_offset = NO_TLS_OFFSET;
# endif
		tpnt->l_tls_blocksize = tlsppnt->p_memsz;
		tpnt->l_tls_align = tlsppnt->p_align;
		if (tlsppnt->p_align == 0)
			tpnt->l_tls_firstbyte_offset = 0;
		else
			tpnt->l_tls_firstbyte_offset = tlsppnt->p_vaddr &
				(tlsppnt->p_align - 1);
		tpnt->l_tls_initimage_size = tlsppnt->p_filesz;
		tpnt->l_tls_initimage = (void *) tlsppnt->p_vaddr;

		/* Assign the next available module ID.  */
		tpnt->l_tls_modid = _dl_next_tls_modid ();

		/* We know the load address, so add it to the offset. */
#ifdef __LDSO_STANDALONE_SUPPORT__
		if ((tpnt->l_tls_initimage != NULL) && piclib)
#else
		if (tpnt->l_tls_initimage != NULL)
#endif
		{
# ifdef __SUPPORT_LD_DEBUG_EARLY__
			unsigned int tmp = (unsigned int) tpnt->l_tls_initimage;
			tpnt->l_tls_initimage = (char *) tlsppnt->p_vaddr + tpnt->loadaddr;
			_dl_debug_early("Relocated TLS initial image from %x to %x (size = %x)\n", tmp, tpnt->l_tls_initimage, tpnt->l_tls_initimage_size);
			tmp = 0;
# else
			tpnt->l_tls_initimage = (char *) tlsppnt->p_vaddr + tpnt->loadaddr;
# endif
		}
	}
#endif

	/*
	 * Add this object into the symbol chain
	 */
	if (*rpnt
#ifdef __LDSO_STANDALONE_SUPPORT__
		/* Do not create a new chain entry for the main executable */
		&& (*rpnt)->dyn
#endif
		) {
		(*rpnt)->next = _dl_malloc(sizeof(struct dyn_elf));
		_dl_memset((*rpnt)->next, 0, sizeof(struct dyn_elf));
		(*rpnt)->next->prev = (*rpnt);
		*rpnt = (*rpnt)->next;
	}
#ifndef SHARED
	/* When statically linked, the first time we dlopen a DSO
	 * the *rpnt is NULL, so we need to allocate memory for it,
	 * and initialize the _dl_symbol_table.
	 */
	else {
		*rpnt = _dl_symbol_tables = _dl_malloc(sizeof(struct dyn_elf));
		_dl_memset(*rpnt, 0, sizeof(struct dyn_elf));
	}
#endif
	(*rpnt)->dyn = tpnt;
	tpnt->usage_count++;
#ifdef __LDSO_STANDALONE_SUPPORT__
	tpnt->libtype = (epnt->e_type == ET_DYN) ? elf_lib : elf_executable;
#else
	tpnt->libtype = elf_lib;
#endif

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

#ifdef __DSBT__
	/* Handle DSBT initialization */
	{
		struct elf_resolve *t, *ref;
		int idx = tpnt->loadaddr.map->dsbt_index;
		unsigned *dsbt = tpnt->loadaddr.map->dsbt_table;

		if (idx == 0) {
			if (!dynamic_info[DT_TEXTREL]) {
				/* This DSO has not been assigned an index. */
				_dl_dprintf(2, "%s: '%s' is missing a dsbt index assignment!\n",
					    _dl_progname, libname);
				_dl_exit(1);
			}
			/* Find a dsbt table from another module. */
			ref = NULL;
			for (t = _dl_loaded_modules; t; t = t->next) {
				if (ref == NULL && t != tpnt) {
					ref = t;
					break;
				}
			}
			idx = tpnt->loadaddr.map->dsbt_size;
			while (idx-- > 0)
				if (!ref || ref->loadaddr.map->dsbt_table[idx] == NULL)
					break;
			if (idx <= 0) {
				_dl_dprintf(2, "%s: '%s' caused DSBT table overflow!\n",
					    _dl_progname, libname);
				_dl_exit(1);
			}
			_dl_if_debug_dprint("\n\tfile='%s';  assigned index %d\n",
					    libname, idx);
			tpnt->loadaddr.map->dsbt_index = idx;

		}

		/*
		 * Setup dsbt slot for this module in dsbt of all modules.
		 */
		ref = NULL;
		for (t = _dl_loaded_modules; t; t = t->next) {
			/* find a dsbt table from another module */
			if (ref == NULL && t != tpnt) {
				ref = t;

				/* make sure index is not already used */
				if (t->loadaddr.map->dsbt_table[idx]) {
					struct elf_resolve *dup;
					char *dup_name;

					for (dup = _dl_loaded_modules; dup; dup = dup->next)
						if (dup != tpnt && dup->loadaddr.map->dsbt_index == idx)
							break;
					if (dup)
						dup_name = dup->libname;
					else if (idx == 1)
						dup_name = "runtime linker";
					else
						dup_name = "unknown library";
					_dl_dprintf(2, "%s: '%s' dsbt index %d already used by %s!\n",
						    _dl_progname, libname, idx, dup_name);
					_dl_exit(1);
				}
			}
			t->loadaddr.map->dsbt_table[idx] = (unsigned)dsbt;
		}
		if (ref)
			_dl_memcpy(dsbt, ref->loadaddr.map->dsbt_table,
				   tpnt->loadaddr.map->dsbt_size * sizeof(unsigned *));
	}
#endif
	_dl_if_debug_dprint("\n\tfile='%s';  generating link map\n", libname);
	_dl_if_debug_dprint("\t\tdynamic: %x  base: %x\n", dynamic_addr, DL_LOADADDR_BASE(lib_loadaddr));
	_dl_if_debug_dprint("\t\t  entry: %x  phdr: %x  phnum: %x\n\n",
			DL_RELOC_ADDR(lib_loadaddr, epnt->e_entry), tpnt->ppnt, tpnt->n_phent);

	_dl_munmap(header, _dl_pagesize);

	return tpnt;
}

/* now_flag must be RTLD_NOW or zero */
int _dl_fixup(struct dyn_elf *rpnt, struct r_scope_elem *scope, int now_flag)
{
	int goof = 0;
	struct elf_resolve *tpnt;
	ElfW(Word) reloc_size, relative_count;
	ElfW(Addr) reloc_addr;

	if (rpnt->next)
		goof = _dl_fixup(rpnt->next, scope, now_flag);
	if (goof)
		return goof;
	tpnt = rpnt->dyn;

	if (!(tpnt->init_flag & RELOCS_DONE))
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
#ifdef __LDSO_PRELINK_SUPPORT__
			if (tpnt->loadaddr || (!tpnt->dynamic_info[DT_GNU_PRELINKED_IDX]))
#endif
				elf_machine_relative(tpnt->loadaddr, reloc_addr, relative_count);
			reloc_addr += relative_count * sizeof(ELF_RELOC);
		}
		goof += _dl_parse_relocation_information(rpnt, scope,
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
			goof += _dl_parse_relocation_information(rpnt, scope,
					tpnt->dynamic_info[DT_JMPREL],
					tpnt->dynamic_info[DT_PLTRELSZ]);
		}
		tpnt->init_flag |= JMP_RELOCS_DONE;
	}

#if 0
/* _dl_add_to_slotinfo is called by init_tls() for initial DSO
   or by dlopen() for dynamically loaded DSO. */
#if defined(USE_TLS) && USE_TLS
	/* Add object to slot information data if necessasy. */
	if (tpnt->l_tls_blocksize != 0 && tls_init_tp_called)
		_dl_add_to_slotinfo ((struct link_map *) tpnt);
#endif
#endif
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
	char *buf;

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

unsigned int _dl_parse_dynamic_info(ElfW(Dyn) *dpnt, unsigned long dynamic_info[],
                                    void *debug_addr, DL_LOADADDR_TYPE load_off)
{
	return __dl_parse_dynamic_info(dpnt, dynamic_info, debug_addr, load_off);
}
