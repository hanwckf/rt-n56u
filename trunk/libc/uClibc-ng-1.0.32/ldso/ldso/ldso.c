/*
 * Program to load an ELF binary on a linux system, and run it
 * after resolving ELF shared library symbols
 *
 * Copyright (C) 2005 by Joakim Tjernlund
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
#include "unsecvars.h"

/* Pull in common debug code */
#include "dl-debug.c"

#define ALLOW_ZERO_PLTGOT

#if defined(USE_TLS) && USE_TLS
#include "dl-tls.c"
#endif

/* Pull in the value of _dl_progname */
#include LDSO_ELFINTERP

/* Global variables used within the shared library loader */
#ifdef __LDSO_LD_LIBRARY_PATH__
char *_dl_library_path         = NULL;	/* Where we look for libraries */
#endif
#ifdef __LDSO_PRELOAD_ENV_SUPPORT__
char *_dl_preload              = NULL;	/* Things to be loaded before the libs */
#endif
int _dl_errno                  = 0;	/* We can't use the real errno in ldso */
size_t _dl_pagesize            = 0;	/* Store the page size for use later */
struct r_debug *_dl_debug_addr = NULL;	/* Used to communicate with the gdb debugger */
void *(*_dl_malloc_function) (size_t size) = NULL;
void (*_dl_free_function) (void *p) = NULL;

#ifdef __LDSO_PRELINK_SUPPORT__
char *_dl_trace_prelink                      = NULL;	/* Library for prelinking trace */
struct elf_resolve *_dl_trace_prelink_map    = NULL;	/* Library module for prelinking trace */
bool _dl_verbose				= true;					/* On by default */
bool prelinked					= false;
#endif
int _dl_secure = 1; /* Are we dealing with setuid stuff? */

#ifdef __SUPPORT_LD_DEBUG__
char *_dl_debug           = NULL;
char *_dl_debug_symbols   = NULL;
char *_dl_debug_move      = NULL;
char *_dl_debug_reloc     = NULL;
char *_dl_debug_detail    = NULL;
char *_dl_debug_nofixups  = NULL;
char *_dl_debug_bindings  = NULL;
int   _dl_debug_file      = 2;
#endif

#ifdef __DSBT__
void **_dl_ldso_dsbt = NULL;
#endif

unsigned long attribute_hidden _dl_skip_args = 0;

const char *_dl_progname = UCLIBC_LDSO;      /* The name of the executable being run */
#include "dl-startup.c"
#include "dl-symbols.c"
#include "dl-array.c"

/*
 * This stub function is used by some debuggers.  The idea is that they
 * can set an internal breakpoint on it, so that we are notified when the
 * address mapping is changed in some way.
 */
void _dl_debug_state(void);
rtld_hidden_proto(_dl_debug_state, noinline);
void _dl_debug_state(void)
{
	/* Make sure GCC doesn't recognize this function as pure, to avoid
	 * having the calls optimized away.
	 */
	__asm__("");
}
rtld_hidden_def(_dl_debug_state);

static unsigned char *_dl_malloc_addr = NULL;	/* Lets _dl_malloc use the already allocated memory page */
static unsigned char *_dl_mmap_zero   = NULL;	/* Also used by _dl_malloc */

static struct elf_resolve **init_fini_list;
static struct elf_resolve **scope_elem_list;
static unsigned int nlist; /* # items in init_fini_list */
#ifdef __FDPIC__
/* We need to take the address of _start instead of its FUNCDESC:
   declare it as void* to control the relocation emitted.  */
extern void *_start;
#else
extern void _start(void);
#endif

#ifdef __UCLIBC_HAS_SSP__
# include <dl-osinfo.h>
static uintptr_t stack_chk_guard;
# ifndef THREAD_SET_STACK_GUARD
/* Only exported for architectures that don't store the stack guard canary
 * in local thread area.  */
uintptr_t __stack_chk_guard attribute_relro;
# endif
#endif

#ifdef __LDSO_SEARCH_INTERP_PATH__
const char *_dl_ldsopath = NULL;	/* Location of the shared lib loader */

static void _dl_ldsopath_init(struct elf_resolve *tpnt)
{
	char *ldsopath, *ptmp;

	/*
	 * Store the path where the shared lib loader was found for later use.
	 * Note that this logic isn't bullet proof when it comes to relative
	 * paths: if you use "./lib/ldso.so", and then the app does chdir()
	 * followed by dlopen(), the old ldso path won't get searched.  But
	 * that is a fairly pathological use case, so if you don't like that,
	 * then set a full path to your interp and be done :P.
	 */
	ldsopath = _dl_strdup(tpnt->libname);
	ptmp = _dl_strrchr(ldsopath, '/');
	/*
	 * If there is no "/", then set the path to "", and the code
	 * later on will take this to implicitly mean "search $PWD".
	 */
	if (!ptmp)
		ptmp = ldsopath;
	*ptmp = '\0';

	_dl_ldsopath = ldsopath;
	_dl_debug_early("Lib Loader: (%x) %s: using path: %s\n",
		(unsigned) DL_LOADADDR_BASE(tpnt->loadaddr), tpnt->libname,
		_dl_ldsopath);
}
#else
#define _dl_ldsopath_init(tpnt)
#endif

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

	if (uid == euid && gid == egid) {
		return 1;
	}
	return 0;
}

