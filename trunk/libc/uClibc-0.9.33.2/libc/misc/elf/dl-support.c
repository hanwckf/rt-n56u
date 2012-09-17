/*
 * Support for dynamic linking code in static libc.
 * Copyright (C) 1996-2002, 2003, 2004, 2005 Free Software Foundation, Inc.
 *
 * Partially based on GNU C Library (file: libc/elf/dl-support.c)
 *
 * Copyright (C) 2008 STMicroelectronics Ltd.
 * Author: Carmelo Amoroso <carmelo.amoroso@st.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 */

#include <link.h>
#include <elf.h>
#if defined(USE_TLS) && USE_TLS
#include <assert.h>
#include <tls.h>
#include <ldsodefs.h>
#include <string.h>
#endif

#if defined(USE_TLS) && USE_TLS

void (*_dl_init_static_tls) (struct link_map *) = &_dl_nothread_init_static_tls;

#endif

ElfW(Phdr) *_dl_phdr;
size_t _dl_phnum;

void internal_function _dl_aux_init (ElfW(auxv_t) *av);
void internal_function _dl_aux_init (ElfW(auxv_t) *av)
{
   /* Get the program headers base address from the aux vect */
   _dl_phdr = (ElfW(Phdr) *) av[AT_PHDR].a_un.a_val;

   /* Get the number of program headers from the aux vect */
   _dl_phnum = (size_t) av[AT_PHNUM].a_un.a_val;
}

#if defined(USE_TLS) && USE_TLS
/* Initialize static TLS area and DTV for current (only) thread.
   libpthread implementations should provide their own hook
   to handle all threads.  */
void
attribute_hidden
_dl_nothread_init_static_tls (struct link_map *map)
{
# if defined(TLS_TCB_AT_TP)
  void *dest = (char *) THREAD_SELF - map->l_tls_offset;
# elif defined(TLS_DTV_AT_TP)
  void *dest = (char *) THREAD_SELF + map->l_tls_offset + TLS_PRE_TCB_SIZE;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* Fill in the DTV slot so that a later LD/GD access will find it.  */
  dtv_t *dtv = THREAD_DTV ();
  assert (map->l_tls_modid <= dtv[-1].counter);
  dtv[map->l_tls_modid].pointer.val = dest;
  dtv[map->l_tls_modid].pointer.is_static = true;

  /* Initialize the memory.  */
  memset (mempcpy (dest, map->l_tls_initimage, map->l_tls_initimage_size),
	  '\0', map->l_tls_blocksize - map->l_tls_initimage_size);
}

#endif

