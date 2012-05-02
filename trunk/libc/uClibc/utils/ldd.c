/* vi: set sw=4 ts=4: */
/*
 * A small little ldd implementation for uClibc
 *
 * Copyright (C) 2000-2004 Erik Andersen <andersee@debian.org>
 *
 * Several functions in this file (specifically, elf_find_section_type(),
 * elf_find_phdr_type(), and elf_find_dynamic(), were stolen from elflib.c from
 * elfvector (http://www.BitWagon.com/elfvector.html) by John F. Reiser
 * <jreiser@BitWagon.com>, which is copyright 2000 BitWagon Software LLC
 * (GPL2).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */


#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "bswap.h"
#if defined (sun)
#include "link.h"
#else
#include "elf.h"
#endif
#include "dl-defs.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#if defined(__arm__)
#define MATCH_MACHINE(x) (x == EM_ARM)
#define ELFCLASSM	ELFCLASS32
#endif

#if defined(__s390__)
#define MATCH_MACHINE(x) (x == EM_S390)
#define ELFCLASSM	ELFCLASS32
#endif

#if defined(__i386__)
#ifndef EM_486
#define MATCH_MACHINE(x) (x == EM_386)
#else
#define MATCH_MACHINE(x) (x == EM_386 || x == EM_486)
#endif
#define ELFCLASSM	ELFCLASS32
#endif

#if defined(__mc68000__)
#define MATCH_MACHINE(x) (x == EM_68K)
#define ELFCLASSM	ELFCLASS32
#endif

#if defined(__mips__)
#define MATCH_MACHINE(x) (x == EM_MIPS || x == EM_MIPS_RS3_LE)
#define ELFCLASSM	ELFCLASS32
#endif

#if defined(__powerpc__)
#define MATCH_MACHINE(x) (x == EM_PPC)
#define ELFCLASSM	ELFCLASS32
#endif

#if defined(__sh__)
#define MATCH_MACHINE(x) (x == EM_SH)
#define ELFCLASSM	ELFCLASS32
#endif

#if defined (__v850e__)
#define MATCH_MACHINE(x) ((x) == EM_V850 || (x) == EM_CYGNUS_V850)
#define ELFCLASSM	ELFCLASS32
#endif

#if defined (__sparc__)
#define MATCH_MACHINE(x) ((x) == EM_SPARC || (x) == EM_SPARC32PLUS)
#define ELFCLASSM    ELFCLASS32
#endif

#if defined(__cris__)
#define MATCH_MACHINE(x) (x == EM_CRIS)
#define ELFCLASSM	ELFCLASS32
#endif

#ifndef MATCH_MACHINE
#warning "You really should add a MATCH_MACHINE() macro for your architecture"
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ELFDATAM	ELFDATA2LSB
#elif __BYTE_ORDER == __BIG_ENDIAN
#define ELFDATAM	ELFDATA2MSB
#endif

struct library {
	char *name;
	int resolved;
	char *path;
	struct library *next;
};
struct library *lib_list = NULL;
char not_found[] = "not found";
char *interp = NULL;
char *interp_dir = NULL;
int byteswap;
static int interpreter_already_found=0;

inline uint32_t byteswap32_to_host(uint32_t value)
{
	if (byteswap==1) {
		return(bswap_32(value));
	} else {
		return(value);
	}
}

Elf32_Shdr * elf_find_section_type( int key, Elf32_Ehdr *ehdr)
{
	int j;
	Elf32_Shdr *shdr;
	shdr = (Elf32_Shdr *)(ehdr->e_shoff + (char *)ehdr);
	for (j = ehdr->e_shnum; --j>=0; ++shdr) {
		if (key==(int)byteswap32_to_host(shdr->sh_type)) {
			return shdr;
		}
	}
	return NULL;
}

Elf32_Phdr * elf_find_phdr_type( int type, Elf32_Ehdr *ehdr)
{
	int j;
	Elf32_Phdr *phdr = (Elf32_Phdr *)(ehdr->e_phoff + (char *)ehdr);
	for (j = ehdr->e_phnum; --j>=0; ++phdr) {
		if (type==(int)byteswap32_to_host(phdr->p_type)) {
			return phdr;
		}
	}
	return NULL;
}