void *_dl_malloc(size_t size)
{
	void *retval;

#if 0
	_dl_debug_early("request for %d bytes\n", size);
#endif

	if (_dl_malloc_function)
		return (*_dl_malloc_function) (size);

	if (_dl_malloc_addr - _dl_mmap_zero + size > _dl_pagesize) {
		size_t rounded_size;

		/* Since the above assumes we get a full page even if
		   we request less than that, make sure we request a
		   full page, since uClinux may give us less than than
		   a full page.  We might round even
		   larger-than-a-page sizes, but we end up never
		   reusing _dl_mmap_zero/_dl_malloc_addr in that case,
		   so we don't do it.

		   The actual page size doesn't really matter; as long
		   as we're self-consistent here, we're safe.  */
		if (size < _dl_pagesize)
			rounded_size = (size + ADDR_ALIGN) & _dl_pagesize;
		else
			rounded_size = size;

		_dl_debug_early("mmapping more memory\n");
		_dl_mmap_zero = _dl_malloc_addr = _dl_mmap((void *) 0, rounded_size,
				PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | _MAP_UNINITIALIZED, -1, 0);
		if (_dl_mmap_check_error(_dl_mmap_zero)) {
			_dl_dprintf(2, "%s: mmap of a spare page failed!\n", _dl_progname);
			_dl_exit(20);
		}
	}
	retval = _dl_malloc_addr;
	_dl_malloc_addr += size;

	/*
	 * Align memory to DL_MALLOC_ALIGN byte boundary.  Some
	 * platforms require this, others simply get better
	 * performance.
	 */
	_dl_malloc_addr = (unsigned char *) (((unsigned long) _dl_malloc_addr + DL_MALLOC_ALIGN - 1) & ~(DL_MALLOC_ALIGN - 1));
	return retval;
}

static void *_dl_zalloc(size_t size)
{
	void *p = _dl_malloc(size);
	if (p)
		_dl_memset(p, 0, size);
	else
		_dl_exit(1);
	return p;
}

void _dl_free(void *p)
{
	if (_dl_free_function)
		(*_dl_free_function) (p);
}

#if defined(USE_TLS) && USE_TLS
void *_dl_memalign(size_t __boundary, size_t __size)
{
	void *result;
	int i = 0;
	size_t delta;
	size_t rounded = 0;

	if (_dl_memalign_function)
		return (*_dl_memalign_function) (__boundary, __size);

	while (rounded < __boundary) {
		rounded = (1 << i++);
	}

	delta = (((size_t) _dl_malloc_addr + __size) & (rounded - 1));

	if ((result = _dl_malloc(rounded - delta)) == NULL)
		return result;

	result = _dl_malloc(__size);

	return result;
}
#endif

static void __attribute__ ((destructor)) __attribute_used__ _dl_fini(void)
{
	unsigned int i;
	struct elf_resolve * tpnt;

	for (i = 0; i < nlist; ++i) {
		tpnt = init_fini_list[i];
		if (tpnt->init_flag & FINI_FUNCS_CALLED)
			continue;
		tpnt->init_flag |= FINI_FUNCS_CALLED;
		_dl_run_fini_array(tpnt);
		if (tpnt->dynamic_info[DT_FINI]) {
			void (*dl_elf_func) (void);

			dl_elf_func = (void (*)(void)) (intptr_t) DL_RELOC_ADDR(tpnt->loadaddr, tpnt->dynamic_info[DT_FINI]);
			_dl_if_debug_dprint("\ncalling FINI: %s\n\n", tpnt->libname);
			DL_CALL_FUNC_AT_ADDR (dl_elf_func, tpnt->loadaddr, (void(*)(void)));
		}
	}
}

#ifdef __LDSO_PRELINK_SUPPORT__
static void trace_objects(struct elf_resolve *tpnt, char *str_name)
{
	if (_dl_strcmp(_dl_trace_prelink, tpnt->libname) == 0)
		_dl_trace_prelink_map = tpnt;
	if (tpnt->libtype == elf_executable) {
/* Main executeble */
		_dl_dprintf(1, "\t%s => %s (%x, %x)", tpnt->libname, tpnt->libname,
					tpnt->mapaddr, DL_LOADADDR_BASE(tpnt->loadaddr));
	} else {
/* Preloaded, Needed or interpreter */
		_dl_dprintf(1, "\t%s => %s (%x, %x)", str_name, tpnt->libname,
					tpnt->mapaddr, DL_LOADADDR_BASE(tpnt->loadaddr));
	}

#if defined USE_TLS && USE_TLS
	if ((tpnt->libtype != program_interpreter) && (tpnt->l_tls_modid))
		_dl_dprintf (1, " TLS(%x, %x)\n", tpnt->l_tls_modid,
					(size_t) tpnt->l_tls_offset);
	else
#endif
		_dl_dprintf (1, "\n");
}
#endif

static struct elf_resolve * add_ldso(struct elf_resolve *tpnt,
									 DL_LOADADDR_TYPE load_addr,
									 ElfW(Addr) ldso_mapaddr,
									 ElfW(auxv_t) auxvt[AT_EGID + 1],
									 struct dyn_elf *rpnt)
{
		ElfW(Ehdr) *epnt = (ElfW(Ehdr) *) auxvt[AT_BASE].a_un.a_val;
		ElfW(Phdr) *myppnt = (ElfW(Phdr) *)
				DL_RELOC_ADDR(DL_GET_RUN_ADDR(load_addr, ldso_mapaddr),
							  epnt->e_phoff);
		int j;
		struct stat st;

		tpnt = _dl_add_elf_hash_table(tpnt->libname, tpnt->loadaddr,
					      tpnt->dynamic_info, (unsigned long)tpnt->dynamic_addr,
					      0);

		tpnt->mapaddr = ldso_mapaddr;
		if (_dl_stat(tpnt->libname, &st) >= 0) {
			tpnt->st_dev = st.st_dev;
			tpnt->st_ino = st.st_ino;
		}
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
		if (rpnt) {
			rpnt->next = _dl_zalloc(sizeof(struct dyn_elf));
			rpnt->next->prev = rpnt;
			rpnt = rpnt->next;
		} else {
			rpnt = _dl_zalloc(sizeof(struct dyn_elf));
		}
		rpnt->dyn = tpnt;
		tpnt->rtld_flags = RTLD_NOW | RTLD_GLOBAL; /* Must not be LAZY */

