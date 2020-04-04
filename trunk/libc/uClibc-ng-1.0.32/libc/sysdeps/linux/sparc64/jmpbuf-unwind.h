/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <setjmp.h>
#include <jmpbuf-offsets.h>

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address)		\
  ((unsigned long int) (address)			\
   < (jmpbuf)->__uc_mcontext.__mc_gregs[MC_O6] + 2047)

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <setjmp.h>
#include <stdint.h>
#include <unwind.h>

#define _JMPBUF_CFA_UNWINDS_ADJ(_jmpbuf, _context, _adj) \
  _JMPBUF_UNWINDS_ADJ (_jmpbuf, (void *) _Unwind_GetCFA (_context), _adj)

#define _JMPBUF_UNWINDS_ADJ(_jmpbuf, _address, _adj) \
  ((uintptr_t) (_address) - (_adj) \
   < (uintptr_t) (_jmpbuf)[0].__uc_mcontext.__mc_gregs[MC_O6] + 2047 - (_adj))

#endif
