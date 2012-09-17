/* Definitions for thread-local data handling.  linuxthreads/MIPS version.
   Copyright (C) 2005 Free Software Foundation, Inc.
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

# include <stdbool.h>
# include <pt-machine.h>
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

# define READ_THREAD_POINTER() \
    ({ void *__result;							      \
       __asm__ __volatile__ (".set\tpush\n\t.set\tmips32r2\n\t"			      \
		     "rdhwr\t%0, $29\n\t.set\tpop" : "=v" (__result));	      \
       __result; })

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>

/* Note: rd must be $v1 to be ABI-conformant.  */
# define READ_THREAD_POINTER(rd) \
	.set	push;							      \
	.set	mips32r2;						      \
	rdhwr	rd, $29;						      \
	.set	pop
#endif /* __ASSEMBLER__ */

/* LinuxThreads can only use TLS if both floating stacks (in the MIPS case,
   that means support for "rdhwr") and support from the tools are available.

   We have to define USE_TLS consistently, or ldsodefs.h will lay out types
   differently between an NPTL build and a LinuxThreads build.  It can be set
   for libc.so and not libpthread.so, but only if we provide appropriate padding
   in the _pthread_descr_struct.

   Currently nothing defines FLOATING_STACKS.  We could assume this based on
   kernel version once the TLS patches are available in kernel.org, but
   it hardly seems worth it.  Use NPTL if you can.

   To avoid bothering with the TLS support code at all, use configure
   --without-tls.  */

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

/* This layout is actually wholly private and not affected by the ABI.
   Nor does it overlap the pthread data structure, so we need nothing
   extra here at all.  */
typedef struct
{
  dtv_t *dtv;
  void *private;
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

/* The thread pointer (in hardware register $29) points to the end of
   the TCB + 0x7000, as for PowerPC.  The pthread_descr structure is
   immediately in front of the TCB.  */
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

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(tcbp, secondcall) \
  ({ INTERNAL_SYSCALL_DECL (err);					\
     long result_var;							\
     result_var = INTERNAL_SYSCALL (set_thread_area, err, 1,		\
				    (char *) (tcbp) + TLS_TCB_OFFSET);	\
     INTERNAL_SYSCALL_ERROR_P (result_var, err)				\
       ? "unknown error" : NULL; })

/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV() \
  (((tcbhead_t *) (READ_THREAD_POINTER () - TLS_TCB_OFFSET))[-1].dtv)

/* Return the thread descriptor for the current thread.  */
#  undef THREAD_SELF
#  define THREAD_SELF \
    ((pthread_descr) (READ_THREAD_POINTER () \
		      - TLS_TCB_OFFSET - TLS_PRE_TCB_SIZE))

/* Get the thread descriptor definition.  */
#  include <linuxthreads/descr.h>

/* l_tls_offset == 0 is perfectly valid on MIPS, so we have to use some
   different value to mean unset l_tls_offset.  */
#  define NO_TLS_OFFSET	-1

/* Initializing the thread pointer requires a syscall which may not be
   available, so don't do it if we don't need to.  */
#  define TLS_INIT_TP_EXPENSIVE 1

# endif /* __ASSEMBLER__ */

#endif /* HAVE_TLS_SUPPORT */

#endif	/* tls.h */
