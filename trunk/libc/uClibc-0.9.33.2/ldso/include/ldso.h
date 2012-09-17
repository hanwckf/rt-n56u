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
#if defined __GNUC__ && __GNUC__ == 2 && __GNUC_MINOR__ < 96
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
#include <stddef.h> /* for ptrdiff_t */
#include <stdbool.h>
#define _FCNTL_H
#include <bits/fcntl.h>
#include <bits/wordsize.h>
/* Pull in the arch specific type information */
#include <sys/types.h>
/* Pull in the arch specific page size */
#include <bits/uClibc_page.h>
/* Pull in the MIN macro */
#include <sys/param.h>
/* Pull in the ldso syscalls and string functions */
#ifndef __ARCH_HAS_NO_SHARED__
#include <dl-syscall.h>
#include <dl-string.h>
#include <dlfcn.h>
/* Now the ldso specific headers */
#include <dl-elf.h>
#ifdef __UCLIBC_HAS_TLS__
/* Defines USE_TLS */
#include <tls.h>
#endif
#include <dl-hash.h>

/* common align masks, if not specified by sysdep headers */
#ifndef ADDR_ALIGN
#define ADDR_ALIGN (_dl_pagesize - 1)
#endif

#ifndef PAGE_ALIGN
#define PAGE_ALIGN (~ADDR_ALIGN)
#endif

#ifndef OFFS_ALIGN
#define OFFS_ALIGN (PAGE_ALIGN & ~(1ul << (sizeof(_dl_pagesize) * 8 - 1)))
#endif

/* For INIT/FINI dependency sorting. */
struct init_fini_list {
	struct init_fini_list *next;
	struct elf_resolve *tpnt;
};

/* Global variables used within the shared library loader */
extern char *_dl_library_path;         /* Where we look for libraries */
extern char *_dl_preload;              /* Things to be loaded before the libs */
#ifdef __LDSO_SEARCH_INTERP_PATH__
extern const char *_dl_ldsopath;       /* Where the shared lib loader was found */
#endif
extern const char *_dl_progname;       /* The name of the executable being run */
extern size_t _dl_pagesize;            /* Store the page size for use later */
#ifdef __LDSO_PRELINK_SUPPORT__
extern char *_dl_trace_prelink;        /* Library for prelinking trace */
extern struct elf_resolve *_dl_trace_prelink_map;	/* Library map for prelinking trace */
#else
#define _dl_trace_prelink		0
#endif

#if defined(USE_TLS) && USE_TLS
extern void _dl_add_to_slotinfo (struct link_map  *l);
extern void ** __attribute__ ((const)) _dl_initial_error_catch_tsd (void);
#endif

#ifdef USE_TLS
void _dl_add_to_slotinfo (struct link_map  *l);
void ** __attribute__ ((const)) _dl_initial_error_catch_tsd (void);
#endif
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
	_dl_dprintf(_dl_debug_file, "%s:%i: " fmt, __func__, __LINE__, ## args);
# define _dl_if_debug_dprint(fmt, args...) \
	do { if (_dl_debug) __dl_debug_dprint(fmt, ## args); } while (0)
#else
# define __dl_debug_dprint(fmt, args...) do {} while (0)
# define _dl_if_debug_dprint(fmt, args...) do {} while (0)
# define _dl_debug_file 2
#endif /* __SUPPORT_LD_DEBUG__ */

#ifdef IS_IN_rtld
# ifdef __SUPPORT_LD_DEBUG__
#  define _dl_assert(expr)						\
	do {								\
		if (!(expr)) {						\
			__dl_debug_dprint("assert(%s)\n", #expr);	\
			_dl_exit(45);					\
		}							\
	} while (0)
# else
#  define _dl_assert(expr) ((void)0)
# endif
#else
# include <assert.h>
# define _dl_assert(expr) assert(expr)
#endif

#ifdef __SUPPORT_LD_DEBUG_EARLY__
# define _dl_debug_early(fmt, args...) __dl_debug_dprint(fmt, ## args)
#else
# define _dl_debug_early(fmt, args...) do {} while (0)
#endif /* __SUPPORT_LD_DEBUG_EARLY__ */

#ifndef NULL
#define NULL ((void *) 0)
#endif

extern void *_dl_malloc(size_t size);
extern void *_dl_calloc(size_t __nmemb, size_t __size);
extern void *_dl_realloc(void *__ptr, size_t __size);
extern void _dl_free(void *);
extern char *_dl_getenv(const char *symbol, char **envp);
extern void _dl_unsetenv(const char *symbol, char **envp);
extern char *_dl_strdup(const char *string);
extern void _dl_dprintf(int, const char *, ...);

#ifndef DL_GET_READY_TO_RUN_EXTRA_PARMS
# define DL_GET_READY_TO_RUN_EXTRA_PARMS
#endif
#ifndef DL_GET_READY_TO_RUN_EXTRA_ARGS
# define DL_GET_READY_TO_RUN_EXTRA_ARGS
#endif

extern void *_dl_get_ready_to_run(struct elf_resolve *tpnt, DL_LOADADDR_TYPE load_addr,
		ElfW(auxv_t) auxvt[AT_EGID + 1], char **envp, char **argv
		DL_GET_READY_TO_RUN_EXTRA_PARMS);

#ifdef HAVE_DL_INLINES_H
#include <dl-inlines.h>
#endif

#else /* __ARCH_HAS_NO_SHARED__ */
#include <dl-defs.h>
#endif

#endif /* _LDSO_H_ */
