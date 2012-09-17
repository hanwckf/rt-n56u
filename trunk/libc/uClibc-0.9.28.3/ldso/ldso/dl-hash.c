/* vi: set sw=4 ts=4: */
/*
 * Program to load an ELF binary on a linux system, and run it
 * after resolving ELF shared library symbols
 *
 * Copyright (C) 2004 by Joakim Tjernlund <joakim.tjernlund@lumentis.se>
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
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


/* Various symbol table handling functions, including symbol lookup */

/*
 * This is the start of the linked list that describes all of the files present
 * in the system with pointers to all of the symbol, string, and hash tables,
 * as well as all of the other good stuff in the binary.
 */
struct elf_resolve *_dl_loaded_modules = NULL;

/*
 * This is the list of modules that are loaded when the image is first
 * started.  As we add more via dlopen, they get added into other
 * chains.
 */
struct dyn_elf *_dl_symbol_tables = NULL;

/*
 * This is the list of modules that are loaded via dlopen.  We may need
 * to search these for RTLD_GLOBAL files.
 */
struct dyn_elf *_dl_handles = NULL;


/* This is the hash function that is used by the ELF linker to generate the
 * hash table that each executable and library is required to have.  We need
 * it to decode the hash table.  */
static inline Elf_Symndx _dl_elf_hash(const char *name)
{
	unsigned long hash=0;
	unsigned long tmp;

	while (*name) {
		hash = (hash << 4) + *name++;
		tmp = hash & 0xf0000000;
		/* The algorithm specified in the ELF ABI is as follows:
		   if (tmp != 0)
		       hash ^= tmp >> 24;
		   hash &= ~tmp;
		   But the following is equivalent and a lot
		   faster, especially on modern processors. */
		hash ^= tmp;
		hash ^= tmp >> 24;
	}
	return hash;
}

/*
 * We call this function when we have just read an ELF library or executable.
 * We add the relevant info to the symbol chain, so that we can resolve all
 * externals properly.
 */
struct elf_resolve *_dl_add_elf_hash_table(const char *libname,
	char *loadaddr, unsigned long *dynamic_info, unsigned long dynamic_addr,
	//attribute_unused
	unsigned long dynamic_size)
{
	Elf_Symndx *hash_addr;
	struct elf_resolve *tpnt;
	int i;

	if (!_dl_loaded_modules) {
		tpnt = _dl_loaded_modules = (struct elf_resolve *) _dl_malloc(sizeof(struct elf_resolve));
		_dl_memset(tpnt, 0, sizeof(struct elf_resolve));
	} else {
		tpnt = _dl_loaded_modules;
		while (tpnt->next)
			tpnt = tpnt->next;
		tpnt->next = (struct elf_resolve *) _dl_malloc(sizeof(struct elf_resolve));
		_dl_memset(tpnt->next, 0, sizeof(struct elf_resolve));
		tpnt->next->prev = tpnt;
		tpnt = tpnt->next;
	};

	tpnt->next = NULL;
	tpnt->init_flag = 0;
	tpnt->libname = _dl_strdup(libname);
	tpnt->dynamic_addr = (ElfW(Dyn) *)dynamic_addr;
	tpnt->libtype = loaded_file;

	if (dynamic_info[DT_HASH] != 0) {
		hash_addr = (Elf_Symndx*)dynamic_info[DT_HASH];
		tpnt->nbucket = *hash_addr++;
		tpnt->nchain = *hash_addr++;
		tpnt->elf_buckets = hash_addr;
		hash_addr += tpnt->nbucket;
		tpnt->chains = hash_addr;
	}
	tpnt->loadaddr = (ElfW(Addr))loadaddr;
	for (i = 0; i < DYNAMIC_SIZE; i++)
		tpnt->dynamic_info[i] = dynamic_info[i];
	return tpnt;
}


/*
 * This function resolves externals, and this is either called when we process
 * relocations or when we call an entry in the PLT table for the first time.
 */
char *_dl_find_hash(const char *name, struct dyn_elf *rpnt, struct elf_resolve *mytpnt, int type_class)
{
	struct elf_resolve *tpnt;
	int si;
	char *strtab;
	ElfW(Sym) *symtab;
	unsigned long elf_hash_number, hn;
	const ElfW(Sym) *sym;
	char *weak_result = NULL;

	elf_hash_number = _dl_elf_hash(name);

	for (; rpnt; rpnt = rpnt->next) {
		tpnt = rpnt->dyn;

		if (!(tpnt->rtld_flags & RTLD_GLOBAL) && mytpnt) {
			if (mytpnt == tpnt)
				;
			else {
				struct init_fini_list *tmp;

				for (tmp = mytpnt->rtld_local; tmp; tmp = tmp->next) {
					if (tmp->tpnt == tpnt)
						break;
				}
				if (!tmp)
					continue;
			}
		}
		/* Don't search the executable when resolving a copy reloc. */
		if ((type_class &  ELF_RTYPE_CLASS_COPY) && tpnt->libtype == elf_executable)
			continue;

		/* Avoid calling .urem here. */
		do_rem(hn, elf_hash_number, tpnt->nbucket);
		symtab = (ElfW(Sym) *) (intptr_t) (tpnt->dynamic_info[DT_SYMTAB]);
		strtab = (char *) (tpnt->dynamic_info[DT_STRTAB]);

		for (si = tpnt->elf_buckets[hn]; si != STN_UNDEF; si = tpnt->chains[si]) {
			sym = &symtab[si];

			if (type_class & (sym->st_shndx == SHN_UNDEF))
				continue;
			if (_dl_strcmp(strtab + sym->st_name, name) != 0)
				continue;
			if (sym->st_value == 0)
				continue;
			if (ELF_ST_TYPE(sym->st_info) > STT_FUNC)
				continue;

			switch (ELF_ST_BIND(sym->st_info)) {
			case STB_WEAK:
#if 0
/* Perhaps we should support old style weak symbol handling
 * per what glibc does when you export LD_DYNAMIC_WEAK */
				if (!weak_result)
					weak_result = (char *)tpnt->loadaddr + sym->st_value;
				break;
#endif
			case STB_GLOBAL:
				return (char*)tpnt->loadaddr + sym->st_value;
			default:	/* Local symbols not handled here */
				break;
			}
		}
	}
	return weak_result;
}
