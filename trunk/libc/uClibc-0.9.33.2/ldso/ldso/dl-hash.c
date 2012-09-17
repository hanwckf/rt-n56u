/* vi: set sw=4 ts=4: */
/*
 * Program to load an ELF binary on a linux system, and run it
 * after resolving ELF shared library symbols
 *
 * Copyright (C) 2004 by Joakim Tjernlund <joakim.tjernlund@lumentis.se>
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


/* Various symbol table handling functions, including symbol lookup */
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

#ifdef __LDSO_GNU_HASH_SUPPORT__
/* This is the new hash function that is used by the ELF linker to generate the
 * GNU hash table that each executable and library will have if --hash-style=[gnu,both]
 * is passed to the linker. We need it to decode the GNU hash table.  */
static __inline__ Elf_Symndx _dl_gnu_hash (const unsigned char *name)
{
  unsigned long h = 5381;
  unsigned char c;
  for (c = *name; c != '\0'; c = *++name)
    h = h * 33 + c;
  return h & 0xffffffff;
}
#endif

/* This is the hash function that is used by the ELF linker to generate the
 * hash table that each executable and library is required to have.  We need
 * it to decode the hash table.  */
static __inline__ Elf_Symndx _dl_elf_hash(const unsigned char *name)
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
	DL_LOADADDR_TYPE loadaddr, unsigned long *dynamic_info, unsigned long dynamic_addr,
	attribute_unused unsigned long dynamic_size)
{
	Elf_Symndx *hash_addr;
	struct elf_resolve *tpnt;
	int i;

	tpnt = _dl_malloc(sizeof(struct elf_resolve));
	_dl_memset(tpnt, 0, sizeof(struct elf_resolve));

	if (!_dl_loaded_modules)
		_dl_loaded_modules = tpnt;
	else {
		struct elf_resolve *t = _dl_loaded_modules;
		while (t->next)
			t = t->next;
		t->next = tpnt;
		t->next->prev = t;
		tpnt = t->next;
	}

	tpnt->next = NULL;
	tpnt->init_flag = 0;
	tpnt->libname = _dl_strdup(libname);
	tpnt->dynamic_addr = (ElfW(Dyn) *)dynamic_addr;
	tpnt->libtype = loaded_file;

#ifdef __LDSO_GNU_HASH_SUPPORT__
	if (dynamic_info[DT_GNU_HASH_IDX] != 0) {
		Elf32_Word *hash32 = (Elf_Symndx*)dynamic_info[DT_GNU_HASH_IDX];

		tpnt->nbucket = *hash32++;
		Elf32_Word symbias = *hash32++;
		Elf32_Word bitmask_nwords = *hash32++;
		/* Must be a power of two.  */
		_dl_assert ((bitmask_nwords & (bitmask_nwords - 1)) == 0);
		tpnt->l_gnu_bitmask_idxbits = bitmask_nwords - 1;
		tpnt->l_gnu_shift = *hash32++;

		tpnt->l_gnu_bitmask = (ElfW(Addr) *) hash32;
		hash32 += __ELF_NATIVE_CLASS / 32 * bitmask_nwords;

		tpnt->l_gnu_buckets = hash32;
		hash32 += tpnt->nbucket;
		tpnt->l_gnu_chain_zero = hash32 - symbias;
	} else
	/* Fall using old SysV hash table if GNU hash is not present */
#endif

	if (dynamic_info[DT_HASH] != 0) {
		hash_addr = (Elf_Symndx*)dynamic_info[DT_HASH];
		tpnt->nbucket = *hash_addr++;
		tpnt->nchain = *hash_addr++;
		tpnt->elf_buckets = hash_addr;
		hash_addr += tpnt->nbucket;
		tpnt->chains = hash_addr;
	}
	tpnt->loadaddr = loadaddr;
	for (i = 0; i < DYNAMIC_SIZE; i++)
		tpnt->dynamic_info[i] = dynamic_info[i];
	return tpnt;
}


