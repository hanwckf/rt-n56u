/* Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

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

#include <tls.h>
#include <pt-machine.h>
#ifndef __ASSEMBLER__
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
    SINGLE_THREAD_P;							      \
    jne L(pseudo_cancel);						      \
    DO_CALL (syscall_name, args);					      \
    cmpl $-4095, %eax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
    ret;								      \
  L(pseudo_cancel):							      \
    CENABLE								      \
    SAVE_OLDTYPE_##args							      \
    PUSHCARGS_##args							      \
    DOCARGS_##args							      \
    movl $SYS_ify (syscall_name), %eax;					      \
    int $0x80								      \
    POPCARGS_##args;							      \
    POPSTATE_##args							      \
    cmpl $-4095, %eax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
  L(pseudo_end):

# define SAVE_OLDTYPE_0	movl %eax, %ecx;
# define SAVE_OLDTYPE_1	SAVE_OLDTYPE_0
# define SAVE_OLDTYPE_2	pushl %eax; cfi_adjust_cfa_offset (4);
# define SAVE_OLDTYPE_3	SAVE_OLDTYPE_2
# define SAVE_OLDTYPE_4	SAVE_OLDTYPE_2
# define SAVE_OLDTYPE_5	SAVE_OLDTYPE_2

# define PUSHCARGS_0	/* No arguments to push.  */
# define DOCARGS_0	/* No arguments to frob.  */
# define POPCARGS_0	/* No arguments to pop.  */
# define _PUSHCARGS_0	/* No arguments to push.  */
# define _POPCARGS_0	/* No arguments to pop.  */

# define PUSHCARGS_1	movl %ebx, %edx; cfi_register (ebx, edx); PUSHCARGS_0
# define DOCARGS_1	_DOARGS_1 (4)
# define POPCARGS_1	POPCARGS_0; movl %edx, %ebx; cfi_restore (ebx);
# define _PUSHCARGS_1	pushl %ebx; cfi_adjust_cfa_offset (4); \
			cfi_rel_offset (ebx, 0); _PUSHCARGS_0
# define _POPCARGS_1	_POPCARGS_0; popl %ebx; \
			cfi_adjust_cfa_offset (-4); cfi_restore (ebx);

# define PUSHCARGS_2	PUSHCARGS_1
# define DOCARGS_2	_DOARGS_2 (12)
# define POPCARGS_2	POPCARGS_1
# define _PUSHCARGS_2	_PUSHCARGS_1
# define _POPCARGS_2	_POPCARGS_1

# define PUSHCARGS_3	_PUSHCARGS_2
# define DOCARGS_3	_DOARGS_3 (20)
# define POPCARGS_3	_POPCARGS_3
# define _PUSHCARGS_3	_PUSHCARGS_2
# define _POPCARGS_3	_POPCARGS_2

# define PUSHCARGS_4	_PUSHCARGS_4
# define DOCARGS_4	_DOARGS_4 (28)
# define POPCARGS_4	_POPCARGS_4
# define _PUSHCARGS_4	pushl %esi; cfi_adjust_cfa_offset (4); \
			cfi_rel_offset (esi, 0); _PUSHCARGS_3
# define _POPCARGS_4	_POPCARGS_3; popl %esi; \
			cfi_adjust_cfa_offset (-4); cfi_restore (esi);

# define PUSHCARGS_5	_PUSHCARGS_5
# define DOCARGS_5	_DOARGS_5 (36)
# define POPCARGS_5	_POPCARGS_5
# define _PUSHCARGS_5	pushl %edi; cfi_adjust_cfa_offset (4); \
			cfi_rel_offset (edi, 0); _PUSHCARGS_4
# define _POPCARGS_5	_POPCARGS_4; popl %edi; \
			cfi_adjust_cfa_offset (-4); cfi_restore (edi);

# ifdef IS_IN_libpthread
#  define CENABLE	call __pthread_enable_asynccancel;
#  define CDISABLE	call __pthread_disable_asynccancel
# elif defined IS_IN_librt
#  ifdef __PIC__
#   define CENABLE	pushl %ebx; \
			call __i686.get_pc_thunk.bx; \
			addl     $_GLOBAL_OFFSET_TABLE_, %ebx; \
			call __librt_enable_asynccancel@PLT; \
			popl %ebx;
#   define CDISABLE	pushl %ebx; \
			call __i686.get_pc_thunk.bx; \
			addl     $_GLOBAL_OFFSET_TABLE_, %ebx; \
			call __librt_disable_asynccancel@PLT; \
			popl %ebx;
#  else
#   define CENABLE	call __librt_enable_asynccancel;
#   define CDISABLE	call __librt_disable_asynccancel
#  endif
# else
#  define CENABLE	call __libc_enable_asynccancel;
#  define CDISABLE	call __libc_disable_asynccancel
# endif
# define POPSTATE_0 \
 pushl %eax; cfi_adjust_cfa_offset (4); movl %ecx, %eax; \
 CDISABLE; popl %eax; cfi_adjust_cfa_offset (-4);
# define POPSTATE_1	POPSTATE_0
# define POPSTATE_2	xchgl (%esp), %eax; CDISABLE; popl %eax; \
			cfi_adjust_cfa_offset (-4);
# define POPSTATE_3	POPSTATE_2
# define POPSTATE_4	POPSTATE_3
# define POPSTATE_5	POPSTATE_4

#if !defined NOT_IN_libc
# define __local_multiple_threads __libc_multiple_threads
#elif defined IS_IN_libpthread
# define __local_multiple_threads __pthread_multiple_threads
#else
# define __local_multiple_threads __librt_multiple_threads
#endif

# ifndef __ASSEMBLER__
#  if defined FLOATING_STACKS && USE___THREAD && defined __PIC__
#   define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				      \
				   p_header.data.multiple_threads) == 0, 1)
#  else
extern int __local_multiple_threads
#   if !defined NOT_IN_libc || defined IS_IN_libpthread
  attribute_hidden;
#   else
  ;
#   endif
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  endif
# else
#  if !defined __PIC__
#   define SINGLE_THREAD_P cmpl $0, __local_multiple_threads
#  elif defined FLOATING_STACKS && USE___THREAD
#   define SINGLE_THREAD_P cmpl $0, %gs:MULTIPLE_THREADS_OFFSET
#  else
#   if !defined NOT_IN_libc || defined IS_IN_libpthread
#    define __SINGLE_THREAD_CMP cmpl $0, __local_multiple_threads@GOTOFF(%ecx)
#   else
#    define __SINGLE_THREAD_CMP \
  movl __local_multiple_threads@GOT(%ecx), %ecx;\
  cmpl $0, (%ecx)
#   endif
#   if !defined HAVE_HIDDEN || !USE___THREAD
#    define SINGLE_THREAD_P \
  SETUP_PIC_REG (cx);				\
  addl $_GLOBAL_OFFSET_TABLE_, %ecx;		\
  __SINGLE_THREAD_CMP
#   else
#    define SINGLE_THREAD_P \
  call __i686.get_pc_thunk.cx;			\
  addl $_GLOBAL_OFFSET_TABLE_, %ecx;		\
  __SINGLE_THREAD_CMP
#   endif
#  endif
# endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
