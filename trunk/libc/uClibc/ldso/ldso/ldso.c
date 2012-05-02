/* vi: set sw=4 ts=4: */
/*
 * Program to load an ELF binary on a linux system, and run it
 * after resolving ELF shared library symbols
 *
 * Copyright (C) 2005 by Joakim Tjernlund
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

#include "ldso.h"
#include "unsecvars.h"

/* Pull in common debug code */
#include "dl-debug.c"

#define ALLOW_ZERO_PLTGOT

/* Pull in the value of _dl_progname */
#include LDSO_ELFINTERP

/* Global variables used within the shared library loader */
char *_dl_library_path         = 0;	/* Where we look for libraries */
char *_dl_preload              = 0;	/* Things to be loaded before the libs */
char *_dl_ldsopath             = 0;	/* Location of the shared lib loader */
int _dl_secure                 = 1;	/* Are we dealing with setuid stuff? */
int _dl_errno                  = 0;	/* We can't use the real errno in ldso */
size_t _dl_pagesize            = 0;	/* Store the page size for use later */
struct r_debug *_dl_debug_addr = NULL;	/* Used to communicate with the gdb debugger */
void *(*_dl_malloc_function) (size_t size) = NULL;

#ifdef __SUPPORT_LD_DEBUG__
char *_dl_debug           = 0;
char *_dl_debug_symbols   = 0;
char *_dl_debug_move      = 0;
char *_dl_debug_reloc     = 0;
char *_dl_debug_detail    = 0;
char *_dl_debug_nofixups  = 0;
char *_dl_debug_bindings  = 0;
int   _dl_debug_file      = 2;
#endif

/* Needed for standalone execution. */
unsigned long attribute_hidden _dl_skip_args = 0;
const char *_dl_progname = UCLIBC_LDSO;      /* The name of the executable being run */
#include "dl-startup.c"
/* Forward function declarations */
static int _dl_suid_ok(void);

/*
 * This stub function is used by some debuggers.  The idea is that they
 * can set an internal breakpoint on it, so that we are notified when the
 * address mapping is changed in some way.
 */
void _dl_debug_state(void);
void _dl_debug_state()
{
}

static unsigned char *_dl_malloc_addr = 0;	/* Lets _dl_malloc use the already allocated memory page */
static unsigned char *_dl_mmap_zero   = 0;	/* Also used by _dl_malloc */

static struct elf_resolve **init_fini_list;
static unsigned int nlist; /* # items in init_fini_list */
extern void _start(void);

static void _dl_run_array_forward(unsigned long array, unsigned long size,
				  ElfW(Addr) loadaddr)
{
	if (array != 0) {
		unsigned int j;
		unsigned int jm;
		ElfW(Addr) *addrs;
		jm = size / sizeof (ElfW(Addr));
		addrs = (ElfW(Addr) *) (array + loadaddr);
		for (j = 0; j < jm; ++j) {
			void (*dl_elf_func) (void);
			dl_elf_func = (void (*)(void)) (intptr_t) addrs[j];
			(*dl_elf_func) ();
		}
	}
}

void _dl_run_init_array(struct elf_resolve *tpnt);
void _dl_run_init_array(struct elf_resolve *tpnt)
{
	_dl_run_array_forward(tpnt->dynamic_info[DT_INIT_ARRAY],
			      tpnt->dynamic_info[DT_INIT_ARRAYSZ],
			      tpnt->loadaddr);
}

void _dl_app_init_array(void);
void _dl_app_init_array(void)
{
	_dl_run_init_array(_dl_loaded_modules);
}

void _dl_run_fini_array(struct elf_resolve *tpnt);
void _dl_run_fini_array(struct elf_resolve *tpnt)
{
	if (tpnt->dynamic_info[DT_FINI_ARRAY]) {
		ElfW(Addr) *array = (ElfW(Addr) *) (tpnt->loadaddr + tpnt->dynamic_info[DT_FINI_ARRAY]);
		unsigned int i = (tpnt->dynamic_info[DT_FINI_ARRAYSZ] / sizeof(ElfW(Addr)));
		while (i-- > 0) {
			void (*dl_elf_func) (void);
			dl_elf_func = (void (*)(void)) (intptr_t) array[i];
			(*dl_elf_func) ();
		}
	}
}

void _dl_app_fini_array(void);
void _dl_app_fini_array(void)
{
	_dl_run_fini_array(_dl_loaded_modules);
}

