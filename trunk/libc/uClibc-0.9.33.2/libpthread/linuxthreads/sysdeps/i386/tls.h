/* Definition for thread-local data handling.  linuxthreads/i386 version.
   Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
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


typedef struct
{
  void *tcb;		/* Pointer to the TCB.  Not necessary the
			   thread descriptor used by libpthread.  */
  dtv_t *dtv;
  void *self;		/* Pointer to the thread descriptor.  */
  int multiple_threads;
  uintptr_t sysinfo;
} tcbhead_t;

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
#endif

/* We can support TLS only if the floating-stack support is available.
   However, we want to compile in the support and test at runtime whether
   the running kernel can support it or not.  To avoid bothering with the
   TLS support code at all, use configure --without-tls.

   We need USE_TLS to be consistently defined, for ldsodefs.h conditionals.
   But some of the code below can cause problems in building libpthread
   (e.g. useldt.h will defined FLOATING_STACKS when it shouldn't).  */

#if defined HAVE_TLS_SUPPORT \
    && (defined FLOATING_STACKS || !defined IS_IN_libpthread)

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
  ({ struct _pthread_descr_struct *__descr;				      \
     THREAD_SETMEM (__descr, p_header.data.dtvp, (dtv)); })

/* Return dtv of given thread descriptor.  */
#  define GET_DTV(descr) \
  (((tcbhead_t *) (descr))->dtv)

#  ifdef __PIC__
#   define TLS_EBX_ARG "r"
#   define TLS_LOAD_EBX "xchgl %3, %%ebx\n\t"
#  else
#   define TLS_EBX_ARG "b"
#   define TLS_LOAD_EBX
#  endif

#  if !defined IS_IN_libpthread && !defined DO_MODIFY_LDT
#   include "useldt.h"		/* For the structure.  */
#  endif
#  if __ASSUME_LDT_WORKS > 0
#   define TLS_DO_MODIFY_LDT_KERNEL_CHECK(doit) (doit) /* Nothing to check.  */
#  else
#   define TLS_DO_MODIFY_LDT_KERNEL_CHECK(doit)				      \
  (__builtin_expect (GLRO(dl_osversion) < 131939, 0)			      \
   ? "kernel too old for thread-local storage support\n"			      \
   : (doit))
#  endif

#  define TLS_DO_MODIFY_LDT(descr, nr)					      \
TLS_DO_MODIFY_LDT_KERNEL_CHECK(						      \
({									      \
  struct modify_ldt_ldt_s ldt_entry =					      \
    { nr, (unsigned long int) (descr), 0xfffff /* 4GB in pages */,	      \
      1, 0, 0, 1, 0, 1, 0 };						      \
  int result;								      \
  __asm__ __volatile__ (TLS_LOAD_EBX						      \
		"int $0x80\n\t"						      \
		TLS_LOAD_EBX						      \
		: "=a" (result)						      \
		: "0" (__NR_modify_ldt),				      \
		/* The extra argument with the "m" constraint is necessary    \
		   to let the compiler know that we are accessing LDT_ENTRY   \
		   here.  */						      \
		"m" (ldt_entry), TLS_EBX_ARG (1), "c" (&ldt_entry),	      \
		"d" (sizeof (ldt_entry)));				      \
  __builtin_expect (result, 0) == 0					      \
  ? ({ __asm__ ("movw %w0, %%gs" : : "q" ((nr) * 8 + 7)); NULL; })		      \
  : "cannot set up LDT for thread-local storage\n";			      \
}))

#  define TLS_DO_SET_THREAD_AREA(descr, secondcall)			      \
({									      \
  struct modify_ldt_ldt_s ldt_entry =					      \
    { -1, (unsigned long int) (descr), 0xfffff /* 4GB in pages */,	      \
      1, 0, 0, 1, 0, 1, 0 };						      \
  int result;								      \
  if (secondcall)							      \
    ldt_entry.entry_number = ({ int _gs;				      \
				__asm__ ("movw %%gs, %w0" : "=q" (_gs));	      \
				(_gs & 0xffff) >> 3; });		      \
  __asm__ __volatile__ (TLS_LOAD_EBX						      \
		"int $0x80\n\t"						      \
		TLS_LOAD_EBX						      \
		: "=a" (result), "=m" (ldt_entry.entry_number)		      \
		: "0" (__NR_set_thread_area),				      \
		/* The extra argument with the "m" constraint is necessary    \
		   to let the compiler know that we are accessing LDT_ENTRY   \
		   here.  */						      \
		TLS_EBX_ARG (&ldt_entry), "m" (ldt_entry));		      \
  if (__builtin_expect (result, 0) == 0)				      \
    __asm__ ("movw %w0, %%gs" : : "q" (ldt_entry.entry_number * 8 + 3));	      \
  result;								      \
})

#  ifdef __ASSUME_SET_THREAD_AREA_SYSCALL
#   define TLS_SETUP_GS_SEGMENT(descr, secondcall)			      \
  (TLS_DO_SET_THREAD_AREA (descr, secondcall)				      \
   ? "set_thread_area failed when setting up thread-local storage\n" : NULL)
#  elif defined __NR_set_thread_area
#   define TLS_SETUP_GS_SEGMENT(descr, secondcall) \
  (TLS_DO_SET_THREAD_AREA (descr, secondcall)				      \
   ? TLS_DO_MODIFY_LDT (descr, 0) : NULL)
#  else
#   define TLS_SETUP_GS_SEGMENT(descr, secondcall) \
  TLS_DO_MODIFY_LDT ((descr), 0)
#  endif

#if defined NEED_DL_SYSINFO
# define INIT_SYSINFO \
  head->sysinfo = GLRO(dl_sysinfo)
#else
# define INIT_SYSINFO
#endif

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
    INIT_SYSINFO;							      \
    TLS_SETUP_GS_SEGMENT (_descr, secondcall);				      \
  })

/* Indicate that dynamic linker shouldn't try to initialize TLS even
   when no PT_TLS segments are found in the program and libraries
   it is linked against.  */
#  define TLS_INIT_TP_EXPENSIVE 1

/* Return the address of the dtv for the current thread.  */
#  define THREAD_DTV() \
  ({ struct _pthread_descr_struct *__descr;				      \
     THREAD_GETMEM (__descr, p_header.data.dtvp); })

# endif	/* HAVE_TLS_SUPPORT && (FLOATING_STACKS || !IS_IN_libpthread) */
#endif /* __ASSEMBLER__ */

#endif	/* tls.h */