	return tpnt;
}

static ptrdiff_t _dl_build_local_scope (struct elf_resolve **list,
										struct elf_resolve *map)
{
	struct elf_resolve **p = list;
	struct init_fini_list *q;

	*p++ = map;
	map->init_flag |= DL_RESERVED;
	if (map->init_fini)
		for (q = map->init_fini; q; q = q->next)
			if (! (q->tpnt->init_flag & DL_RESERVED))
				p += _dl_build_local_scope (p, q->tpnt);
	return p - list;
}

static void _dl_setup_progname(const char *argv0)
{
	char image[PATH_MAX];
	ssize_t s;

	s = _dl_readlink(AT_FDCWD, "/proc/self/exe", image, sizeof(image));
	if (s > 0 && image[0] == '/') {
		image[s] = 0;
		_dl_progname = _dl_strdup(image);
	} else if (argv0) {
		_dl_progname = argv0;
	}
}

void *_dl_get_ready_to_run(struct elf_resolve *tpnt, DL_LOADADDR_TYPE load_addr,
			  ElfW(auxv_t) auxvt[AT_EGID + 1], char **envp, char **argv
			  DL_GET_READY_TO_RUN_EXTRA_PARMS)
{
	ElfW(Addr) app_mapaddr = 0, ldso_mapaddr = 0;
	ElfW(Phdr) *ppnt;
	ElfW(Dyn) *dpnt;
	char *lpntstr;
	unsigned int i, cnt, nscope_elem;
	int unlazy = 0, trace_loaded_objects = 0;
	struct dyn_elf *rpnt;
	struct elf_resolve *tcurr;
	struct elf_resolve *tpnt1;
	struct elf_resolve *ldso_tpnt = NULL;
	struct elf_resolve app_tpnt_tmp;
	struct elf_resolve *app_tpnt = &app_tpnt_tmp;
	struct r_debug *debug_addr;
	unsigned long *lpnt;
	unsigned long *_dl_envp;		/* The environment address */
	ElfW(Addr) relro_addr = 0;
	size_t relro_size = 0;
	struct r_scope_elem *global_scope;
	struct elf_resolve **local_scope;
#if defined(__FDPIC__)
	int rtype_class = ELF_RTYPE_CLASS_DLSYM;
#else
	int rtype_class = ELF_RTYPE_CLASS_PLT;
#endif

#if defined(USE_TLS) && USE_TLS
	void *tcbp = NULL;
#endif

	/* Wahoo!!! We managed to make a function call!  Get malloc
	 * setup so we can use _dl_dprintf() to print debug noise
	 * instead of the SEND_STDERR macros used in dl-startup.c */

	_dl_memset(app_tpnt, 0, sizeof(*app_tpnt));

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
	_dl_setup_progname(argv[0]);

#ifdef __DSBT__
	_dl_ldso_dsbt = (void *)tpnt->dynamic_info[DT_DSBT_BASE_IDX];
	_dl_ldso_dsbt[tpnt->dynamic_info[DT_DSBT_INDEX_IDX]] = _dl_ldso_dsbt;
#endif

#ifndef __LDSO_STANDALONE_SUPPORT__
	if (_start == (void *) auxvt[AT_ENTRY].a_un.a_val) {
		_dl_dprintf(2, "Standalone execution is not enabled\n");
		_dl_exit(1);
	}
#endif

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
#ifdef __LDSO_PRELOAD_ENV_SUPPORT__
		_dl_preload = _dl_getenv("LD_PRELOAD", envp);
#endif
#ifdef __LDSO_LD_LIBRARY_PATH__
		_dl_library_path = _dl_getenv("LD_LIBRARY_PATH", envp);
#endif
	} else {
		static const char unsecure_envvars[] =
#ifdef EXTRA_UNSECURE_ENVVARS
			EXTRA_UNSECURE_ENVVARS
#endif
			UNSECURE_ENVVARS;
		const char *nextp;
		_dl_secure = 1;

#ifdef __LDSO_PRELOAD_ENV_SUPPORT__
		_dl_preload = _dl_getenv("LD_PRELOAD", envp);
#endif
		nextp = unsecure_envvars;
		do {
			_dl_unsetenv (nextp, envp);
			/* We could use rawmemchr but this need not be fast.  */
			nextp = _dl_strchr(nextp, '\0') + 1;
		} while (*nextp != '\0');
#ifdef __LDSO_LD_LIBRARY_PATH__
		_dl_library_path = NULL;
#endif
		/* SUID binaries can be exploited if they do LAZY relocation. */
		unlazy = RTLD_NOW;
	}

#if defined(USE_TLS) && USE_TLS
	_dl_error_catch_tsd = &_dl_initial_error_catch_tsd;
	_dl_init_static_tls = &_dl_nothread_init_static_tls;
#endif

