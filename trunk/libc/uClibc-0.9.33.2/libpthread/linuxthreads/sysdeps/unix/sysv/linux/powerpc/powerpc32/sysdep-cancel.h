/* Copyright (C) 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Franz Sirl <Franz.Sirl-kernel@lauterbach.com>, 2003.

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

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .section ".text";							\
  ENTRY (name)								\
    SINGLE_THREAD_P;							\
    bne- .Lpseudo_cancel;						\
    DO_CALL (SYS_ify (syscall_name));					\
    PSEUDO_RET;								\
  .Lpseudo_cancel:							\
    stwu 1,-48(1);							\
    mflr 9;								\
    stw 9,52(1);							\
    CGOTSETUP;								\
    DOCARGS_##args;	/* save syscall args around CENABLE.  */	\
    CENABLE;								\
    stw 3,16(1);	/* store CENABLE return value (MASK).  */	\
    UNDOCARGS_##args;	/* restore syscall args.  */			\
    DO_CALL (SYS_ify (syscall_name));					\
    mfcr 0;		/* save CR/R3 around CDISABLE.  */		\
    stw 3,8(1);								\
    stw 0,12(1);							\
    lwz 3,16(1);	/* pass MASK to CDISABLE.  */			\
    CDISABLE;								\
    lwz 4,52(1);							\
    lwz 0,12(1);	/* restore CR/R3. */				\
    lwz 3,8(1);								\
    CGOTRESTORE;							\
    mtlr 4;								\
    mtcr 0;								\
    addi 1,1,48;

# define DOCARGS_0
# define UNDOCARGS_0

# define DOCARGS_1	stw 3,20(1); DOCARGS_0
# define UNDOCARGS_1	lwz 3,20(1); UNDOCARGS_0

# define DOCARGS_2	stw 4,24(1); DOCARGS_1
# define UNDOCARGS_2	lwz 4,24(1); UNDOCARGS_1

# define DOCARGS_3	stw 5,28(1); DOCARGS_2
# define UNDOCARGS_3	lwz 5,28(1); UNDOCARGS_2

# define DOCARGS_4	stw 6,32(1); DOCARGS_3
# define UNDOCARGS_4	lwz 6,32(1); UNDOCARGS_3

# define DOCARGS_5	stw 7,36(1); DOCARGS_4
# define UNDOCARGS_5	lwz 7,36(1); UNDOCARGS_4

# define DOCARGS_6	stw 8,40(1); DOCARGS_5
# define UNDOCARGS_6	lwz 8,40(1); UNDOCARGS_5

# define CGOTSETUP
# define CGOTRESTORE

# ifdef IS_IN_libpthread
#  define CENABLE	bl __pthread_enable_asynccancel@local
#  define CDISABLE	bl __pthread_disable_asynccancel@local
# elif !defined NOT_IN_libc
#  define CENABLE	bl __libc_enable_asynccancel@local
#  define CDISABLE	bl __libc_disable_asynccancel@local
# else
#  define CENABLE	bl JUMPTARGET(__librt_enable_asynccancel)
#  define CDISABLE	bl JUMPTARGET(__librt_disable_asynccancel)
#  if defined HAVE_AS_REL16 && defined __PIC__
#   undef CGOTSETUP
#   define CGOTSETUP							\
    bcl 20,31,1f;							\
 1: stw 30,44(1);							\
    mflr 30;								\
    addis 30,30,_GLOBAL_OFFSET_TABLE-1b@ha;				\
    addi 30,30,_GLOBAL_OFFSET_TABLE-1b@l
#   undef CGOTRESTORE
#   define CGOTRESTORE							\
    lwz 30,44(1)
#  endif
# endif

# ifdef HAVE_TLS_SUPPORT
#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P						\
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, p_multiple_threads) == 0, 1)
#  else
#   define SINGLE_THREAD_P						\
  lwz 10,MULTIPLE_THREADS_OFFSET(2);					\
  cmpwi 10,0
#  endif
# else
#  if !defined NOT_IN_libc
#   define __local_multiple_threads __libc_multiple_threads
#  else
#   define __local_multiple_threads __librt_multiple_threads
#  endif
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   if !defined __PIC__
#    define SINGLE_THREAD_P						\
  lis 10,__local_multiple_threads@ha;					\
  lwz 10,__local_multiple_threads@l(10);				\
  cmpwi 10,0
#   else
#    ifdef HAVE_ASM_PPC_REL16
#     define SINGLE_THREAD_P						\
  mflr 9;								\
  bcl 20,31,1f;								\
1:mflr 10;								\
  addis 10,10,__local_multiple_threads-1b@ha;				\
  lwz 10,__local_multiple_threads-1b@l(10);				\
  mtlr 9;								\
  cmpwi 10,0
#    else
#     define SINGLE_THREAD_P						\
  mflr 9;								\
  bl _GLOBAL_OFFSET_TABLE_@local-4;					\
  mflr 10;								\
  mtlr 9;								\
  lwz 10,__local_multiple_threads@got(10);				\
  lwz 10,0(10);								\
  cmpwi 10,0
#    endif
#   endif
#  endif
# endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
