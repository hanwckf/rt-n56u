/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Lesser General Public License version 2.1 or later.
 */

#ifndef _LDSO_H_
#define _LDSO_H_

#include <features.h>

/* Prepare for the case that `__builtin_expect' is not available.  */
#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define __builtin_expect(x, expected_value) (x)
#endif
#ifndef likely
# define likely(x)	__builtin_expect((!!(x)),1)
#endif
#ifndef unlikely
# define unlikely(x)	__builtin_expect((!!(x)),0)
#endif
#ifndef __LINUX_COMPILER_H
#define __LINUX_COMPILER_H
#endif

/* Pull in compiler and arch stuff */
#include <stdlib.h>
#include <stdarg.h>
#include <bits/wordsize.h>
/* Pull in the arch specific type information */
#include <sys/types.h>
/* Pull in the arch specific page size */
#include <bits/uClibc_page.h>
#define attribute_unused __attribute__ ((unused))
/* Pull in the ldso syscalls and string functions */
#include <dl-syscall.h>
#include <dl-string.h>
/* Now the ldso specific headers */
#include <dl-elf.h>
#include <dl-hash.h>

/* For INIT/FINI dependency sorting. */
struct init_fini_list {
	struct init_fini_list *next;
	struct elf_resolve *tpnt;
};

/* Global variables used within the shared library loader */
extern char *_dl_library_path;         /* Where we look for libraries */
extern char *_dl_preload;              /* Things to be loaded before the libs */
extern char *_dl_ldsopath;             /* Where the shared lib loader was found */
extern const char *_dl_progname;       /* The name of the executable being run */
extern int _dl_secure;                 /* Are we dealing with setuid stuff? */
extern size_t _dl_pagesize;            /* Store the page size for use later */

#ifdef __SUPPORT_LD_DEBUG__
extern char *_dl_debug;
extern char *_dl_debug_symbols;
extern char *_dl_debug_move;
extern char *_dl_debug_reloc;
extern char *_dl_debug_detail;
extern char *_dl_debug_nofixups;
extern char *_dl_debug_bindings;
extern int   _dl_debug_file;
# define __dl_debug_dprint(fmt, args...) \
	_dl_dprintf(_dl_debug_file, "%s:%i: " fmt, __FUNCTION__, __LINE__, ## args);
# define _dl_if_debug_dprint(fmt, args...) \
	do { if (_dl_debug) __dl_debug_dprint(fmt, ## args); } while (0)
#else
# define _dl_debug_dprint(fmt, args...)
# define _dl_if_debug_dprint(fmt, args...)
# define _dl_debug_file 2
#endif /* __SUPPORT_LD_DEBUG__ */

#ifdef __SUPPORT_LD_DEBUG_EARLY__
# define _dl_debug_early(fmt, args...) __dl_debug_dprint(fmt, ## args)
#else
# define _dl_debug_early(fmt, args...)
#endif /* __SUPPORT_LD_DEBUG_EARLY__ */

#ifndef NULL
#define NULL ((void *) 0)
#endif

extern void *_dl_malloc(int size);
extern char *_dl_getenv(const char *symbol, char **envp);
extern void _dl_unsetenv(const char *symbol, char **envp);
extern char *_dl_strdup(const char *string);
extern void _dl_dprintf(int, const char *, ...);

extern void _dl_get_ready_to_run(struct elf_resolve *tpnt, unsigned long load_addr,
		ElfW(auxv_t) auxvt[AT_EGID + 1], char **envp, char **argv);

#endif /* _LDSO_H_ */