#ifdef __LDSO_STANDALONE_SUPPORT__
	if (_start == (void *) auxvt[AT_ENTRY].a_un.a_val) {
		ElfW(Addr) *aux_dat = (ElfW(Addr) *) argv;
		int argc = (int) aux_dat[-1];

		tpnt->libname = argv[0];
		while (argc > 1)
			if (! _dl_strcmp (argv[1], "--library-path") && argc > 2) {
#ifdef __LDSO_LD_LIBRARY_PATH__
				_dl_library_path = argv[2];
#endif
				_dl_skip_args += 2;
				argc -= 2;
				argv += 2;
			} else
				break;

	/*
	 * If we have no further argument the program was called incorrectly.
	 * Grant the user some education.
	 */

		if (argc < 2) {
			_dl_dprintf(1, "\
Usage: ld.so [OPTION]... EXECUTABLE-FILE [ARGS-FOR-PROGRAM...]\n\
You have invoked `ld.so', the helper program for shared library executables.\n\
This program usually lives in the file `/lib/ld.so', and special directives\n\
in executable files using ELF shared libraries tell the system's program\n\
loader to load the helper program from this file.  This helper program loads\n\
the shared libraries needed by the program executable, prepares the program\n\
to run, and runs it.  You may invoke this helper program directly from the\n\
command line to load and run an ELF executable file; this is like executing\n\
that file itself, but always uses this helper program from the file you\n\
specified, instead of the helper program file specified in the executable\n\
file you run.  This is mostly of use for maintainers to test new versions\n\
of this helper program; chances are you did not intend to run this program.\n\
\n\
  --library-path PATH   use given PATH instead of content of the environment\n\
                        variable LD_LIBRARY_PATH\n");
			_dl_exit(1);
		}

		++_dl_skip_args;
		++argv;
		_dl_progname = argv[0];

		_dl_symbol_tables = rpnt = _dl_zalloc(sizeof(struct dyn_elf));
		/*
		 * It needs to load the _dl_progname and to map it
		 * Usually it is the main application launched by means of the ld.so
		 * but it could be also a shared object (when ld.so used for tracing)
		 * We keep the misleading app_tpnt name to avoid variable pollution
		 */
		app_tpnt = _dl_load_elf_shared_library(_dl_secure ? __RTLD_SECURE : 0,
							&rpnt, _dl_progname);
		if (!app_tpnt) {
			_dl_dprintf(2, "can't load '%s'\n", _dl_progname);
			_dl_exit(16);
		}
		/*
		 * FIXME: it needs to properly handle a PIE executable
		 * Usually for a main application, loadaddr is computed as difference
		 * between auxvt entry points and phdr, so if it is not 0, that it is a
		 * PIE executable. In this case instead we need to set the loadaddr to 0
		 * because we are actually mapping the ELF for the main application by
		 * ourselves. So the PIE case must be checked.
		 */

		app_tpnt->rtld_flags = unlazy | RTLD_GLOBAL;

		/*
		 * This is used by gdb to locate the chain of shared libraries that are
		 * currently loaded.
		 */
		debug_addr = _dl_zalloc(sizeof(struct r_debug));
		ppnt = (ElfW(Phdr) *)app_tpnt->ppnt;
		for (i = 0; i < app_tpnt->n_phent; i++, ppnt++) {
			if (ppnt->p_type == PT_DYNAMIC) {
				dpnt = (ElfW(Dyn) *) DL_RELOC_ADDR(app_tpnt->loadaddr, ppnt->p_vaddr);
				_dl_parse_dynamic_info(dpnt, app_tpnt->dynamic_info, debug_addr, app_tpnt->loadaddr);
			}
		}

		_dl_ldsopath_init(tpnt);
	} else {
#endif

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
				DL_INIT_LOADADDR_PROG(app_tpnt->loadaddr, auxvt[AT_PHDR].a_un.a_val - phdr->p_vaddr);
				break;
			}

		if (DL_LOADADDR_BASE(app_tpnt->loadaddr))
			_dl_debug_early("Position Independent Executable: "
					"app_tpnt->loadaddr=%x\n", DL_LOADADDR_BASE(app_tpnt->loadaddr));
	}

	/*
	 * This is used by gdb to locate the chain of shared libraries that are
	 * currently loaded.
	 */
	debug_addr = _dl_zalloc(sizeof(struct r_debug));

	ppnt = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_val;
	for (i = 0; i < auxvt[AT_PHNUM].a_un.a_val; i++, ppnt++) {
		if (ppnt->p_type == PT_GNU_RELRO) {
			relro_addr = ppnt->p_vaddr;
			relro_size = ppnt->p_memsz;
		}
		if (!app_mapaddr && (ppnt->p_type == PT_LOAD)) {
			app_mapaddr = DL_RELOC_ADDR (app_tpnt->loadaddr, ppnt->p_vaddr);
		}
		if (ppnt->p_type == PT_DYNAMIC) {
			dpnt = (ElfW(Dyn) *) DL_RELOC_ADDR(app_tpnt->loadaddr, ppnt->p_vaddr);
			_dl_parse_dynamic_info(dpnt, app_tpnt->dynamic_info, debug_addr, app_tpnt->loadaddr);
#ifndef __FORCE_SHAREABLE_TEXT_SEGMENTS__
			/* Ugly, ugly.  We need to call mprotect to change the
			 * protection of the text pages so that we can do the
			 * dynamic linking.  We can set the protection back
			 * again once we are done.
			 */
			/* Now cover the application program. */
			if (app_tpnt->dynamic_info[DT_TEXTREL]) {
				int j;
				ElfW(Phdr) *ppnt_outer = ppnt;
				_dl_debug_early("calling mprotect on the application program\n");
				ppnt = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_val;
				for (j = 0; j < auxvt[AT_PHNUM].a_un.a_val; j++, ppnt++) {
					if (ppnt->p_type == PT_LOAD && !(ppnt->p_flags & PF_W))
						_dl_mprotect((void *) (DL_RELOC_ADDR(app_tpnt->loadaddr, ppnt->p_vaddr) & PAGE_ALIGN),
							     (DL_RELOC_ADDR(app_tpnt->loadaddr, ppnt->p_vaddr) & ADDR_ALIGN) +
							     (unsigned long) ppnt->p_filesz,
							     PROT_READ | PROT_WRITE | PROT_EXEC);
				}
				ppnt = ppnt_outer;
			}
#else
			if (app_tpnt->dynamic_info[DT_TEXTREL]) {
				_dl_dprintf(2, "Can't modify application's text section; use the GCC option -fPIE for position-independent executables.\n");
				_dl_exit(1);
			}
#endif

#ifndef ALLOW_ZERO_PLTGOT
			/* make sure it's really there. */
			if (app_tpnt->dynamic_info[DT_PLTGOT] == 0)
				continue;
#endif
			/* OK, we have what we need - slip this one into the list. */
			app_tpnt = _dl_add_elf_hash_table(_dl_progname, app_tpnt->loadaddr,
					app_tpnt->dynamic_info,
					(unsigned long) DL_RELOC_ADDR(app_tpnt->loadaddr, ppnt->p_vaddr),
					ppnt->p_filesz);
			_dl_loaded_modules->libtype = elf_executable;
			_dl_loaded_modules->ppnt = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_val;
			_dl_loaded_modules->n_phent = auxvt[AT_PHNUM].a_un.a_val;
			_dl_symbol_tables = rpnt = _dl_zalloc(sizeof(struct dyn_elf));
			rpnt->dyn = _dl_loaded_modules;
			app_tpnt->mapaddr = app_mapaddr;
			app_tpnt->rtld_flags = unlazy | RTLD_GLOBAL;
			app_tpnt->usage_count++;
#ifdef __DSBT__
			_dl_ldso_dsbt[0] = app_tpnt->dsbt_table;
			_dl_memcpy(app_tpnt->dsbt_table, _dl_ldso_dsbt,
				   app_tpnt->dsbt_size * sizeof(tpnt->dsbt_table[0]));
#endif
			lpnt = (unsigned long *) (app_tpnt->dynamic_info[DT_PLTGOT]);
#ifdef ALLOW_ZERO_PLTGOT
			if (lpnt)
#endif
				INIT_GOT(lpnt, _dl_loaded_modules);
		}

		/* OK, fill this in - we did not have this before */
		if (ppnt->p_type == PT_INTERP) {
			tpnt->libname = (char *) DL_RELOC_ADDR(app_tpnt->loadaddr, ppnt->p_vaddr);

			_dl_ldsopath_init(tpnt);
		}

		/* Discover any TLS sections if the target supports them. */
		if (ppnt->p_type == PT_TLS) {
#if defined(USE_TLS) && USE_TLS
			if (ppnt->p_memsz > 0) {
				app_tpnt->l_tls_blocksize = ppnt->p_memsz;
				app_tpnt->l_tls_align = ppnt->p_align;
				if (ppnt->p_align == 0)
					app_tpnt->l_tls_firstbyte_offset = 0;
				else
					app_tpnt->l_tls_firstbyte_offset =
						(ppnt->p_vaddr & (ppnt->p_align - 1));
				app_tpnt->l_tls_initimage_size = ppnt->p_filesz;
				app_tpnt->l_tls_initimage = (void *) ppnt->p_vaddr;

				/* This image gets the ID one.  */
				_dl_tls_max_dtv_idx = app_tpnt->l_tls_modid = 1;

			}
			_dl_debug_early("Found TLS header for application program\n");
			break;
#else
			_dl_dprintf(2, "Program uses unsupported TLS data!\n");
			_dl_exit(1);
#endif
		}
	}
	app_tpnt->relro_addr = relro_addr;
	app_tpnt->relro_size = relro_size;

