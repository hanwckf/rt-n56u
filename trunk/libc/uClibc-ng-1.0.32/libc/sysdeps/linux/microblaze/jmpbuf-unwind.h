/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <setjmp.h>

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)[0].__sp)

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <stdint.h>
#include <unwind.h>

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#undef _JMPBUF_UNWINDS
#define _JMPBUF_UNWINDS(jmpbuf, address)		\
  ((void *) (address) < (void *) (jmpbuf)[0].__sp)

#define _JMPBUF_CFA_UNWINDS_ADJ(_jmpbuf, _context, _adj) \
  _JMPBUF_UNWINDS_ADJ (_jmpbuf, (void *) _Unwind_GetCFA (_context), _adj)

static inline uintptr_t __attribute__ ((unused))
_jmpbuf_sp (__jmp_buf regs)
{
  void *sp = (void *) regs[0].__sp;
  return (uintptr_t) sp;
}

#define _JMPBUF_UNWINDS_ADJ(_jmpbuf, _address, _adj) \
  ((uintptr_t) (_address) - (_adj) < _jmpbuf_sp (_jmpbuf) - (_adj))

#endif
