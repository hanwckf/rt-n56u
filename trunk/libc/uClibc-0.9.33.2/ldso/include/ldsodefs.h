#ifndef _LDSODEFS_H
#define _LDSODEFS_H     1

#include <bits/kernel-features.h>

#include <features.h>
#include <tls.h>

#ifdef __mips__
/* The MIPS ABI specifies that the dynamic section has to be read-only.  */

#define DL_RO_DYN_SECTION 1

/* TODO: Import in 64-bit relocations from glibc. */
#endif

#ifndef SHARED
# define EXTERN extern
#else
# ifdef IS_IN_rtld
#  define EXTERN
# else
#  define EXTERN extern
# endif
#endif

/* Non-shared code has no support for multiple namespaces.  */
#ifdef SHARED
# define DL_NNS 16
#else
# define DL_NNS 1
#endif

#define GL(x) _##x
#define GLRO(x) _##x

/* Variable pointing to the end of the stack (or close to it).  This value
   must be constant over the runtime of the application.  Some programs
   might use the variable which results in copy relocations on some
   platforms.  But this does not matter, ld.so can always use the local
   copy.  */
extern void *__libc_stack_end;

/* Determine next available module ID.  */
extern size_t _dl_next_tls_modid (void) internal_function attribute_hidden;

/* Calculate offset of the TLS blocks in the static TLS block.  */
extern void _dl_determine_tlsoffset (void) internal_function attribute_hidden;

/* Set up the data structures for TLS, when they were not set up at startup.
   Returns nonzero on malloc failure.
   This is called from _dl_map_object_from_fd or by libpthread.  */
extern int _dl_tls_setup (void) internal_function;
rtld_hidden_proto (_dl_tls_setup)

/* Allocate memory for static TLS block (unless MEM is nonzero) and dtv.  */
extern void *_dl_allocate_tls (void *mem) internal_function;

/* Get size and alignment requirements of the static TLS block.  */
extern void _dl_get_tls_static_info (size_t *sizep, size_t *alignp)
     internal_function;

extern void _dl_allocate_static_tls (struct link_map *map)
     internal_function attribute_hidden;

/* Taken from glibc/elf/dl-reloc.c */
#define CHECK_STATIC_TLS(sym_map)											\
	do {																	\
		if (__builtin_expect ((sym_map)->l_tls_offset == NO_TLS_OFFSET, 0))	\
			_dl_allocate_static_tls (sym_map);								\
	} while (0)

/* These are internal entry points to the two halves of _dl_allocate_tls,
   only used within rtld.c itself at startup time.  */
extern void *_dl_allocate_tls_storage (void)
     internal_function attribute_hidden;
extern void *_dl_allocate_tls_init (void *) internal_function;

/* Deallocate memory allocated with _dl_allocate_tls.  */
extern void _dl_deallocate_tls (void *tcb, bool dealloc_tcb) internal_function;

extern void _dl_nothread_init_static_tls (struct link_map *) attribute_hidden;

/* Highest dtv index currently needed.  */
EXTERN size_t _dl_tls_max_dtv_idx;
/* Flag signalling whether there are gaps in the module ID allocation.  */
EXTERN bool _dl_tls_dtv_gaps;
/* Information about the dtv slots.  */
EXTERN struct dtv_slotinfo_list
{
  size_t len;
  struct dtv_slotinfo_list *next;
  struct dtv_slotinfo
  {
    size_t gen;
    bool is_static;
    struct link_map *map;
  } slotinfo[0];
} *_dl_tls_dtv_slotinfo_list;
/* Number of modules in the static TLS block.  */
EXTERN size_t _dl_tls_static_nelem;
/* Size of the static TLS block.  */
EXTERN size_t _dl_tls_static_size;
/* Size actually allocated in the static TLS block.  */
EXTERN size_t _dl_tls_static_used;
/* Alignment requirement of the static TLS block.  */
EXTERN size_t _dl_tls_static_align;
/* Function pointer for catching TLS errors.  */
EXTERN void **(*_dl_error_catch_tsd) (void) __attribute__ ((const));

/* Number of additional entries in the slotinfo array of each slotinfo
   list element.  A large number makes it almost certain take we never
   have to iterate beyond the first element in the slotinfo list.  */
# define TLS_SLOTINFO_SURPLUS (62)

/* Number of additional slots in the dtv allocated.  */
# define DTV_SURPLUS	(14)

/* Initial dtv of the main thread, not allocated with normal malloc.  */
EXTERN void *_dl_initial_dtv;
/* Generation counter for the dtv.  */
EXTERN size_t _dl_tls_generation;

EXTERN void (*_dl_init_static_tls) (struct link_map *);

/* We have the auxiliary vector.  */
#define HAVE_AUX_VECTOR

/* We can assume that the kernel always provides the AT_UID, AT_EUID,
   AT_GID, and AT_EGID values in the auxiliary vector from 2.4.0 or so on.  */
#if __ASSUME_AT_XID
# define HAVE_AUX_XID
#endif

/* We can assume that the kernel always provides the AT_SECURE value
   in the auxiliary vector from 2.5.74 or so on.  */
#if __ASSUME_AT_SECURE
# define HAVE_AUX_SECURE
#endif

/* Starting with one of the 2.4.0 pre-releases the Linux kernel passes
   up the page size information.  */
#if __ASSUME_AT_PAGESIZE
# define HAVE_AUX_PAGESIZE
#endif

#endif