/* Returns value if return_val==1, ptr otherwise */
void * elf_find_dynamic(int const key, Elf32_Dyn *dynp,
	Elf32_Ehdr *ehdr, int return_val)
{
	Elf32_Phdr *pt_text = elf_find_phdr_type(PT_LOAD, ehdr);
	unsigned tx_reloc = byteswap32_to_host(pt_text->p_vaddr) - byteswap32_to_host(pt_text->p_offset);
	for (; DT_NULL!=byteswap32_to_host(dynp->d_tag); ++dynp) {
		if (key == (int)byteswap32_to_host(dynp->d_tag)) {
			if (return_val == 1)
				return (void *)(intptr_t)byteswap32_to_host(dynp->d_un.d_val);
			else
				return (void *)(byteswap32_to_host(dynp->d_un.d_val) - tx_reloc + (char *)ehdr );
		}
	}
	return NULL;
}

static char * elf_find_rpath(Elf32_Ehdr* ehdr, Elf32_Dyn* dynamic)
{
	Elf32_Dyn  *dyns;

	for (dyns=dynamic; byteswap32_to_host(dyns->d_tag)!=DT_NULL; ++dyns) {
		if (DT_RPATH == byteswap32_to_host(dyns->d_tag)) {
			char *strtab;
			strtab = (char *)elf_find_dynamic(DT_STRTAB, dynamic, ehdr, 0);
			return ((char*)strtab + byteswap32_to_host(dyns->d_un.d_val));
		}
	}
	return NULL;
}

int check_elf_header(Elf32_Ehdr *const ehdr)
{
	if (! ehdr || strncmp((void *)ehdr, ELFMAG, SELFMAG) != 0 ||
			ehdr->e_ident[EI_CLASS] != ELFCLASS32 ||
			ehdr->e_ident[EI_VERSION] != EV_CURRENT)
	{
		return 1;
	}

	/* Check if the target endianness matches the host's endianness */
	byteswap = 0;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	if (ehdr->e_ident[5] == ELFDATA2MSB) {
		/* Ick -- we will have to byte-swap everything */
		byteswap = 1;
	}
#elif __BYTE_ORDER == __BIG_ENDIAN
	if (ehdr->e_ident[5] == ELFDATA2LSB) {
		/* Ick -- we will have to byte-swap everything */
		byteswap = 1;
	}
#else
#error Unknown host byte order!
#endif

	/* Be vary lazy, and only byteswap the stuff we use */
	if (byteswap==1) {
		ehdr->e_type=bswap_16(ehdr->e_type);
		ehdr->e_phoff=bswap_32(ehdr->e_phoff);
		ehdr->e_shoff=bswap_32(ehdr->e_shoff);
		ehdr->e_phnum=bswap_16(ehdr->e_phnum);
		ehdr->e_shnum=bswap_16(ehdr->e_shnum);
	}

	return 0;
}

#ifdef __LDSO_CACHE_SUPPORT__
static caddr_t cache_addr = NULL;
static size_t cache_size = 0;