#if defined(USE_TLS) && USE_TLS
	/*
	 * Adjust the address of the TLS initialization image in
	 * case the executable is actually an ET_DYN object.
	 */
	if (app_tpnt->l_tls_initimage != NULL) {
		char *tmp attribute_unused =
			(char *) app_tpnt->l_tls_initimage;
		app_tpnt->l_tls_initimage =
			(char *) DL_RELOC_ADDR(app_tpnt->loadaddr, app_tpnt->l_tls_initimage);
		_dl_debug_early("Relocated TLS initial image from %x to %x (size = %x)\n",
			tmp, app_tpnt->l_tls_initimage, app_tpnt->l_tls_initimage_size);
	}
#endif

#ifdef __LDSO_STANDALONE_SUPPORT__
	} /* ! ldso standalone mode */
#endif

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

			filename = _dl_malloc(len1 + len2 + 2);

			if (filename) {
				_dl_strcpy (filename, dl_debug_output);
				filename[len1] = '.';
				_dl_strcpy (&filename[len1+1], tmp1);

				_dl_debug_file = _dl_open(filename, O_WRONLY|O_CREAT, 0644);
				if (_dl_debug_file < 0) {
					_dl_debug_file = 2;
					_dl_dprintf(_dl_debug_file, "can't open file: '%s'\n",filename);
				}
			}
		}
	}
#endif

#ifdef __LDSO_PRELINK_SUPPORT__
{
	char *ld_warn = _dl_getenv ("LD_WARN", envp);

	if (ld_warn && *ld_warn == '\0')
		_dl_verbose = false;
}
	_dl_trace_prelink = _dl_getenv("LD_TRACE_PRELINKING", envp);
#endif

	if (_dl_getenv("LD_TRACE_LOADED_OBJECTS", envp) != NULL) {
		trace_loaded_objects++;
	}

#ifndef __LDSO_LDD_SUPPORT__
	if (trace_loaded_objects) {
		_dl_dprintf(2, "Use the ldd provided by uClibc\n");
		_dl_exit(1);
	}
#endif

	ldso_mapaddr = (ElfW(Addr)) auxvt[AT_BASE].a_un.a_val;
	/*
	 * OK, fix one more thing - set up debug_addr so it will point
	 * to our chain.  Later we may need to fill in more fields, but this
	 * should be enough for now.
	 */
	debug_addr->r_map = (struct link_map *) _dl_loaded_modules;
	debug_addr->r_version = 1;
	debug_addr->r_ldbase = (ElfW(Addr))
		DL_LOADADDR_BASE(DL_GET_RUN_ADDR(load_addr, ldso_mapaddr));
	debug_addr->r_brk = (unsigned long) &_dl_debug_state;
	_dl_debug_addr = debug_addr;

	/* Do not notify the debugger until the interpreter is in the list */

	/* OK, we now have the application in the list, and we have some
	 * basic stuff in place.  Now search through the list for other shared
	 * libraries that should be loaded, and insert them on the list in the
	 * correct order.
	 */

	_dl_map_cache();