/* Routine to check whether the symbol matches.  */
static __attribute_noinline__ const ElfW(Sym) *
check_match (const ElfW(Sym) *sym, char *strtab, const char* undef_name, int type_class)
{

#if defined(USE_TLS) && USE_TLS
	if ((sym->st_value == 0 && (ELF_ST_TYPE(sym->st_info) != STT_TLS))
		      || (type_class & (sym->st_shndx == SHN_UNDEF)))
		/* No value or undefined symbol itself */
		return NULL;

	if (ELF_ST_TYPE(sym->st_info) > STT_FUNC
		&& ELF_ST_TYPE(sym->st_info) != STT_COMMON
		&& ELF_ST_TYPE(sym->st_info) != STT_TLS)
		/* Ignore all but STT_NOTYPE, STT_OBJECT, STT_FUNC and STT_COMMON
		 * entries (and STT_TLS if TLS is supported) since these
		 * are no code/data definitions.
		 */
		return NULL;
#else
	if (type_class & (sym->st_shndx == SHN_UNDEF))
		/* undefined symbol itself */
		return NULL;

	if (sym->st_value == 0)
		/* No value */
		return NULL;

	if (ELF_ST_TYPE(sym->st_info) > STT_FUNC
		&& ELF_ST_TYPE(sym->st_info) != STT_COMMON)
		/* Ignore all but STT_NOTYPE, STT_OBJECT, STT_FUNC
		 * and STT_COMMON entries since these are no
		 * code/data definitions
		 */
		return NULL;
#endif
#ifdef ARCH_SKIP_RELOC
	if (ARCH_SKIP_RELOC(type_class, sym))
		return NULL;
#endif
	if (_dl_strcmp(strtab + sym->st_name, undef_name) != 0)
		return NULL;

	/* This is the matching symbol */
	return sym;
}


#ifdef __LDSO_GNU_HASH_SUPPORT__

static __always_inline const ElfW(Sym) *
_dl_lookup_gnu_hash(struct elf_resolve *tpnt, ElfW(Sym) *symtab, unsigned long hash,
					const char* undef_name, int type_class)
{
	Elf_Symndx symidx;
	const ElfW(Sym) *sym;
	char *strtab;

	const ElfW(Addr) *bitmask = tpnt->l_gnu_bitmask;

	ElfW(Addr) bitmask_word	= bitmask[(hash / __ELF_NATIVE_CLASS) & tpnt->l_gnu_bitmask_idxbits];

	unsigned int hashbit1 = hash & (__ELF_NATIVE_CLASS - 1);
	unsigned int hashbit2 = ((hash >> tpnt->l_gnu_shift) & (__ELF_NATIVE_CLASS - 1));
	_dl_assert (bitmask != NULL);

	if (unlikely((bitmask_word >> hashbit1) & (bitmask_word >> hashbit2) & 1)) {
		unsigned long rem;
		Elf32_Word bucket;

		do_rem (rem, hash, tpnt->nbucket);
		bucket = tpnt->l_gnu_buckets[rem];

		if (bucket != 0) {
			const Elf32_Word *hasharr = &tpnt->l_gnu_chain_zero[bucket];
			do {
				if (((*hasharr ^ hash) >> 1) == 0) {
					symidx = hasharr - tpnt->l_gnu_chain_zero;
					strtab = (char *) (tpnt->dynamic_info[DT_STRTAB]);
					sym = check_match (&symtab[symidx], strtab, undef_name, type_class);
					if (sym != NULL)
						return sym;
				}
			} while ((*hasharr++ & 1u) == 0);
		}
	}
	/* No symbol found.  */
	return NULL;
}
#endif

static __always_inline const ElfW(Sym) *
_dl_lookup_sysv_hash(struct elf_resolve *tpnt, ElfW(Sym) *symtab, unsigned long hash,  const char* undef_name, int type_class)
{
	unsigned long hn;
	char *strtab;
	const ElfW(Sym) *sym;
	Elf_Symndx symidx;

	/* Avoid calling .urem here. */
	do_rem(hn, hash, tpnt->nbucket);
	strtab = (char *) (tpnt->dynamic_info[DT_STRTAB]);

	_dl_assert(tpnt->elf_buckets != NULL);

	for (symidx = tpnt->elf_buckets[hn]; symidx != STN_UNDEF; symidx = tpnt->chains[symidx]) {
		sym = check_match (&symtab[symidx], strtab, undef_name, type_class);
		if (sym != NULL)
			/* At this point the symbol is that we are looking for */
			return sym;
	}
	/* No symbol found into the current module*/
	return NULL;
}