int map_cache(void)
{
	int fd;
	struct stat st;
	header_t *header;
	libentry_t *libent;
	int i, strtabsize;

	if (cache_addr == (caddr_t) - 1)
		return -1;
	else if (cache_addr != NULL)
		return 0;

	if (stat(LDSO_CACHE, &st)
			|| (fd = open(LDSO_CACHE, O_RDONLY, 0)) < 0) {
		dprintf(2, "ldd: can't open cache '%s'\n", LDSO_CACHE);
		cache_addr = (caddr_t) - 1;	/* so we won't try again */
		return -1;
	}

	cache_size = st.st_size;
	cache_addr = (caddr_t) mmap(0, cache_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	if (cache_addr == MAP_FAILED) {
		dprintf(2, "ldd: can't map cache '%s'\n", LDSO_CACHE);
		return -1;
	}

	header = (header_t *) cache_addr;

	if (cache_size < sizeof(header_t) ||
			memcmp(header->magic, LDSO_CACHE_MAGIC, LDSO_CACHE_MAGIC_LEN)
			|| memcmp(header->version, LDSO_CACHE_VER, LDSO_CACHE_VER_LEN)
			|| cache_size <
			(sizeof(header_t) + header->nlibs * sizeof(libentry_t))
			|| cache_addr[cache_size - 1] != '\0')
	{
		dprintf(2, "ldd: cache '%s' is corrupt\n", LDSO_CACHE);
		goto fail;
	}

	strtabsize = cache_size - sizeof(header_t) -
		header->nlibs * sizeof(libentry_t);
	libent = (libentry_t *) & header[1];

	for (i = 0; i < header->nlibs; i++) {
		if (libent[i].sooffset >= strtabsize ||
				libent[i].liboffset >= strtabsize)
		{
			dprintf(2, "ldd: cache '%s' is corrupt\n", LDSO_CACHE);
			goto fail;
		}
	}

	return 0;

fail:
	munmap(cache_addr, cache_size);
	cache_addr = (caddr_t) - 1;
	return -1;
}

int unmap_cache(void)
{
	if (cache_addr == NULL || cache_addr == (caddr_t) - 1)
		return -1;

#if 1
	munmap(cache_addr, cache_size);
	cache_addr = NULL;
#endif

	return 0;
}
#else
static inline void map_cache(void) { }
static inline void unmap_cache(void) { }
#endif

/* This function's behavior must exactly match that
 * in uClibc/ldso/ldso/dl-elf.c */
static void search_for_named_library(char *name, char *result, const char *path_list)
{
	int i, count = 1;
	char *path, *path_n;
	struct stat filestat;

	/* We need a writable copy of this string */
	path = strdup(path_list);
	if (!path) {
		fprintf(stderr, "Out of memory!\n");
		exit(EXIT_FAILURE);
	}
	/* Eliminate all double //s */
	path_n=path;
	while((path_n=strstr(path_n, "//"))) {
		i = strlen(path_n);
		memmove(path_n, path_n+1, i-1);
		*(path_n + i - 1)='\0';
	}

	/* Replace colons with zeros in path_list and count them */
	for(i=strlen(path); i > 0; i--) {
		if (path[i]==':') {
			path[i]=0;
			count++;
		}
	}
	path_n = path;
	for (i = 0; i < count; i++) {
		strcpy(result, path_n);
		strcat(result, "/");
		strcat(result, name);
		if (stat (result, &filestat) == 0 && filestat.st_mode & S_IRUSR) {
			free(path);
			return;
		}
		path_n += (strlen(path_n) + 1);
	}
	free(path);
	*result = '\0';
}

void locate_library_file(Elf32_Ehdr* ehdr, Elf32_Dyn* dynamic, int is_suid, struct library *lib)
{
	char *buf;
	char *path;
	struct stat filestat;

	/* If this is a fully resolved name, our job is easy */
	if (stat (lib->name, &filestat) == 0) {
		lib->path = strdup(lib->name);
		return;
	}

	/* We need some elbow room here.  Make some room...*/
	buf = malloc(1024);
	if (!buf) {
		fprintf(stderr, "Out of memory!\n");
		exit(EXIT_FAILURE);
	}

	/* This function must match the behavior of _dl_load_shared_library
	 * in readelflib1.c or things won't work out as expected... */

	/* The ABI specifies that RPATH is searched first, so do that now.  */
	path = elf_find_rpath(ehdr, dynamic);
	if (path) {
		search_for_named_library(lib->name, buf, path);
		if (*buf != '\0') {
			lib->path = buf;
			return;
		}
	}

	/* Next check LD_{ELF_}LIBRARY_PATH if specified and allowed.
	 * Since this app doesn't actually run an executable I will skip
	 * the suid check, and just use LD_{ELF_}LIBRARY_PATH if set */
	if (is_suid==1)
		path = NULL;
	else
		path = getenv("LD_LIBRARY_PATH");
	if (path) {
		search_for_named_library(lib->name, buf, path);
		if (*buf != '\0') {
			lib->path = buf;
			return;
		}
	}

#ifdef __LDSO_CACHE_SUPPORT__
	if (cache_addr != NULL && cache_addr != (caddr_t) - 1) {
		int i;
		header_t *header = (header_t *) cache_addr;
		libentry_t *libent = (libentry_t *) & header[1];
		char *strs = (char *) &libent[header->nlibs];

		for (i = 0; i < header->nlibs; i++) {
			if ((libent[i].flags == LIB_ELF ||
			    libent[i].flags == LIB_ELF_LIBC0 ||
			    libent[i].flags == LIB_ELF_LIBC5) &&
			    strcmp(lib->name, strs + libent[i].sooffset) == 0) {
				lib->path = strdup(strs + libent[i].liboffset);
				return;
			}
		}
	}
#endif


	/* Next look for libraries wherever the shared library
	 * loader was installed -- this is usually where we
	 * should find things... */
	if (interp_dir) {
		search_for_named_library(lib->name, buf, interp_dir);
		if (*buf != '\0') {
			lib->path = buf;
			return;
		}
	}

	/* Lastly, search the standard list of paths for the library.
	   This list must exactly match the list in uClibc/ldso/ldso/dl-elf.c */
	path =	UCLIBC_RUNTIME_PREFIX "lib:"
		UCLIBC_RUNTIME_PREFIX "usr/lib"
#ifndef __LDSO_CACHE_SUPPORT__
		":" UCLIBC_RUNTIME_PREFIX "usr/X11R6/lib"
#endif
		;
	search_for_named_library(lib->name, buf, path);
	if (*buf != '\0') {
		lib->path = buf;
	} else {
		free(buf);
		lib->path = not_found;
	}
}

static int add_library(Elf32_Ehdr* ehdr, Elf32_Dyn* dynamic, int is_setuid, char *s)
{
	char *tmp, *tmp1, *tmp2;
	struct library *cur, *newlib=lib_list;

	if (!s || !strlen(s))
		return 1;

	tmp = s;
	while (*tmp) {
		if (*tmp == '/')
			s = tmp + 1;
		tmp++;
	}

	/* We add ldso elsewhere */
	if (interpreter_already_found && (tmp=strrchr(interp, '/')) != NULL)
	{
		int len = strlen(interp_dir);
		if (strcmp(s, interp+1+len)==0)
			return 1;
	}

	for (cur = lib_list; cur; cur=cur->next) {
		/* Check if this library is already in the list */
		tmp1 = tmp2 = cur->name;
		while (*tmp1) {
			if (*tmp1 == '/')
				tmp2 = tmp1 + 1;
			tmp1++;
		}
		if(strcmp(tmp2, s)==0) {
			//printf("find_elf_interpreter is skipping '%s' (already in list)\n", cur->name);
			return 0;
		}
	}

	/* Ok, this lib needs to be added to the list */
	newlib = malloc(sizeof(struct library));
	if (!newlib)
		return 1;
	newlib->name = malloc(strlen(s)+1);
	strcpy(newlib->name, s);
	newlib->resolved = 0;
	newlib->path = NULL;
	newlib->next = NULL;

	/* Now try and locate where this library might be living... */
	locate_library_file(ehdr, dynamic, is_setuid, newlib);

	//printf("add_library is adding '%s' to '%s'\n", newlib->name, newlib->path);
	if (!lib_list) {
		lib_list = newlib;
	} else {
		for (cur = lib_list;  cur->next; cur=cur->next); /* nothing */
		cur->next = newlib;
	}
	return 0;
}

static void find_needed_libraries(Elf32_Ehdr* ehdr,
		Elf32_Dyn* dynamic, int is_setuid)
{
	Elf32_Dyn  *dyns;

	for (dyns=dynamic; byteswap32_to_host(dyns->d_tag)!=DT_NULL; ++dyns) {
		if (DT_NEEDED == byteswap32_to_host(dyns->d_tag)) {
			char *strtab;
			strtab = (char *)elf_find_dynamic(DT_STRTAB, dynamic, ehdr, 0);
			add_library(ehdr, dynamic, is_setuid,
					(char*)strtab + byteswap32_to_host(dyns->d_un.d_val));
		}
	}
}

static struct library * find_elf_interpreter(Elf32_Ehdr* ehdr)
{
	Elf32_Phdr *phdr;

	if (interpreter_already_found==1)
		return NULL;
	phdr = elf_find_phdr_type(PT_INTERP, ehdr);
	if (phdr) {
		struct library *cur, *newlib=NULL;
		char *s = (char*)ehdr + byteswap32_to_host(phdr->p_offset);

		char *tmp, *tmp1;
		interp = strdup(s);
		interp_dir = strdup(s);
		tmp = strrchr(interp_dir, '/');
		if (*tmp)
			*tmp = '\0';
		else {
			free(interp_dir);
			interp_dir = interp;
		}
		tmp1 = tmp = s;
		while (*tmp) {
			if (*tmp == '/')
				tmp1 = tmp + 1;
			tmp++;
		}
		for (cur = lib_list; cur; cur=cur->next) {
			/* Check if this library is already in the list */
			if(strcmp(cur->name, tmp1)==0) {
				//printf("find_elf_interpreter is replacing '%s' (already in list)\n", cur->name);
				newlib = cur;
				free(newlib->name);
				if (newlib->path != not_found) {
					free(newlib->path);
				}
				newlib->name = NULL;
				newlib->path = NULL;
				return NULL;
			}
		}
		if (newlib == NULL)
			newlib = malloc(sizeof(struct library));
		if (!newlib)
			return NULL;
		newlib->name = malloc(strlen(s)+1);
		strcpy(newlib->name, s);
		newlib->path = strdup(newlib->name);
		newlib->resolved = 1;
		newlib->next = NULL;

#if 0
		//printf("find_elf_interpreter is adding '%s' to '%s'\n", newlib->name, newlib->path);
		if (!lib_list) {
			lib_list = newlib;
		} else {
			for (cur = lib_list;  cur->next; cur=cur->next); /* nothing */
			cur->next = newlib;
		}
#endif
		interpreter_already_found=1;
		return newlib;
	}
	return NULL;
}

/* map the .so, and locate interesting pieces */
int find_dependancies(char* filename)
{
	int is_suid = 0;
	FILE *thefile;
	struct stat statbuf;
	Elf32_Ehdr *ehdr = NULL;
	Elf32_Shdr *dynsec = NULL;
	Elf32_Dyn *dynamic = NULL;
	struct library *interp;

	if (filename == not_found)
		return 0;

	if (!filename) {
		fprintf(stderr, "No filename specified.\n");
		return -1;
	}
	if (!(thefile = fopen(filename, "r"))) {
		perror(filename);
		return -1;
	}
	if (fstat(fileno(thefile), &statbuf) < 0) {
		perror(filename);
		fclose(thefile);
		return -1;
	}

	if ((size_t)statbuf.st_size < sizeof(Elf32_Ehdr))
		goto foo;

	if (!S_ISREG(statbuf.st_mode))
		goto foo;

	/* mmap the file to make reading stuff from it effortless */
	ehdr = (Elf32_Ehdr *)mmap(0, statbuf.st_size,
			PROT_READ|PROT_WRITE, MAP_PRIVATE, fileno(thefile), 0);
	if (ehdr == MAP_FAILED) {
		fclose(thefile);
		fprintf(stderr, "Out of memory!\n");
		return -1;
	}

foo:
	fclose(thefile);

	/* Check if this looks like a legit ELF file */
	if (check_elf_header(ehdr)) {
		fprintf(stderr, "%s: not an ELF file.\n", filename);
		return -1;
	}
	/* Check if this is the right kind of ELF file */
	if (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN) {
		fprintf(stderr, "%s: not a dynamic executable\n", filename);
		return -1;
	}
	if (ehdr->e_type == ET_EXEC || ehdr->e_type == ET_DYN) {
		if (statbuf.st_mode & S_ISUID)
			is_suid = 1;
		if ((statbuf.st_mode & (S_ISGID | S_IXGRP)) == (S_ISGID | S_IXGRP))
			is_suid = 1;
		/* FIXME */
		if (is_suid)
			fprintf(stderr, "%s: is setuid\n", filename);
	}

	interpreter_already_found=0;
	interp = find_elf_interpreter(ehdr);

#ifdef __LDSO_LDD_SUPPORT__
	if (interp && (ehdr->e_type == ET_EXEC || ehdr->e_type == ET_DYN) && ehdr->e_ident[EI_CLASS] == ELFCLASSM &&
			ehdr->e_ident[EI_DATA] == ELFDATAM
			&& ehdr->e_ident[EI_VERSION] == EV_CURRENT && MATCH_MACHINE(ehdr->e_machine))
	{
		struct stat statbuf;
		if (stat(interp->path, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
			pid_t pid;
			int status;
			static const char * const environment[] = {
				"PATH=/usr/bin:/bin:/usr/sbin:/sbin",
				"SHELL=/bin/sh",
				"LD_TRACE_LOADED_OBJECTS=1",
				NULL
			};

			if ((pid = fork()) == 0) {
				/* Cool, it looks like we should be able to actually
				 * run this puppy.  Do so now... */
				execle(filename, filename, NULL, environment);
				_exit(0xdead);
			}

			/* Wait till it returns */
			waitpid(pid, &status, 0);
			if (WIFEXITED(status) && WEXITSTATUS(status)==0) {
				return 1;
			}

			/* If the exec failed, we fall through to trying to find
			 * all the needed libraries ourselves by rummaging about
			 * in the ELF headers... */
		}
	}
#endif

	dynsec = elf_find_section_type(SHT_DYNAMIC, ehdr);
	if (dynsec) {
		dynamic = (Elf32_Dyn*)(byteswap32_to_host(dynsec->sh_offset) + (intptr_t)ehdr);
		find_needed_libraries(ehdr, dynamic, is_suid);
	}

	return 0;
}

int main( int argc, char** argv)
{
	int multi=0;
	int got_em_all=1;
	char *filename = NULL;
	struct library *cur;

	if (argc < 2) {
		fprintf(stderr, "ldd: missing file arguments\n");
		fprintf(stderr, "Try `ldd --help' for more information.\n");
		exit(EXIT_FAILURE);
	}
	if (argc > 2) {
		multi++;
	}

	while (--argc > 0) {
		++argv;

		if(strcmp(*argv, "--")==0) {
			/* Ignore "--" */
			continue;
		}

		if(strcmp(*argv, "--help")==0) {
			fprintf(stderr, "Usage: ldd [OPTION]... FILE...\n");
			fprintf(stderr, "\t--help\t\tprint this help and exit\n");
			exit(EXIT_FAILURE);
		}

		filename=*argv;
		if (!filename) {
			fprintf(stderr, "No filename specified.\n");
			exit(EXIT_FAILURE);
		}

		if (multi) {
			printf("%s:\n", *argv);
		}

		map_cache();

		if (find_dependancies(filename)!=0)
			continue;

		while(got_em_all) {
			got_em_all=0;
			/* Keep walking the list till everybody is resolved */
			for (cur = lib_list; cur; cur=cur->next) {
				if (cur->resolved == 0 && cur->path) {
					got_em_all=1;
					//printf("checking sub-depends for '%s\n", cur->path);
					find_dependancies(cur->path);
					cur->resolved = 1;
				}
			}
		}

		unmap_cache();

		/* Print the list */
		got_em_all=0;
		for (cur = lib_list; cur; cur=cur->next) {
			got_em_all=1;
			printf("\t%s => %s (0x00000000)\n", cur->name, cur->path);
		}
		if (interp && interpreter_already_found==1)
			printf("\t%s => %s (0x00000000)\n", interp, interp);
		else
			printf("\tnot a dynamic executable\n");

		for (cur = lib_list; cur; cur=cur->next) {
			free(cur->name);
			cur->name=NULL;
			if (cur->path && cur->path != not_found) {
				free(cur->path);
				cur->path=NULL;
			}
		}
		lib_list=NULL;
	}

	return 0;
}

