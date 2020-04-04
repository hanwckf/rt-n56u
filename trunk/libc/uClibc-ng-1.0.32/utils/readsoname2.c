static char *readsonameXX(char *name, FILE *infile, int expected_type, int *type)
{
	ElfW(Ehdr) *epnt;
	ElfW(Phdr) *ppnt;
	unsigned int i, j;
	char *header;
	ElfW(Addr) dynamic_addr = 0;
	ElfW(Addr) dynamic_size = 0;
	unsigned long page_size = getpagesize();
	ElfW(Addr) strtab_val = 0;
	ElfW(Addr) needed_val;
	ElfW(Addr) loadaddr = -1;
	ElfW(Dyn) *dpnt;
	struct stat st;
	char *needed;
	char *soname = NULL;
	int multi_libcs = 0;

	if (expected_type == LIB_DLL) {
		warn("%s does not match type specified for directory!", name);
		expected_type = LIB_ANY;
	}

	*type = LIB_ELF;

	if (fstat(fileno(infile), &st))
		return NULL;
	header =
	    mmap(0, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
		 fileno(infile), 0);
	if (header == (caddr_t) - 1)
		return NULL;

	epnt = (ElfW(Ehdr) *) header;
	if ((char *)(epnt + 1) > (char *)(header + st.st_size))
		goto skip;

	if (UCLIBC_ENDIAN_HOST == UCLIBC_ENDIAN_LITTLE)
		byteswap = (epnt->e_ident[5] == ELFDATA2MSB) ? 1 : 0;
	else if (UCLIBC_ENDIAN_HOST == UCLIBC_ENDIAN_BIG)
		byteswap = (epnt->e_ident[5] == ELFDATA2LSB) ? 1 : 0;

	/* Be very lazy, and only byteswap the stuff we use */
	if (byteswap == 1) {
		epnt->e_phoff = bswap_32(epnt->e_phoff);
		epnt->e_phnum = bswap_16(epnt->e_phnum);
	}

	ppnt = (ElfW(Phdr) *) & header[epnt->e_phoff];
	if ((char *)ppnt < (char *)header ||
	    (char *)(ppnt + epnt->e_phnum) > (char *)(header + st.st_size))
		goto skip;

	for (i = 0; i < epnt->e_phnum; i++) {
		/* Be very lazy, and only byteswap the stuff we use */
		if (byteswap == 1) {
			ppnt->p_type = bswap_32(ppnt->p_type);
			ppnt->p_vaddr = bswap_32(ppnt->p_vaddr);
			ppnt->p_offset = bswap_32(ppnt->p_offset);
			ppnt->p_filesz = bswap_32(ppnt->p_filesz);
		}

		if (loadaddr == (ElfW(Addr)) - 1 && ppnt->p_type == PT_LOAD)
			loadaddr = (ppnt->p_vaddr & ~(page_size - 1)) -
			    (ppnt->p_offset & ~(page_size - 1));
		if (ppnt->p_type == 2) {
			dynamic_addr = ppnt->p_offset;
			dynamic_size = ppnt->p_filesz;
		};
		ppnt++;
	};

	dpnt = (ElfW(Dyn) *) & header[dynamic_addr];
	dynamic_size = dynamic_size / sizeof(ElfW(Dyn));
	if ((char *)dpnt < (char *)header ||
	    (char *)(dpnt + dynamic_size) > (char *)(header + st.st_size))
		goto skip;

	if (byteswap == 1) {
		dpnt->d_tag = bswap_32(dpnt->d_tag);
		dpnt->d_un.d_val = bswap_32(dpnt->d_un.d_val);
	}

	while (dpnt->d_tag != DT_NULL) {
		if (dpnt->d_tag == DT_STRTAB)
			strtab_val = dpnt->d_un.d_val;
		dpnt++;
		if (byteswap == 1) {
			dpnt->d_tag = bswap_32(dpnt->d_tag);
			dpnt->d_un.d_val = bswap_32(dpnt->d_un.d_val);
		}
	};

	if (!strtab_val)
		goto skip;

	dpnt = (ElfW(Dyn) *) & header[dynamic_addr];
	while (dpnt->d_tag != DT_NULL) {
		if (dpnt->d_tag == DT_SONAME || dpnt->d_tag == DT_NEEDED) {
			needed_val = dpnt->d_un.d_val;
			if (needed_val + strtab_val >= loadaddr ||
			    needed_val + strtab_val < st.st_size - loadaddr) {
				needed =
				    (char *)(header - loadaddr + strtab_val +
					     needed_val);

				if (dpnt->d_tag == DT_SONAME)
					soname = xstrdup(needed);

				for (j = 0; needed_tab[j].soname != NULL; j++) {
					if (strcmp(needed, needed_tab[j].soname)
					    == 0) {
						if (*type != LIB_ELF
						    && *type !=
						    needed_tab[j].type)
							multi_libcs = 1;
						*type = needed_tab[j].type;
					}
				}
			}
		}
		dpnt++;
	};

	if (multi_libcs)
		warn("%s appears to be for multiple libc's", name);

	/* If we could not deduce the libc type, and we know what to expect, set the type */
	if (*type == LIB_ELF && expected_type != LIB_ANY)
		*type = expected_type;

	if (expected_type != LIB_ANY && expected_type != LIB_ELF &&
	    expected_type != *type) {
		warn("%s does not match type specified for directory!", name);
	}

      skip:
	munmap(header, st.st_size);

	return soname;
}