/*
 * This function resolves externals, and this is either called when we process
 * relocations or when we call an entry in the PLT table for the first time.
 */
char *_dl_find_hash(const char *name, struct r_scope_elem *scope, struct elf_resolve *mytpnt,
	int type_class, struct symbol_ref *sym_ref)
{
	struct elf_resolve *tpnt = NULL;
	ElfW(Sym) *symtab;
	int i = 0;

	unsigned long elf_hash_number = 0xffffffff;
	const ElfW(Sym) *sym = NULL;

	char *weak_result = NULL;
	struct r_scope_elem *loop_scope;

#ifdef __LDSO_GNU_HASH_SUPPORT__
	unsigned long gnu_hash_number = _dl_gnu_hash((const unsigned char *)name);
#endif

	if ((sym_ref) && (sym_ref->sym) && (ELF32_ST_VISIBILITY(sym_ref->sym->st_other) == STV_PROTECTED)) {
			sym = sym_ref->sym;
		if (mytpnt)
			tpnt = mytpnt;
	} else
	for (loop_scope = scope; loop_scope && !sym; loop_scope = loop_scope->next) {
		for (i = 0; i < loop_scope->r_nlist; i++) {
			tpnt = loop_scope->r_list[i];

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

			/* If the hash table is empty there is nothing to do here.  */
			if (tpnt->nbucket == 0)
				continue;

			symtab = (ElfW(Sym) *) (intptr_t) (tpnt->dynamic_info[DT_SYMTAB]);

#ifdef __LDSO_GNU_HASH_SUPPORT__
			/* Prefer GNU hash style, if any */
			if (tpnt->l_gnu_bitmask) {
				sym = _dl_lookup_gnu_hash(tpnt, symtab, gnu_hash_number, name, type_class);
				if (sym != NULL)
					/* If sym has been found, do not search further */
					break;
			} else {
#endif
				/* Use the old SysV-style hash table */

				/* Calculate the old sysv hash number only once */
				if (elf_hash_number == 0xffffffff)
					elf_hash_number = _dl_elf_hash((const unsigned char *)name);

				sym = _dl_lookup_sysv_hash(tpnt, symtab, elf_hash_number, name, type_class);
				if (sym != NULL)
					/* If sym has been found, do not search further */
					break;
#ifdef __LDSO_GNU_HASH_SUPPORT__
			}
#endif
		} /* End of inner for */
	}

	if (sym) {
		if (sym_ref) {
			sym_ref->sym = sym;
			sym_ref->tpnt = tpnt;
		}
		/* At this point we have found the requested symbol, do binding */
#if defined(USE_TLS) && USE_TLS
		if (ELF_ST_TYPE(sym->st_info) == STT_TLS) {
			_dl_assert(sym_ref != NULL);
			sym_ref->tpnt = tpnt;
			return (char *)sym->st_value;
		}
#endif

		switch (ELF_ST_BIND(sym->st_info)) {
			case STB_WEAK:
#if 0
	/* Perhaps we should support old style weak symbol handling
	* per what glibc does when you export LD_DYNAMIC_WEAK */
				if (!weak_result)
					weak_result = (char *)DL_FIND_HASH_VALUE(tpnt, type_class, sym);
				break;
#endif
			case STB_GLOBAL:
#ifdef __FDPIC__
			if (sym_ref)
				sym_ref->tpnt = tpnt;
#endif
				return (char *)DL_FIND_HASH_VALUE(tpnt, type_class, sym);
			default:	/* Local symbols not handled here */
				break;
		}
	}
#ifdef __FDPIC__
	if (sym_ref)
		sym_ref->tpnt = tpnt;
#endif
	return weak_result;
}
