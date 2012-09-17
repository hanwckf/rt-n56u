/* Definitions for thread-local data handling.  linuxthreads/ARM version.
   Copyright (C) 2004 Free Software Foundation, Inc.
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

typedef struct
{
  dtv_t *dtv;

  /* Reserved for the thread implementation.  Unused in LinuxThreads.  */
  void *private;
} tcbhead_t;
#endif


/* We can support TLS only if the floating-stack support is available.
   However, we want to compile in the support and test at runtime whether
   the running kernel can support it or not.  To avoid bothering with the
   TLS support code at all, use configure --without-tls.

   We need USE_TLS to be consistently defined, for ldsodefs.h conditionals.
   But some of the code below can cause problems in building libpthread
   (e.g. useldt.h will defined FLOATING_STACKS when it shouldn't).  */

/* LinuxThreads can only support TLS if both floating stacks and support
   from the tools are available.

   We have to define USE_TLS consistently, or ldsodefs.h will lay out types
   differently between an NPTL build and a LinuxThreads build.  It can be set
   for libc.so and not libpthread.so, but only if we provide appropriate padding
   in the _pthread_descr_struct.

   Currently nothing defines FLOATING_STACKS.  We could assume this based on
   kernel version once the TLS patches are available in kernel.org.

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
/* Get system call information.  */
#  include <sysdep.h>

/* This is the size of the initial TCB.  */
#  define TLS_INIT_TCB_SIZE	sizeof (tcbhead_t)

/* Alignment requirements for the initial TCB.  */
#  define TLS_INIT_TCB_ALIGN	__alignof__ (tcbhead_t)

/* This is the size of the TCB.  */
#  define TLS_TCB_SIZE		sizeof (tcbhead_t)

/* Alignment requirements for the TCB.  */
#  define TLS_TCB_ALIGN		__alignof__ (tcbhead_t)

/* This is the size we need before TCB.  */
#  define TLS_PRE_TCB_SIZE	sizeof (struct _pthread_descr_struct)

/* The DTV is allocated at the TP; the TCB is placed elsewhere.  */
#  define TLS_DTV_AT_TP 1

/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
#  define INSTALL_DTV(TCBP, DTVP) \
  (((tcbhead_t *) (TCBP))->dtv = (DTVP) + 1)

/* Install new dtv for current thread.  */
#  define INSTALL_NEW_DTV(DTV) \
  (((tcbhead_t *)__builtin_thread_pointer ())->dtv = (DTV))

/* Return dtv of given thread descriptor.  */
#  define GET_DTV(TCBP) \
  (((tcbhead_t *) (TCBP))->dtv)

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(TCBP, SECONDCALL) \
  ({ INTERNAL_SYSCALL_DECL (err);					\
     long result_var;							\
     result_var = INTERNAL_SYSCALL_ARM (set_tls, err, 1, (TCBP));	\
     INTERNAL_SYSCALL_ERROR_P (result_var, err)				\
       ? "unknown error" : NULL; })

/* Return the address of the dtv for the current thread.  */
#  define THREAD_DTV() \
  (((tcbhead_t *)__builtin_thread_pointer ())->dtv)

/* Return the thread descriptor for the current thread.  */
#  undef THREAD_SELF
#  define THREAD_SELF \
  ((pthread_descr)__builtin_thread_pointer () - 1)

#  undef INIT_THREAD_SELF
#  define INIT_THREAD_SELF(DESCR, NR) \
  TLS_INIT_TP ((struct _pthread_descr_struct *)(DESCR) + 1, 0)

/* Get the thread descriptor definition.  */
#  include <linuxthreads/descr.h>

/* ??? Generic bits of LinuxThreads may call these macros with
   DESCR set to NULL.  We are expected to be able to reference
   the "current" value.

   In our case, we'd really prefer to use DESCR, since lots of
   PAL_code calls would be expensive.  We can only trust that
   the compiler does its job and unifies the multiple
   __builtin_thread_pointer instances.  */

#define THREAD_GETMEM(descr, member) \
  ((void) sizeof (descr), THREAD_SELF->member)
#define THREAD_GETMEM_NC(descr, member) \
  ((void) sizeof (descr), THREAD_SELF->member)
#define THREAD_SETMEM(descr, member, value) \
  ((void) sizeof (descr), THREAD_SELF->member = (value))
#define THREAD_SETMEM_NC(descr, member, value) \
  ((void) sizeof (descr), THREAD_SELF->member = (value))

/* Initializing the thread pointer will generate a SIGILL if the syscall
   is not available.  */
#define TLS_INIT_TP_EXPENSIVE 1

# endif	/* HAVE_TLS_SUPPORT */
#endif /* __ASSEMBLER__ */

#endif	/* tls.h */
