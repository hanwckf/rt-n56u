/* Definition for thread-local data handling.  linuxthreads/SH version.
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

# include <pt-machine.h>

#ifndef __ASSEMBLER__
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

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

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
#endif /* __ASSEMBLER__ */


/* We can support TLS only if the floating-stack support is available.  */
#if defined HAVE_TLS_SUPPORT \
    && (defined FLOATING_STACKS || !defined IS_IN_libpthread)

/* Signal that TLS support is available.  */
# define USE_TLS	1

/* Include padding in _pthread_descr_struct so that libc can find p_errno,
   if libpthread will only include the padding because of the !IS_IN_libpthread
   check.  */
#ifndef FLOATING_STACKS
# define INCLUDE_TLS_PADDING	1
#endif

# ifndef __ASSEMBLER__

typedef struct
{
  dtv_t *dtv;
  void *private;
} tcbhead_t;

/* This is the size of the initial TCB.  */
#  define TLS_INIT_TCB_SIZE sizeof (tcbhead_t)

/* Alignment requirements for the initial TCB.  */
#  define TLS_INIT_TCB_ALIGN __alignof__ (tcbhead_t)

/* This is the size of the TCB.  */
#  define TLS_TCB_SIZE sizeof (tcbhead_t)

/* This is the size we need before TCB.  */
#  define TLS_PRE_TCB_SIZE sizeof (struct _pthread_descr_struct)

/* Alignment requirements for the TCB.  */
#  define TLS_TCB_ALIGN __alignof__ (struct _pthread_descr_struct)

/* The TLS blocks start right after the TCB.  */
#  define TLS_DTV_AT_TP	1

/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
#  define INSTALL_DTV(tcbp, dtvp) \
  ((tcbhead_t *) (tcbp))->dtv = dtvp + 1

/* Install new dtv for current thread.  */
#  define INSTALL_NEW_DTV(dtv) \
  ({ tcbhead_t *__tcbp;							      \
     __asm__ __volatile__ ("stc gbr,%0" : "=r" (__tcbp));			      \
     __tcbp->dtv = (dtv);})

/* Return dtv of given thread descriptor.  */
#  define GET_DTV(tcbp) \
  (((tcbhead_t *) (tcbp))->dtv)

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
#  define TLS_INIT_TP(tcbp, secondcall) \
  ({ __asm__ __volatile__ ("ldc %0,gbr" : : "r" (tcbp)); 0; })

/* Return the address of the dtv for the current thread.  */
#  define THREAD_DTV() \
  ({ tcbhead_t *__tcbp;							      \
     __asm__ __volatile__ ("stc gbr,%0" : "=r" (__tcbp));			      \
     __tcbp->dtv;})

/* Return the thread descriptor for the current thread.  */
#  undef THREAD_SELF
#  define THREAD_SELF \
  ({ struct _pthread_descr_struct *__self;				      \
     __asm__ ("stc gbr,%0" : "=r" (__self));				      \
     __self - 1;})

#  undef INIT_THREAD_SELF
#  define INIT_THREAD_SELF(descr, nr) \
  ({ struct _pthread_descr_struct *__self = (void *) descr;		      \
     __asm__ __volatile__ ("ldc %0,gbr" : : "r" (__self + 1));		      \
     0; })

# define TLS_MULTIPLE_THREADS_IN_TCB 1

/* Get the thread descriptor definition.  This must be after the
   the definition of THREAD_SELF for TLS.  */
#  include <linuxthreads/descr.h>

# endif /* __ASSEMBLER__ */

#else

# ifndef __ASSEMBLER__

typedef struct
{
  void *tcb;
  dtv_t *dtv;
  void *self;
  int multiple_threads;
} tcbhead_t;

/* Get the thread descriptor definition.  */
#  include <linuxthreads/descr.h>

#  define NONTLS_INIT_TP \
  do { 									\
    static const tcbhead_t nontls_init_tp = { .multiple_threads = 0 };	\
    __asm__ __volatile__ ("ldc %0,gbr" : : "r" (&nontls_init_tp));	        \
  } while (0)

# endif /* __ASSEMBLER__ */

#endif	/* HAVE_TLS_SUPPORT && (FLOATING_STACKS || !IS_IN_libpthread) */

#endif	/* tls.h */
