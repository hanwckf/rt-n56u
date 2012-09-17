/* Definitions for thread-local data handling.  linuxthreads/s390 version.
   Copyright (C) 2002, 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _TLS_H
#define _TLS_H

#ifndef __ASSEMBLER__

# include <pt-machine.h>
# include <stdbool.h>
# include <stddef.h>

/* Type for the dtv.  */
typedef union dtv
{
  size_t counter;
  struct
  {
    void *val;
    bool is_static;
  } pointer;
} dtv_t;

typedef struct
{
  void *tcb;		/* Pointer to the TCB.  Not necessary the
			   thread descriptor used by libpthread.  */
  dtv_t *dtv;
  void *self;		/* Pointer to the thread descriptor.  */
  int multiple_threads;
} tcbhead_t;

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
#endif /* __ASSEMBLER__ */

/* TLS is always supported if the tools support it.  There are no
   kernel dependencies.  To avoid bothering with the TLS support code
   at all, use configure --without-tls.

   We need USE_TLS to be consistently defined, for ldsodefs.h
   conditionals.  */

#ifdef HAVE_TLS_SUPPORT

/* Signal that TLS support is available.  */
# define USE_TLS	1

# ifndef __ASSEMBLER__
/* Get system call information.  */
#  include <sysdep.h>


/* Get the thread descriptor definition.  */
#  include <linuxthreads/descr.h>

/* This is the size of the initial TCB.  */
#  define TLS_INIT_TCB_SIZE sizeof (tcbhead_t)

/* Alignment requirements for the initial TCB.  */
#  define TLS_INIT_TCB_ALIGN __alignof__ (tcbhead_t)

/* This is the size of the TCB.  */
#  define TLS_TCB_SIZE sizeof (struct _pthread_descr_struct)

/* Alignment requirements for the TCB.  */
#  define TLS_TCB_ALIGN __alignof__ (struct _pthread_descr_struct)

/* The TCB can have any size and the memory following the address the
   thread pointer points to is unspecified.  Allocate the TCB there.  */
#  define TLS_TCB_AT_TP	1


/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
#  define INSTALL_DTV(descr, dtvp) \
  ((tcbhead_t *) (descr))->dtv = (dtvp) + 1

/* Install new dtv for current thread.  */
#  define INSTALL_NEW_DTV(dtv) \
  (((tcbhead_t *) __builtin_thread_pointer ())->dtv = (dtv))

/* Return dtv of given thread descriptor.  */
#  define GET_DTV(descr) \
  (((tcbhead_t *) (descr))->dtv)

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.

   The value of this macro is null if successful, or an error string.  */
#  define TLS_INIT_TP(descr, secondcall)				      \
  ({									      \
    void *_descr = (descr);						      \
    tcbhead_t *head = _descr;						      \
									      \
    head->tcb = _descr;							      \
    /* For now the thread descriptor is at the same address.  */	      \
    head->self = _descr;						      \
									      \
    __builtin_set_thread_pointer (_descr);				      \
    NULL;								      \
  })

/* Return the address of the dtv for the current thread.  */
#  define THREAD_DTV() \
  (((tcbhead_t *) __builtin_thread_pointer ())->dtv)

# endif /* __ASSEMBLER__ */

#else	/* HAVE_TLS_SUPPORT && (FLOATING_STACKS || !IS_IN_libpthread) */

# ifndef __ASSEMBLER__

/* Get the thread descriptor definition.  */
#  include <linuxthreads/descr.h>

#  define NONTLS_INIT_TP \
  do { 								\
    static const tcbhead_t nontls_init_tp			\
      = { .multiple_threads = 0 };				\
    INIT_THREAD_SELF (&nontls_init_tp, 0);			\
  } while (0)

# endif /* __ASSEMBLER__ */

#endif	/* HAVE_TLS_SUPPORT && (FLOATING_STACKS || !IS_IN_libpthread) */

#endif	/* tls.h */
