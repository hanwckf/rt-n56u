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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

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
  void *pointer;
} dtv_t;


typedef struct
{
  void *tcb;		/* Pointer to the TCB.  Not necessary the
			   thread descriptor used by libpthread.  */
  dtv_t *dtv;
  void *self;		/* Pointer to the thread descriptor.  */
} tcbhead_t;


/* We can support TLS only if the floating-stack support is available.  */
#if defined FLOATING_STACKS && defined HAVE_TLS_SUPPORT

/* Get system call information.  */
# include <sysdep.h>

/* Signal that TLS support is available.  */
//# define USE_TLS	1


/* Get the thread descriptor definition.  */
# include <linuxthreads/descr.h>

/* This is the size of the initial TCB.  */
# define TLS_INIT_TCB_SIZE sizeof (tcbhead_t)

/* Alignment requirements for the initial TCB.  */
# define TLS_INIT_TCB_ALIGN __alignof__ (tcbhead_t)

/* This is the size of the TCB.  */
# define TLS_TCB_SIZE sizeof (struct _pthread_descr_struct)

/* Alignment requirements for the TCB.  */
# define TLS_TCB_ALIGN __alignof__ (struct _pthread_descr_struct)

/* The TLS blocks start right after the TCB.  */
# define TLS_DTV_AT_TP	1


/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
# define INSTALL_DTV(descr, dtvp) \
  ((tcbhead_t *) (descr))->dtv = dtvp + 1

/* Install new dtv for current thread.  */
# define INSTALL_NEW_DTV(dtv) \
  ({ struct _pthread_descr_struct *__descr;				      \
     THREAD_SETMEM (__descr, p_header.data.dtvp, (dtv)); })

/* Return dtv of given thread descriptor.  */
# define GET_DTV(descr) \
  (((tcbhead_t *) (descr))->dtv)

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(descr, secondcall) \
  ({									      \
    void *_descr = (descr);						      \
    int result;								      \
    tcbhead_t *head = _descr;						      \
									      \
    head->tcb = _descr;							      \
    /* For now the thread descriptor is at the same address.  */	      \
    head->self = _descr;						      \
									      \
    __asm__ ("ldc %0,gbr" : : "r" (_descr));				      \
									      \
    0;									      \
  })


/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV() \
  ({ struct _pthread_descr_struct *__descr;				      \
     THREAD_GETMEM (__descr, p_header.data.dtvp); })

#endif	/* FLOATING_STACKS && HAVE_TLS_SUPPORT */
#endif /* __ASSEMBLER__ */

#endif	/* tls.h */
