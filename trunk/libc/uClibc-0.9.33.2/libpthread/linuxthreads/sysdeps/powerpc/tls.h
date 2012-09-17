/* Definitions for thread-local data handling.  linuxthreads/PPC version.
   Copyright (C) 2003, 2005 Free Software Foundation, Inc.
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

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
#endif /* __ASSEMBLER__ */

#ifdef HAVE_TLS_SUPPORT

/* Signal that TLS support is available.  */
# define USE_TLS	1

# ifndef __ASSEMBLER__

/* This layout is actually wholly private and not affected by the ABI.
   Nor does it overlap the pthread data structure, so we need nothing
   extra here at all.  */
typedef struct
{
  dtv_t *dtv;
} tcbhead_t;

/* This is the size of the initial TCB.  */
#  define TLS_INIT_TCB_SIZE	0

/* Alignment requirements for the initial TCB.  */
#  define TLS_INIT_TCB_ALIGN	__alignof__ (struct _pthread_descr_struct)

/* This is the size of the TCB.  */
#  define TLS_TCB_SIZE		0

/* Alignment requirements for the TCB.  */
#  define TLS_TCB_ALIGN		__alignof__ (struct _pthread_descr_struct)

/* This is the size we need before TCB.  */
#  define TLS_PRE_TCB_SIZE \
  (sizeof (struct _pthread_descr_struct)				      \
   + ((sizeof (tcbhead_t) + TLS_TCB_ALIGN - 1) & ~(TLS_TCB_ALIGN - 1)))

/* The following assumes that TP (R2 or R13) is points to the end of the
   TCB + 0x7000 (per the ABI).  This implies that TCB address is
   TP - 0x7000.  As we define TLS_DTV_AT_TP we can
   assume that the pthread_descr is allocated immediately ahead of the
   TCB.  This implies that the pthread_descr address is
   TP - (TLS_PRE_TCB_SIZE + 0x7000).  */
#define TLS_TCB_OFFSET		0x7000

/* The DTV is allocated at the TP; the TCB is placed elsewhere.  */
/* This is not really true for powerpc64.  We are following alpha
   where the DTV pointer is first doubleword in the TCB.  */
#  define TLS_DTV_AT_TP 1

/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
#  define INSTALL_DTV(TCBP, DTVP) \
  (((tcbhead_t *) (TCBP))[-1].dtv = (DTVP) + 1)

/* Install new dtv for current thread.  */
#  define INSTALL_NEW_DTV(DTV) (THREAD_DTV() = (DTV))

/* Return dtv of given thread descriptor.  */
#  define GET_DTV(TCBP)	(((tcbhead_t *) (TCBP))[-1].dtv)

/* We still need this define so that tcb-offsets.sym can override it and
   use THREAD_SELF to generate MULTIPLE_THREADS_OFFSET.  */
#  define __thread_register ((void *) __thread_self)

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.

   The global register variable is declared in pt-machine.h with the
   wrong type, so we need some extra casts to get the desired result.
   This avoids a lvalue cast that gcc-3.4 does not like.  */
# define TLS_INIT_TP(TCBP, SECONDCALL) \
    (__thread_self = (struct _pthread_descr_struct *) \
	((void *) (TCBP) + TLS_TCB_OFFSET), NULL)

/* Return the address of the dtv for the current thread.  */
#  define THREAD_DTV() \
     (((tcbhead_t *) ((void *) __thread_self - TLS_TCB_OFFSET))[-1].dtv)

/* Return the thread descriptor for the current thread.  */
#  undef THREAD_SELF
#  define THREAD_SELF \
    ((pthread_descr) (__thread_register \
		      - TLS_TCB_OFFSET - TLS_PRE_TCB_SIZE))

#  undef INIT_THREAD_SELF
#  define INIT_THREAD_SELF(DESCR, NR) \
     (__thread_self = (struct _pthread_descr_struct *)((void *) (DESCR) \
		           + TLS_TCB_OFFSET + TLS_PRE_TCB_SIZE))

/* Make sure we have the p_multiple_threads member in the thread structure.
   See below.  */
#  define TLS_MULTIPLE_THREADS_IN_TCB 1

/* Get the thread descriptor definition.  */
#  include <linuxthreads/descr.h>

/* l_tls_offset == 0 is perfectly valid on PPC, so we have to use some
   different value to mean unset l_tls_offset.  */
#  define NO_TLS_OFFSET	-1

# endif /* __ASSEMBLER__ */

#elif !defined __ASSEMBLER__

/* This overlaps the start of the pthread_descr.  System calls
   and such use this to find the multiple_threads flag and need
   to use the same offset relative to the thread register in both
   single-threaded and multi-threaded code.  */
typedef struct
{
  void *tcb;			/* Never used.  */
  dtv_t *dtv;			/* Never used.  */
  void *self;			/* Used only if multithreaded, and rarely.  */
  int multiple_threads;		/* Only this member is really used.  */
} tcbhead_t;

#define NONTLS_INIT_TP							\
  do {									\
    static const tcbhead_t nontls_init_tp = { .multiple_threads = 0 };	\
    __thread_self = (__typeof (__thread_self)) &nontls_init_tp;		\
  } while (0)

#endif /* HAVE_TLS_SUPPORT */

#endif	/* tls.h */