static void __attribute__ ((destructor)) __attribute_used__ _dl_fini(void)
{
	int i;
	struct elf_resolve * tpnt;

	for (i = 0; i < nlist; ++i) {
		tpnt = init_fini_list[i];
		if (tpnt->init_flag & FINI_FUNCS_CALLED)
			continue;
		tpnt->init_flag |= FINI_FUNCS_CALLED;
		_dl_run_fini_array(tpnt);
		if (tpnt->dynamic_info[DT_FINI]) {
			void (*dl_elf_func) (void);

			dl_elf_func = (void (*)(void)) (intptr_t) (tpnt->loadaddr + tpnt->dynamic_info[DT_FINI]);
			_dl_if_debug_dprint("\ncalling FINI: %s\n\n", tpnt->libname);
			(*dl_elf_func) ();
		}
	}
}

void _dl_get_ready_to_run(struct elf_resolve *tpnt, unsigned long load_addr,
			  ElfW(auxv_t) auxvt[AT_EGID + 1], char **envp,
			  char **argv)
{
	ElfW(Phdr) *ppnt;
	ElfW(Dyn) *dpnt;
	char *lpntstr;
	unsigned int i;
	int unlazy = 0, trace_loaded_objects = 0;
	struct dyn_elf *rpnt;
	struct elf_resolve *tcurr;
	struct elf_resolve *tpnt1;
	struct elf_resolve app_tpnt_tmp;
	struct elf_resolve *app_tpnt = &app_tpnt_tmp;
	struct r_debug *debug_addr;
	unsigned long *lpnt;
	unsigned long *_dl_envp;		/* The environment address */
	ElfW(Addr) relro_addr = 0;
	size_t relro_size = 0;

	/* Wahoo!!! We managed to make a function call!  Get malloc
	 * setup so we can use _dl_dprintf() to print debug noise
	 * instead of the SEND_STDERR macros used in dl-startup.c */

	_dl_memset(app_tpnt, 0x00, sizeof(*app_tpnt));

	/* Store the page size for later use */
	_dl_pagesize = (auxvt[AT_PAGESZ].a_un.a_val) ? (size_t) auxvt[AT_PAGESZ].a_un.a_val : PAGE_SIZE;
	/* Make it so _dl_malloc can use the page of memory we have already
	 * allocated.  We shouldn't need to grab any more memory.  This must
	 * be first since things like _dl_dprintf() use _dl_malloc()...
	 */
	_dl_malloc_addr = (unsigned char *)_dl_pagesize;
	_dl_mmap_zero = 0;

	/* Wahoo!!! */
	_dl_debug_early("Cool, ldso survived making function calls\n");

	/* Now we have done the mandatory linking of some things.  We are now
	 * free to start using global variables, since these things have all
	 * been fixed up by now.  Still no function calls outside of this
	 * library, since the dynamic resolver is not yet ready.
	 */
	if (argv[0]) {
		_dl_progname = argv[0];
	}

	if (_start == (void *) auxvt[AT_ENTRY].a_un.a_val) {
		_dl_dprintf(_dl_debug_file, "Standalone execution is not supported yet\n");
		_dl_exit(1);
	}

	/* Start to build the tables of the modules that are required for
	 * this beast to run.  We start with the basic executable, and then
	 * go from there.  Eventually we will run across ourself, and we
	 * will need to properly deal with that as well.
	 */
	rpnt = NULL;
	if (_dl_getenv("LD_BIND_NOW", envp))
		unlazy = RTLD_NOW;

	/* Now we need to figure out what kind of options are selected.
	 * Note that for SUID programs we ignore the settings in
	 * LD_LIBRARY_PATH.
	 */
	if ((auxvt[AT_UID].a_un.a_val == (size_t)-1 && _dl_suid_ok()) ||
	    (auxvt[AT_UID].a_un.a_val != (size_t)-1 &&
	     auxvt[AT_UID].a_un.a_val == auxvt[AT_EUID].a_un.a_val &&
	     auxvt[AT_GID].a_un.a_val == auxvt[AT_EGID].a_un.a_val)) {
		_dl_secure = 0;
		_dl_preload = _dl_getenv("LD_PRELOAD", envp);
		_dl_library_path = _dl_getenv("LD_LIBRARY_PATH", envp);
	} else {
		static const char unsecure_envvars[] =
#ifdef EXTRA_UNSECURE_ENVVARS
			EXTRA_UNSECURE_ENVVARS
#endif
			UNSECURE_ENVVARS;
		const char *nextp;
		_dl_secure = 1;

		nextp = unsecure_envvars;
		do {
			_dl_unsetenv (nextp, envp);
			/* We could use rawmemchr but this need not be fast.  */
			nextp = (char *) _dl_strchr(nextp, '\0') + 1;
		} while (*nextp != '\0');
		_dl_preload = NULL;
		_dl_library_path = NULL;
		/* SUID binaries can be exploited if they do LAZY relocation. */
		unlazy = RTLD_NOW;
	}

	/* At this point we are now free to examine the user application,
	 * and figure out which libraries are supposed to be called.  Until
	 * we have this list, we will not be completely ready for dynamic
	 * linking.
	 */

	/* Find the runtime load address of the main executable.  This may be
	 * different from what the ELF header says for ET_DYN/PIE executables.
	 */
	{
		unsigned int idx;
		ElfW(Phdr) *phdr = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_val;

		for (idx = 0; idx < auxvt[AT_PHNUM].a_un.a_val; idx++, phdr++)
			if (phdr->p_type == PT_PHDR) {
				app_tpnt->loadaddr = (ElfW(Addr)) (auxvt[AT_PHDR].a_un.a_val - phdr->p_vaddr);
				break;
			}

		if (app_tpnt->loadaddr)
			_dl_debug_early("Position Independent Executable: "
					"app_tpnt->loadaddr=%x\n", app_tpnt->loadaddr);
	}

	/*
	 * This is used by gdb to locate the chain of shared libraries that are
	 * currently loaded.
	 */
	debug_addr = _dl_malloc(sizeof(struct r_debug));
	_dl_memset(debug_addr, 0, sizeof(struct r_debug));

	ppnt = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_val;
	for (i = 0; i < auxvt[AT_PHNUM].a_un.a_val; i++, ppnt++) {
		if (ppnt->p_type == PT_GNU_RELRO) {
			relro_addr = ppnt->p_vaddr;
			relro_size = ppnt->p_memsz;
		}
		if (ppnt->p_type == PT_DYNAMIC) {
			dpnt = (ElfW(Dyn) *) (ppnt->p_vaddr + app_tpnt->loadaddr);
			_dl_parse_dynamic_info(dpnt, app_tpnt->dynamic_info, debug_addr, app_tpnt->loadaddr);
#ifndef __FORCE_SHAREABLE_TEXT_SEGMENTS__
			/* Ugly, ugly.  We need to call mprotect to change the
			 * protection of the text pages so that we can do the
			 * dynamic linking.  We can set the protection back
			 * again once we are done.
			 */
			_dl_debug_early("calling mprotect on the application program\n");
			/* Now cover the application program. */
			if (app_tpnt->dynamic_info[DT_TEXTREL]) {
				ppnt = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_val;
				for (i = 0; i < auxvt[AT_PHNUM].a_un.a_val; i++, ppnt++) {
					if (ppnt->p_type == PT_LOAD && !(ppnt->p_flags & PF_W))
						_dl_mprotect((void *) ((ppnt->p_vaddr + app_tpnt->loadaddr) & PAGE_ALIGN),
							     ((ppnt->p_vaddr + app_tpnt->loadaddr) & ADDR_ALIGN) +
							     (unsigned long) ppnt->p_filesz,
							     PROT_READ | PROT_WRITE | PROT_EXEC);
				}
			}
#endif

#ifndef ALLOW_ZERO_PLTGOT
			/* make sure it's really there. */
			if (app_tpnt->dynamic_info[DT_PLTGOT] == 0)
				continue;
#endif
			/* OK, we have what we need - slip this one into the list. */
			app_tpnt = _dl_add_elf_hash_table(_dl_progname, (char *)app_tpnt->loadaddr,
					app_tpnt->dynamic_info, ppnt->p_vaddr + app_tpnt->loadaddr, ppnt->p_filesz);
			_dl_loaded_modules->libtype = elf_executable;
			_dl_loaded_modules->ppnt = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_val;
			_dl_loaded_modules->n_phent = auxvt[AT_PHNUM].a_un.a_val;
			_dl_symbol_tables = rpnt = (struct dyn_elf *) _dl_malloc(sizeof(struct dyn_elf));
			_dl_memset(rpnt, 0, sizeof(struct dyn_elf));
			rpnt->dyn = _dl_loaded_modules;
			app_tpnt->rtld_flags = unlazy | RTLD_GLOBAL;
			app_tpnt->usage_count++;
			app_tpnt->symbol_scope = _dl_symbol_tables;
			lpnt = (unsigned long *) (app_tpnt->dynamic_info[DT_PLTGOT]);
#ifdef ALLOW_ZERO_PLTGOT
			if (lpnt)
#endif
				INIT_GOT(lpnt, _dl_loaded_modules);
		}

		/* OK, fill this in - we did not have this before */
		if (ppnt->p_type == PT_INTERP) {
			char *ptmp;

			tpnt->libname = (char *) ppnt->p_vaddr + app_tpnt->loadaddr;

			/* Store the path where the shared lib loader was found
			 * for later use
			 */
			_dl_ldsopath = _dl_strdup(tpnt->libname);
			ptmp = _dl_strrchr(_dl_ldsopath, '/');
			if (ptmp != _dl_ldsopath)
				*ptmp = '\0';

			_dl_debug_early("Lib Loader: (%x) %s\n", tpnt->loadaddr, tpnt->libname);
		}
	}
	app_tpnt->relro_addr = relro_addr;
	app_tpnt->relro_size = relro_size;

#ifdef __SUPPORT_LD_DEBUG__
	_dl_debug = _dl_getenv("LD_DEBUG", envp);
	if (_dl_debug) {
		if (_dl_strstr(_dl_debug, "all")) {
			_dl_debug_detail = _dl_debug_move = _dl_debug_symbols
				= _dl_debug_reloc = _dl_debug_bindings = _dl_debug_nofixups = (void*)1;
		} else {
			_dl_debug_detail   = _dl_strstr(_dl_debug, "detail");
			_dl_debug_move     = _dl_strstr(_dl_debug, "move");
			_dl_debug_symbols  = _dl_strstr(_dl_debug, "sym");
			_dl_debug_reloc    = _dl_strstr(_dl_debug, "reloc");
			_dl_debug_nofixups = _dl_strstr(_dl_debug, "nofix");
			_dl_debug_bindings = _dl_strstr(_dl_debug, "bind");
		}
	}

	{
		const char *dl_debug_output;

		dl_debug_output = _dl_getenv("LD_DEBUG_OUTPUT", envp);

		if (dl_debug_output) {
			char tmp[22], *tmp1, *filename;
			int len1, len2;

			_dl_memset(tmp, 0, sizeof(tmp));
			tmp1 = _dl_simple_ltoa( tmp, (unsigned long)_dl_getpid());

			len1 = _dl_strlen(dl_debug_output);
			len2 = _dl_strlen(tmp1);

			filename = _dl_malloc(len1+len2+2);

			if (filename) {
				_dl_strcpy (filename, dl_debug_output);
				filename[len1] = '.';
				_dl_strcpy (&filename[len1+1], tmp1);

				_dl_debug_file= _dl_open(filename, O_WRONLY|O_CREAT, 0644);
				if (_dl_debug_file < 0) {
					_dl_debug_file = 2;
					_dl_dprintf(_dl_debug_file, "can't open file: '%s'\n",filename);
				}
			}
		}
	}
#endif

	if (_dl_getenv("LD_TRACE_LOADED_OBJECTS", envp) != NULL) {
		trace_loaded_objects++;
	}

#ifndef __LDSO_LDD_SUPPORT__
	if (trace_loaded_objects) {
		_dl_dprintf(_dl_debug_file, "Use the ldd provided by uClibc\n");
		_dl_exit(1);
	}
#endif

	/*
	 * OK, fix one more thing - set up debug_addr so it will point
	 * to our chain.  Later we may need to fill in more fields, but this
	 * should be enough for now.
	 */
	debug_addr->r_map = (struct link_map *) _dl_loaded_modules;
	debug_addr->r_version = 1;
	debug_addr->r_ldbase = load_addr;
	debug_addr->r_brk = (unsigned long) &_dl_debug_state;
	_dl_debug_addr = debug_addr;

	/* Notify the debugger we are in a consistant state */
	_dl_debug_addr->r_state = RT_CONSISTENT;
	_dl_debug_state();

	/* OK, we now have the application in the list, and we have some
	 * basic stuff in place.  Now search through the list for other shared
	 * libraries that should be loaded, and insert them on the list in the
	 * correct order.
	 */

	_dl_map_cache();

	if (_dl_preload) {
		char c, *str, *str2;

		str = _dl_preload;
		while (*str == ':' || *str == ' ' || *str == '\t')
			str++;

		while (*str) {
			str2 = str;
			while (*str2 && *str2 != ':' && *str2 != ' ' && *str2 != '\t')
				str2++;
			c = *str2;
			*str2 = '\0';

			if (!_dl_secure || _dl_strchr(str, '/') == NULL) {
				_dl_if_debug_dprint("\tfile='%s';  needed by '%s'\n", str, _dl_progname);

				tpnt1 = _dl_load_shared_library(_dl_secure, &rpnt, NULL, str, trace_loaded_objects);
				if (!tpnt1) {
#ifdef __LDSO_LDD_SUPPORT__
					if (trace_loaded_objects)
						_dl_dprintf(1, "\t%s => not found\n", str);
					else
#endif
					{
						_dl_dprintf(_dl_debug_file, "%s: can't load " "library '%s'\n", _dl_progname, str);
						_dl_exit(15);
					}
				} else {
					tpnt1->rtld_flags = unlazy | RTLD_GLOBAL;

					_dl_debug_early("Loading: (%x) %s\n", tpnt1->loadaddr, tpnt1->libname);

#ifdef __LDSO_LDD_SUPPORT__
					if (trace_loaded_objects &&
					    tpnt1->usage_count == 1) {
						/* This is a real hack to make
						 * ldd not print the library
						 * itself when run on a
						 * library.
						 */
						if (_dl_strcmp(_dl_progname, str) != 0)
							_dl_dprintf(1, "\t%s => %s (%x)\n", str, tpnt1->libname,
								    tpnt1->loadaddr);
					}
#endif
				}
			}

			*str2 = c;
			str = str2;
			while (*str == ':' || *str == ' ' || *str == '\t')
				str++;
		}
	}

#ifdef __LDSO_PRELOAD_FILE_SUPPORT__
	do {
		struct stat st;
		char *preload;
		int fd;
		char c, *cp, *cp2;

		if (_dl_stat(LDSO_PRELOAD, &st) || st.st_size == 0) {
			break;
		}

		if ((fd = _dl_open(LDSO_PRELOAD, O_RDONLY, 0)) < 0) {
			_dl_dprintf(_dl_debug_file, "%s: can't open file '%s'\n",
				    _dl_progname, LDSO_PRELOAD);
			break;
		}

		preload = (caddr_t) _dl_mmap(0, st.st_size + 1,
					     PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
		_dl_close(fd);
		if (preload == (caddr_t) -1) {
			_dl_dprintf(_dl_debug_file, "%s:%i: can't map '%s'\n",
				    _dl_progname, __LINE__, LDSO_PRELOAD);
			break;
		}

		/* convert all separators and comments to spaces */
		for (cp = preload; *cp; /*nada */ ) {
			if (*cp == ':' || *cp == '\t' || *cp == '\n') {
				*cp++ = ' ';
			} else if (*cp == '#') {
				do {
					*cp++ = ' ';
				} while (*cp != '\n' && *cp != '\0');
			} else {
				cp++;
			}
		}

		/* find start of first library */
		for (cp = preload; *cp && *cp == ' '; cp++)
			/*nada */ ;

		while (*cp) {
			/* find end of library */
			for (cp2 = cp; *cp && *cp != ' '; cp++)
				/*nada */ ;
			c = *cp;
			*cp = '\0';

			_dl_if_debug_dprint("\tfile='%s';  needed by '%s'\n", cp2, _dl_progname);

			tpnt1 = _dl_load_shared_library(0, &rpnt, NULL, cp2, trace_loaded_objects);
			if (!tpnt1) {
#ifdef __LDSO_LDD_SUPPORT__
				if (trace_loaded_objects)
					_dl_dprintf(1, "\t%s => not found\n", cp2);
				else
#endif
				{
					_dl_dprintf(_dl_debug_file, "%s: can't load library '%s'\n", _dl_progname, cp2);
					_dl_exit(15);
				}
			} else {
				tpnt1->rtld_flags = unlazy | RTLD_GLOBAL;

				_dl_debug_early("Loading: (%x) %s\n", tpnt1->loadaddr, tpnt1->libname);

#ifdef __LDSO_LDD_SUPPORT__
				if (trace_loaded_objects &&
				    tpnt1->usage_count == 1) {
					_dl_dprintf(1, "\t%s => %s (%x)\n",
						    cp2, tpnt1->libname,
						    (unsigned)tpnt1->loadaddr);
				}
#endif
			}

			/* find start of next library */
			*cp = c;
			for ( /*nada */ ; *cp && *cp == ' '; cp++)
				/*nada */ ;
		}

		_dl_munmap(preload, st.st_size + 1);
	} while (0);
#endif /* __LDSO_PRELOAD_FILE_SUPPORT__ */

	nlist = 0;
	for (tcurr = _dl_loaded_modules; tcurr; tcurr = tcurr->next) {
		ElfW(Dyn) *this_dpnt;

		nlist++;
		for (this_dpnt = (ElfW(Dyn) *) tcurr->dynamic_addr; this_dpnt->d_tag; this_dpnt++) {
			if (this_dpnt->d_tag == DT_NEEDED) {
				char *name;
				struct init_fini_list *tmp;

				lpntstr = (char*) (tcurr->dynamic_info[DT_STRTAB] + this_dpnt->d_un.d_val);
				name = _dl_get_last_path_component(lpntstr);
				if (_dl_strcmp(name, UCLIBC_LDSO) == 0)
					continue;

				_dl_if_debug_dprint("\tfile='%s';  needed by '%s'\n", lpntstr, _dl_progname);

				if (!(tpnt1 = _dl_load_shared_library(0, &rpnt, tcurr, lpntstr, trace_loaded_objects)))	{
#ifdef __LDSO_LDD_SUPPORT__
					if (trace_loaded_objects) {
						_dl_dprintf(1, "\t%s => not found\n", lpntstr);
						continue;
					} else
#endif
					{
						_dl_dprintf(_dl_debug_file, "%s: can't load library '%s'\n", _dl_progname, lpntstr);
						_dl_exit(16);
					}
				}

				tmp = alloca(sizeof(struct init_fini_list)); /* Allocates on stack, no need to free this memory */
				tmp->tpnt = tpnt1;
				tmp->next = tcurr->init_fini;
				tcurr->init_fini = tmp;

				tpnt1->rtld_flags = unlazy | RTLD_GLOBAL;

				_dl_debug_early("Loading: (%x) %s\n", tpnt1->loadaddr, tpnt1->libname);

#ifdef __LDSO_LDD_SUPPORT__
				if (trace_loaded_objects &&
				    tpnt1->usage_count == 1) {
					_dl_dprintf(1, "\t%s => %s (%x)\n",
						    lpntstr, tpnt1->libname,
						    (unsigned)tpnt1->loadaddr);
				}
#endif
			}
		}
	}
	_dl_unmap_cache();

	--nlist; /* Exclude the application. */
	init_fini_list = _dl_malloc(nlist * sizeof(struct elf_resolve *));
	i = 0;
	for (tcurr = _dl_loaded_modules->next; tcurr; tcurr = tcurr->next) {
		init_fini_list[i++] = tcurr;
	}

	/* Sort the INIT/FINI list in dependency order. */
	for (tcurr = _dl_loaded_modules->next; tcurr; tcurr = tcurr->next) {
		int j, k;

		for (j = 0; init_fini_list[j] != tcurr; ++j)
			/* Empty */;
		for (k = j + 1; k < nlist; ++k) {
			struct init_fini_list *runp = init_fini_list[k]->init_fini;

			for (; runp; runp = runp->next) {
				if (runp->tpnt == tcurr) {
					struct elf_resolve *here = init_fini_list[k];
					_dl_if_debug_dprint("Move %s from pos %d to %d in INIT/FINI list\n", here->libname, k, j);
					for (i = (k - j); i; --i)
						init_fini_list[i+j] = init_fini_list[i+j-1];
					init_fini_list[j] = here;
					++j;
					break;
				}
			}
		}
	}
#ifdef __SUPPORT_LD_DEBUG__
	if(_dl_debug) {
		_dl_dprintf(_dl_debug_file, "\nINIT/FINI order and dependencies:\n");
		for (i = 0; i < nlist; i++) {
			struct init_fini_list *tmp;

			_dl_dprintf(_dl_debug_file, "lib: %s has deps:\n",
				    init_fini_list[i]->libname);
			tmp = init_fini_list[i]->init_fini;
			for (; tmp; tmp = tmp->next)
				_dl_dprintf(_dl_debug_file, " %s ", tmp->tpnt->libname);
			_dl_dprintf(_dl_debug_file, "\n");
		}
	}
#endif

	/*
	 * If the program interpreter is not in the module chain, add it.
	 * This will be required for dlopen to be able to access the internal
	 * functions in the dynamic linker and to relocate the interpreter
	 * again once all libs are loaded.
	 */
	if (tpnt) {
		ElfW(Ehdr) *epnt = (ElfW(Ehdr) *) auxvt[AT_BASE].a_un.a_val;
		ElfW(Phdr) *myppnt = (ElfW(Phdr) *) (load_addr + epnt->e_phoff);
		int j;
		
		tpnt = _dl_add_elf_hash_table(tpnt->libname, (char *)load_addr,
					      tpnt->dynamic_info,
					      (unsigned long)tpnt->dynamic_addr,
					      0);

		tpnt->n_phent = epnt->e_phnum;
		tpnt->ppnt = myppnt;
		for (j = 0; j < epnt->e_phnum; j++, myppnt++) {
			if (myppnt->p_type ==  PT_GNU_RELRO) {
				tpnt->relro_addr = myppnt->p_vaddr;
				tpnt->relro_size = myppnt->p_memsz;
				break;
			}
		}
		tpnt->libtype = program_interpreter;
		tpnt->usage_count++;
		tpnt->symbol_scope = _dl_symbol_tables;
		if (rpnt) {
			rpnt->next = (struct dyn_elf *) _dl_malloc(sizeof(struct dyn_elf));
			_dl_memset(rpnt->next, 0, sizeof(struct dyn_elf));
			rpnt->next->prev = rpnt;
			rpnt = rpnt->next;
		} else {
			rpnt = (struct dyn_elf *) _dl_malloc(sizeof(struct dyn_elf));
			_dl_memset(rpnt, 0, sizeof(struct dyn_elf));
		}
		rpnt->dyn = tpnt;
		tpnt->rtld_flags = RTLD_NOW | RTLD_GLOBAL; /* Must not be LAZY */
#ifdef RERELOCATE_LDSO
		/* Only rerelocate functions for now. */
		tpnt->init_flag = RELOCS_DONE;
		lpnt = (unsigned long *) (tpnt->dynamic_info[DT_PLTGOT]);
# ifdef ALLOW_ZERO_PLTGOT
		if (tpnt->dynamic_info[DT_PLTGOT])
# endif
			INIT_GOT(lpnt, tpnt);
#else
		tpnt->init_flag = RELOCS_DONE | JMP_RELOCS_DONE;
#endif
		tpnt = NULL;
	}

#ifdef __LDSO_LDD_SUPPORT__
	/* End of the line for ldd.... */
	if (trace_loaded_objects) {
		_dl_dprintf(1, "\t%s => %s (%x)\n",
			    rpnt->dyn->libname + _dl_strlen(_dl_ldsopath) + 1,
			    rpnt->dyn->libname, rpnt->dyn->loadaddr);
		_dl_exit(0);
	}
#endif

	_dl_debug_early("Beginning relocation fixups\n");

#ifdef __mips__
	/*
	 * Relocation of the GOT entries for MIPS have to be done
	 * after all the libraries have been loaded.
	 */
	_dl_perform_mips_global_got_relocations(_dl_loaded_modules, !unlazy);
#endif

	/*
	 * OK, now all of the kids are tucked into bed in their proper
	 * addresses.  Now we go through and look for REL and RELA records that
	 * indicate fixups to the GOT tables.  We need to do this in reverse
	 * order so that COPY directives work correctly.
	 */
	if (_dl_symbol_tables)
		if (_dl_fixup(_dl_symbol_tables, unlazy))
			_dl_exit(-1);

	for (tpnt = _dl_loaded_modules; tpnt; tpnt = tpnt->next) {
		if (tpnt->relro_size)
			_dl_protect_relro (tpnt);
	}

	/* OK, at this point things are pretty much ready to run.  Now we need
	 * to touch up a few items that are required, and then we can let the
	 * user application have at it.  Note that the dynamic linker itself
	 * is not guaranteed to be fully dynamicly linked if we are using
	 * ld.so.1, so we have to look up each symbol individually.
	 */

	_dl_envp = (unsigned long *) (intptr_t) _dl_find_hash("__environ", _dl_symbol_tables, NULL, 0);
	if (_dl_envp)
		*_dl_envp = (unsigned long) envp;

#ifndef __FORCE_SHAREABLE_TEXT_SEGMENTS__
	{
		unsigned int j;
		ElfW(Phdr) *myppnt;

		/* We had to set the protections of all pages to R/W for
		 * dynamic linking.  Set text pages back to R/O.
		 */
		for (tpnt = _dl_loaded_modules; tpnt; tpnt = tpnt->next) {
			for (myppnt = tpnt->ppnt, j = 0; j < tpnt->n_phent; j++, myppnt++) {
				if (myppnt->p_type == PT_LOAD && !(myppnt->p_flags & PF_W) && tpnt->dynamic_info[DT_TEXTREL]) {
					_dl_mprotect((void *) (tpnt->loadaddr + (myppnt->p_vaddr & PAGE_ALIGN)),
							(myppnt->p_vaddr & ADDR_ALIGN) + (unsigned long) myppnt->p_filesz, LXFLAGS(myppnt->p_flags));
				}
			}
		}

	}
#endif
	/* Notify the debugger we have added some objects. */
	_dl_debug_addr->r_state = RT_ADD;
	_dl_debug_state();

	/* Run pre-initialization functions for the executable.  */
	_dl_run_array_forward(_dl_loaded_modules->dynamic_info[DT_PREINIT_ARRAY],
			      _dl_loaded_modules->dynamic_info[DT_PREINIT_ARRAYSZ],
			      _dl_loaded_modules->loadaddr);

	/* Run initialization functions for loaded objects.  For the
	   main executable, they will be run from __uClibc_main.  */
	for (i = nlist; i; --i) {
		tpnt = init_fini_list[i-1];
		tpnt->init_fini = NULL; /* Clear, since alloca was used */
		if (tpnt->init_flag & INIT_FUNCS_CALLED)
			continue;
		tpnt->init_flag |= INIT_FUNCS_CALLED;

		if (tpnt->dynamic_info[DT_INIT]) {
			void (*dl_elf_func) (void);

			dl_elf_func = (void (*)(void)) (intptr_t) (tpnt->loadaddr + tpnt->dynamic_info[DT_INIT]);

			_dl_if_debug_dprint("calling INIT: %s\n\n", tpnt->libname);

			(*dl_elf_func) ();
		}
		_dl_run_init_array(tpnt);
	}

	/* Find the real malloc function and make ldso functions use that from now on */
	 _dl_malloc_function = (void* (*)(size_t)) (intptr_t) _dl_find_hash("malloc",
			 _dl_symbol_tables, NULL, ELF_RTYPE_CLASS_PLT);

	/* Notify the debugger that all objects are now mapped in.  */
	_dl_debug_addr->r_state = RT_CONSISTENT;
	_dl_debug_state();
}

char *_dl_getenv(const char *symbol, char **envp)
{
	char *pnt;
	const char *pnt1;

	while ((pnt = *envp++)) {
		pnt1 = symbol;
		while (*pnt && *pnt == *pnt1)
			pnt1++, pnt++;
		if (!*pnt || *pnt != '=' || *pnt1)
			continue;
		return pnt + 1;
	}
	return 0;
}

void _dl_unsetenv(const char *symbol, char **envp)
{
	char *pnt;
	const char *pnt1;
	char **newenvp = envp;

	for (pnt = *envp; pnt; pnt = *++envp) {
		pnt1 = symbol;
		while (*pnt && *pnt == *pnt1)
			pnt1++, pnt++;
		if (!*pnt || *pnt != '=' || *pnt1)
			*newenvp++ = *envp;
	}
	*newenvp++ = *envp;
	return;
}

static int _dl_suid_ok(void)
{
	__kernel_uid_t uid, euid;
	__kernel_gid_t gid, egid;

	uid = _dl_getuid();
	euid = _dl_geteuid();
	gid = _dl_getgid();
	egid = _dl_getegid();

	if(uid == euid && gid == egid) {
		return 1;
	}
	return 0;
}

void *_dl_malloc(int size)
{
	void *retval;

#if 0
	_dl_debug_early("request for %d bytes\n", size);
#endif

	if (_dl_malloc_function)
		return (*_dl_malloc_function) (size);

	if (_dl_malloc_addr - _dl_mmap_zero + (unsigned)size > _dl_pagesize) {
		_dl_debug_early("mmapping more memory\n");
		_dl_mmap_zero = _dl_malloc_addr = _dl_mmap((void *) 0, size,
				PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (_dl_mmap_check_error(_dl_mmap_zero)) {
			_dl_dprintf(_dl_debug_file, "%s: mmap of a spare page failed!\n", _dl_progname);
			_dl_exit(20);
		}
	}
	retval = _dl_malloc_addr;
	_dl_malloc_addr += size;

	/*
	 * Align memory to 4 byte boundary.  Some platforms require this,
	 * others simply get better performance.
	 */
	_dl_malloc_addr = (unsigned char *) (((unsigned long) _dl_malloc_addr + 3) & ~(3));
	return retval;
}

#include "dl-hash.c"
#include "dl-elf.c"