#ifdef __LDSO_PRELOAD_ENV_SUPPORT__
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

				tpnt1 = _dl_load_shared_library(
					_dl_secure ? __RTLD_SECURE : 0,
					&rpnt, NULL, str, trace_loaded_objects);
				if (!tpnt1) {
#ifdef __LDSO_LDD_SUPPORT__
					if (trace_loaded_objects || _dl_trace_prelink)
						_dl_dprintf(1, "\t%s => not found\n", str);
					else
#endif
					{
						_dl_dprintf(2, "%s: library '%s' "
							"from LD_PRELOAD can't be preloaded: ignored.\n",
							_dl_progname, str);
					}
				} else {
					tpnt1->rtld_flags = unlazy | RTLD_GLOBAL;

					_dl_debug_early("Loading: (%x) %s\n", DL_LOADADDR_BASE(tpnt1->loadaddr), tpnt1->libname);

#ifdef __LDSO_LDD_SUPPORT__
					if (trace_loaded_objects && !_dl_trace_prelink &&
					    !(tpnt1->init_flag & DL_OPENED2)) {
						/* This is a real hack to make
						 * ldd not print the library
						 * itself when run on a
						 * library.
						 */
						if (_dl_strcmp(_dl_progname, str) != 0)
							_dl_dprintf(1, "\t%s => %s (%x)\n", str, tpnt1->libname,
								    DL_LOADADDR_BASE(tpnt1->loadaddr));
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
#endif /* __LDSO_PRELOAD_ENV_SUPPORT__ */

#ifdef __LDSO_PRELOAD_FILE_SUPPORT__
	do {
		char *preload;
		int fd;
		char c, *cp, *cp2;
		struct stat st;

		if (_dl_stat(LDSO_PRELOAD, &st) || st.st_size == 0) {
			break;
		}

		if ((fd = _dl_open(LDSO_PRELOAD, O_RDONLY, 0)) < 0) {
			_dl_dprintf(2, "%s: can't open file '%s'\n",
				    _dl_progname, LDSO_PRELOAD);
			break;
		}

		preload = (caddr_t) _dl_mmap(0, st.st_size + 1,
					     PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
		_dl_close(fd);
		if (preload == (caddr_t) -1) {
			_dl_dprintf(2, "%s:%i: can't map '%s'\n",
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
# ifdef __LDSO_LDD_SUPPORT__
				if (trace_loaded_objects || _dl_trace_prelink)
					_dl_dprintf(1, "\t%s => not found\n", cp2);
				else
# endif
				{
					_dl_dprintf(2, "%s: can't load library '%s'\n", _dl_progname, cp2);
					_dl_exit(15);
				}
			} else {
				tpnt1->rtld_flags = unlazy | RTLD_GLOBAL;

				_dl_debug_early("Loading: (%x) %s\n", DL_LOADADDR_BASE(tpnt1->loadaddr), tpnt1->libname);

# ifdef __LDSO_LDD_SUPPORT__
				if (trace_loaded_objects && !_dl_trace_prelink &&
				    !(tpnt1->init_flag & DL_OPENED2)) {
					_dl_dprintf(1, "\t%s => %s (%x)\n",
						    cp2, tpnt1->libname,
						    DL_LOADADDR_BASE(tpnt1->loadaddr));
				}
# endif
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
				_dl_if_debug_dprint("\tfile='%s';  needed by '%s'\n", lpntstr, _dl_progname);

				if (_dl_strcmp(name, UCLIBC_LDSO) == 0) {
						if (!ldso_tpnt) {
							/* Insert the ld.so only once */
							ldso_tpnt = add_ldso(tpnt, load_addr,
												 ldso_mapaddr, auxvt, rpnt);
						} else {
							ldso_tpnt->init_flag |= DL_OPENED2;
						}
						ldso_tpnt->usage_count++;
						tpnt1 = ldso_tpnt;
				} else
					tpnt1 = _dl_load_shared_library(0, &rpnt, tcurr, lpntstr, trace_loaded_objects);

				if (!tpnt1) {
#ifdef __LDSO_LDD_SUPPORT__
					if (trace_loaded_objects || _dl_trace_prelink) {
						_dl_dprintf(1, "\t%s => not found\n", lpntstr);
						continue;
					} else
#endif
					{
						_dl_dprintf(2, "%s: can't load library '%s'\n", _dl_progname, lpntstr);
						_dl_exit(16);
					}
				}

				tmp = alloca(sizeof(struct init_fini_list)); /* Allocates on stack, no need to free this memory */
				tmp->tpnt = tpnt1;
				tmp->next = tcurr->init_fini;
				tcurr->init_fini = tmp;

				tpnt1->rtld_flags = unlazy | RTLD_GLOBAL;

				_dl_debug_early("Loading: (%x) %s\n", DL_LOADADDR_BASE(tpnt1->loadaddr), tpnt1->libname);

#ifdef __LDSO_LDD_SUPPORT__
				if (trace_loaded_objects && !_dl_trace_prelink &&
				    !(tpnt1->init_flag & DL_OPENED2)) {
					_dl_dprintf(1, "\t%s => %s (%x)\n",
						    lpntstr, tpnt1->libname,
						    DL_LOADADDR_BASE(tpnt1->loadaddr));
				}
#endif
			}
		}
	}
	_dl_unmap_cache();

	/* Keep track of the number of elements in the global scope */
	nscope_elem = nlist;

	if (_dl_loaded_modules->libtype == elf_executable) {
		--nlist; /* Exclude the application. */
		tcurr = _dl_loaded_modules->next;
	} else
		tcurr = _dl_loaded_modules;
	init_fini_list = _dl_malloc(nlist * sizeof(struct elf_resolve *));
	i = 0;
	for (; tcurr; tcurr = tcurr->next)
		init_fini_list[i++] = tcurr;

	/* Sort the INIT/FINI list in dependency order. */
	for (tcurr = _dl_loaded_modules->next; tcurr; tcurr = tcurr->next) {
		unsigned int j, k;

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
	if (_dl_debug) {
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
	if (!ldso_tpnt) {
		tpnt = add_ldso(tpnt, load_addr, ldso_mapaddr, auxvt, rpnt);
		tpnt->usage_count++;
		nscope_elem++;
	} else
		tpnt = ldso_tpnt;

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

	/*
	 * Allocate the global scope array.
	 */
	scope_elem_list = (struct elf_resolve **) _dl_malloc(nscope_elem * sizeof(struct elf_resolve *));

	for (i = 0, tcurr = _dl_loaded_modules; tcurr; tcurr = tcurr->next)
		scope_elem_list[i++] = tcurr;

	_dl_loaded_modules->symbol_scope.r_list = scope_elem_list;
	_dl_loaded_modules->symbol_scope.r_nlist = nscope_elem;
	/*
	 * The symbol scope of the application, that is the first entry of the
	 * _dl_loaded_modules list, is just the global scope to be used for the
	 * symbol lookup.
	 */
	global_scope = &_dl_loaded_modules->symbol_scope;

	/* Build the local scope for each loaded modules. */
	local_scope = _dl_malloc(nscope_elem * sizeof(struct elf_resolve *));
	i = 1;
	for (tcurr = _dl_loaded_modules->next; tcurr; tcurr = tcurr->next) {
		unsigned int k;
		cnt = _dl_build_local_scope(local_scope, scope_elem_list[i++]);
		tcurr->symbol_scope.r_list = _dl_malloc(cnt * sizeof(struct elf_resolve *));
		tcurr->symbol_scope.r_nlist = cnt;
		_dl_memcpy (tcurr->symbol_scope.r_list, local_scope, cnt * sizeof (struct elf_resolve *));
		/* Restoring the init_flag.*/
		for (k = 1; k < nscope_elem; k++)
			scope_elem_list[k]->init_flag &= ~DL_RESERVED;
	}

	_dl_free(local_scope);

#ifdef __LDSO_LDD_SUPPORT__
	/* Exit if LD_TRACE_LOADED_OBJECTS is on. */
	if (trace_loaded_objects && !_dl_trace_prelink)
		_dl_exit(0);
#endif

#if defined(USE_TLS) && USE_TLS
	/* We do not initialize any of the TLS functionality unless any of the
	 * initial modules uses TLS.  This makes dynamic loading of modules with
	 * TLS impossible, but to support it requires either eagerly doing setup
	 * now or lazily doing it later.  Doing it now makes us incompatible with
	 * an old kernel that can't perform TLS_INIT_TP, even if no TLS is ever
	 * used.  Trying to do it lazily is too hairy to try when there could be
	 * multiple threads (from a non-TLS-using libpthread).  */
	bool was_tls_init_tp_called = tls_init_tp_called;
	if (tcbp == NULL) {
		_dl_debug_early("Calling init_tls()!\n");
		tcbp = init_tls ();
	}
#endif
#ifdef __UCLIBC_HAS_SSP__
	_dl_debug_early("Setting up SSP guards\n");
	/* Set up the stack checker's canary.  */
	stack_chk_guard = _dl_setup_stack_chk_guard ();
# ifdef THREAD_SET_STACK_GUARD
	THREAD_SET_STACK_GUARD (stack_chk_guard);
# else
	__stack_chk_guard = stack_chk_guard;
# endif
#endif

#ifdef __LDSO_PRELINK_SUPPORT__
	if (_dl_trace_prelink) {

		unsigned int nscope_trace = ldso_tpnt ? nscope_elem : (nscope_elem - 1);

		for (i = 0; i < nscope_trace; i++)
			trace_objects(scope_elem_list[i],
				_dl_get_last_path_component(scope_elem_list[i]->libname));

		if (_dl_verbose)
			/* Warn about undefined symbols. */
			if (_dl_symbol_tables)
				if (_dl_fixup(_dl_symbol_tables, global_scope, unlazy))
					_dl_exit(-1);
		_dl_exit(0);
	}

	if (_dl_loaded_modules->dynamic_info[DT_GNU_LIBLIST_IDX]) {
		ElfW(Lib) *liblist, *liblistend;
		struct elf_resolve **r_list, **r_listend, *l;
		const char *strtab = (const char *)_dl_loaded_modules->dynamic_info[DT_STRTAB];

		_dl_assert (_dl_loaded_modules->dynamic_info[DT_GNU_LIBLISTSZ_IDX] != 0);
		liblist = (ElfW(Lib) *) _dl_loaded_modules->dynamic_info[DT_GNU_LIBLIST_IDX];
		liblistend = (ElfW(Lib) *)
		((char *) liblist + _dl_loaded_modules->dynamic_info[DT_GNU_LIBLISTSZ_IDX]);
		r_list = _dl_loaded_modules->symbol_scope.r_list;
		r_listend = r_list + nscope_elem;

		for (; r_list < r_listend && liblist < liblistend; r_list++) {
			l = *r_list;

			if (l == _dl_loaded_modules)
				continue;

			/* If the library is not mapped where it should, fail.  */
			if (l->loadaddr)
				break;

			/* Next, check if checksum matches.  */
			if (l->dynamic_info[DT_CHECKSUM_IDX] == 0 ||
				l->dynamic_info[DT_CHECKSUM_IDX] != liblist->l_checksum)
				break;

			if (l->dynamic_info[DT_GNU_PRELINKED_IDX] == 0 ||
				(l->dynamic_info[DT_GNU_PRELINKED_IDX] != liblist->l_time_stamp))
				break;

			if (_dl_strcmp(strtab + liblist->l_name, _dl_get_last_path_component(l->libname)) != 0)
				break;

			++liblist;
		}


		if (r_list == r_listend && liblist == liblistend)
			prelinked = true;

	}

	_dl_debug_early ("prelink checking: %s\n", prelinked ? "ok" : "failed");

	if (prelinked) {
		if (_dl_loaded_modules->dynamic_info[DT_GNU_CONFLICT_IDX]) {
			ELF_RELOC *conflict;
			unsigned long conflict_size;

			_dl_assert (_dl_loaded_modules->dynamic_info[DT_GNU_CONFLICTSZ_IDX] != 0);
			conflict = (ELF_RELOC *) _dl_loaded_modules->dynamic_info[DT_GNU_CONFLICT_IDX];
			conflict_size = _dl_loaded_modules->dynamic_info[DT_GNU_CONFLICTSZ_IDX];
			_dl_parse_relocation_information(_dl_symbol_tables, global_scope,
				(unsigned long) conflict, conflict_size);
		}

		/* Mark all the objects so we know they have been already relocated.  */
		for (tpnt = _dl_loaded_modules; tpnt; tpnt = tpnt->next) {
			tpnt->init_flag |= RELOCS_DONE;
			if (tpnt->relro_size)
				_dl_protect_relro (tpnt);
		}
	} else
#endif

	{

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
		if (_dl_fixup(_dl_symbol_tables, global_scope, unlazy))
			_dl_exit(-1);

	for (tpnt = _dl_loaded_modules; tpnt; tpnt = tpnt->next) {
		if (tpnt->relro_size)
			_dl_protect_relro (tpnt);
	}
	} /* not prelinked */

#if defined(USE_TLS) && USE_TLS
	if (!was_tls_init_tp_called && _dl_tls_max_dtv_idx > 0)
		++_dl_tls_generation;

	_dl_debug_early("Calling _dl_allocate_tls_init()!\n");

	/* Now that we have completed relocation, the initializer data
	   for the TLS blocks has its final values and we can copy them
	   into the main thread's TLS area, which we allocated above.  */
	_dl_allocate_tls_init (tcbp);

	/* And finally install it for the main thread.  If ld.so itself uses
	   TLS we know the thread pointer was initialized earlier.  */
	if (! tls_init_tp_called) {
		const char *lossage = (char *) TLS_INIT_TP (tcbp, USE___THREAD);
		if (__builtin_expect (lossage != NULL, 0)) {
			_dl_debug_early("cannot set up thread-local storage: %s\n", lossage);
			_dl_exit(30);
		}
	}
#endif /* USE_TLS */

	/* OK, at this point things are pretty much ready to run.  Now we need
	 * to touch up a few items that are required, and then we can let the
	 * user application have at it.  Note that the dynamic linker itself
	 * is not guaranteed to be fully dynamicly linked if we are using
	 * ld.so.1, so we have to look up each symbol individually.
	 */

	_dl_envp = (unsigned long *) (intptr_t) _dl_find_hash(__C_SYMBOL_PREFIX__ "__environ", global_scope, NULL, 0, NULL);
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
					_dl_mprotect((void *) (DL_RELOC_ADDR(tpnt->loadaddr, myppnt->p_vaddr) & PAGE_ALIGN),
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

			dl_elf_func = (void (*)(void)) DL_RELOC_ADDR(tpnt->loadaddr, tpnt->dynamic_info[DT_INIT]);

			_dl_if_debug_dprint("calling INIT: %s\n\n", tpnt->libname);

			DL_CALL_FUNC_AT_ADDR (dl_elf_func, tpnt->loadaddr, (void(*)(void)));
		}

		_dl_run_init_array(tpnt);
	}

	/* Find the real malloc function and make ldso functions use that from now on */
	_dl_malloc_function = (void* (*)(size_t)) (intptr_t) _dl_find_hash(__C_SYMBOL_PREFIX__ "malloc",
			global_scope, NULL, rtype_class, NULL);

#if defined(USE_TLS) && USE_TLS
	/* Find the real functions and make ldso functions use them from now on */
	_dl_calloc_function = (void* (*)(size_t, size_t)) (intptr_t)
		_dl_find_hash(__C_SYMBOL_PREFIX__ "calloc", global_scope, NULL, rtype_class, NULL);

	_dl_realloc_function = (void* (*)(void *, size_t)) (intptr_t)
		_dl_find_hash(__C_SYMBOL_PREFIX__ "realloc", global_scope, NULL, rtype_class, NULL);

	_dl_free_function = (void (*)(void *)) (intptr_t)
		_dl_find_hash(__C_SYMBOL_PREFIX__ "free", global_scope, NULL, rtype_class, NULL);

	_dl_memalign_function = (void* (*)(size_t, size_t)) (intptr_t)
		_dl_find_hash(__C_SYMBOL_PREFIX__ "memalign", global_scope, NULL, rtype_class, NULL);

#endif

	/* Notify the debugger that all objects are now mapped in.  */
	_dl_debug_addr->r_state = RT_CONSISTENT;
	_dl_debug_state();

#ifdef __LDSO_STANDALONE_SUPPORT__
	if (_start == (void *) auxvt[AT_ENTRY].a_un.a_val)
		return (void *) app_tpnt->l_entry;
	else
#endif
		return (void *) auxvt[AT_ENTRY].a_un.a_val;
}

#include "dl-hash.c"
#include "dl-elf.c"
