/* cancellable system calls for Linux/HPPA.
   Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Carlos O'Donell <carlos@baldric.uwo.ca>, 2003.

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
#ifndef __ASSEMBLER__
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# ifndef NO_ERROR
#  define NO_ERROR -0x1000
# endif

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  ENTRY (name)								\
    SINGLE_THREAD_P					ASM_LINE_SEP	\
    cmpib,<> 0,%ret0,Lpseudo_cancel			ASM_LINE_SEP	\
    nop							ASM_LINE_SEP	\
    DO_CALL(syscall_name, args)				ASM_LINE_SEP	\
    /* DONE! */						ASM_LINE_SEP	\
    bv 0(2)						ASM_LINE_SEP	\
    nop							ASM_LINE_SEP	\
  Lpseudo_cancel:					ASM_LINE_SEP	\
    /* store return ptr */				ASM_LINE_SEP	\
    stw %rp, -20(%sr0,%sp)				ASM_LINE_SEP	\
    /* save syscall args */				ASM_LINE_SEP	\
    PUSHARGS_##args /* MACRO */				ASM_LINE_SEP	\
    STW_PIC						ASM_LINE_SEP	\
    CENABLE /* FUNC CALL */				ASM_LINE_SEP	\
    ldo 64(%sp), %sp					ASM_LINE_SEP	\
    ldo -64(%sp), %sp					ASM_LINE_SEP	\
    LDW_PIC						ASM_LINE_SEP	\
    /* restore syscall args */				ASM_LINE_SEP	\
    POPARGS_##args					ASM_LINE_SEP	\
    /* save r4 in arg0 stack slot */			ASM_LINE_SEP	\
    stw %r4, -36(%sr0,%sp)				ASM_LINE_SEP	\
    /* save mask from cenable */			ASM_LINE_SEP	\
    copy %ret0, %r4					ASM_LINE_SEP	\
    ble 0x100(%sr2,%r0)					ASM_LINE_SEP    \
    ldi SYS_ify (syscall_name), %r20			ASM_LINE_SEP	\
    LDW_PIC						ASM_LINE_SEP	\
    /* pass mask as arg0 to cdisable */			ASM_LINE_SEP	\
    copy %r4, %r26					ASM_LINE_SEP	\
    copy %ret0, %r4					ASM_LINE_SEP	\
    CDISABLE						ASM_LINE_SEP	\
    ldo 64(%sp), %sp					ASM_LINE_SEP	\
    ldo -64(%sp), %sp					ASM_LINE_SEP	\
    LDW_PIC						ASM_LINE_SEP	\
    /* compare error */					ASM_LINE_SEP	\
    ldi NO_ERROR,%r1					ASM_LINE_SEP	\
    /* branch if no error */				ASM_LINE_SEP	\
    cmpb,>>=,n %r1,%r4,Lpre_end				ASM_LINE_SEP	\
    nop							ASM_LINE_SEP	\
    SYSCALL_ERROR_HANDLER				ASM_LINE_SEP	\
    ldo 64(%sp), %sp					ASM_LINE_SEP	\
    ldo -64(%sp), %sp					ASM_LINE_SEP	\
    /* No need to LDW_PIC */				ASM_LINE_SEP	\
    /* make syscall res value positive */		ASM_LINE_SEP	\
    sub %r0, %r4, %r4					ASM_LINE_SEP	\
    /* store into errno location */			ASM_LINE_SEP	\
    stw %r4, 0(%sr0,%ret0)				ASM_LINE_SEP	\
    /* return -1 */					ASM_LINE_SEP	\
    ldo -1(%r0), %ret0					ASM_LINE_SEP	\
  Lpre_end:						ASM_LINE_SEP	\
    ldw -20(%sr0,%sp), %rp             			ASM_LINE_SEP	\
    /* No need to LDW_PIC */				ASM_LINE_SEP	\
    ldw -36(%sr0,%sp), %r4				ASM_LINE_SEP

/* Save arguments into our frame */
# define PUSHARGS_0	/* nothing to do */
# define PUSHARGS_1	PUSHARGS_0 stw %r26, -36(%sr0,%sp)	ASM_LINE_SEP
# define PUSHARGS_2	PUSHARGS_1 stw %r25, -40(%sr0,%sp)	ASM_LINE_SEP
# define PUSHARGS_3	PUSHARGS_2 stw %r24, -44(%sr0,%sp)	ASM_LINE_SEP
# define PUSHARGS_4	PUSHARGS_3 stw %r23, -48(%sr0,%sp)	ASM_LINE_SEP
# define PUSHARGS_5	PUSHARGS_4 /* Args are on the stack... */
# define PUSHARGS_6	PUSHARGS_5

/* Bring them back from the stack */
# define POPARGS_0	/* nothing to do */
# define POPARGS_1	POPARGS_0 ldw -36(%sr0,%sp), %r26	ASM_LINE_SEP
# define POPARGS_2	POPARGS_1 ldw -40(%sr0,%sp), %r25	ASM_LINE_SEP
# define POPARGS_3	POPARGS_2 ldw -44(%sr0,%sp), %r24	ASM_LINE_SEP
# define POPARGS_4	POPARGS_3 ldw -48(%sr0,%sp), %r23	ASM_LINE_SEP
# define POPARGS_5	POPARGS_4 ldw -52(%sr0,%sp), %r22	ASM_LINE_SEP
# define POPARGS_6	POPARGS_5 ldw -54(%sr0,%sp), %r21	ASM_LINE_SEP

# ifdef IS_IN_libpthread
#  ifdef __PIC__
#   define CENABLE .import __pthread_enable_asynccancel,code ASM_LINE_SEP \
			bl __pthread_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE .import __pthread_disable_asynccancel,code ASM_LINE_SEP \
			bl __pthread_disable_asynccancel,%r2 ASM_LINE_SEP
#  else
#   define CENABLE .import __pthread_enable_asynccancel,code ASM_LINE_SEP \
			bl __pthread_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE .import __pthread_disable_asynccancel,code ASM_LINE_SEP \
			bl __pthread_disable_asynccancel,%r2 ASM_LINE_SEP
#  endif
# elif !defined NOT_IN_libc
#  ifdef __PIC__
#   define CENABLE .import __libc_enable_asynccancel,code ASM_LINE_SEP \
			bl __libc_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE	.import __libc_disable_asynccancel,code ASM_LINE_SEP \
			bl __libc_disable_asynccancel,%r2 ASM_LINE_SEP
#  else
#   define CENABLE .import __libc_enable_asynccancel,code ASM_LINE_SEP \
			bl __libc_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE	.import __libc_disable_asynccancel,code ASM_LINE_SEP \
			bl __libc_disable_asynccancel,%r2 ASM_LINE_SEP
#  endif
# else
#  ifdef __PIC__
#   define CENABLE .import __librt_enable_asynccancel,code ASM_LINE_SEP \
			bl __librt_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE .import __librt_disable_asynccancel,code ASM_LINE_SEP \
			bl __librt_disable_asynccancel,%r2 ASM_LINE_SEP
#  else
#   define CENABLE .import __librt_enable_asynccancel,code ASM_LINE_SEP \
			bl __librt_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE .import __librt_disable_asynccancel,code ASM_LINE_SEP \
			bl __librt_disable_asynccancel,%r2 ASM_LINE_SEP
#  endif
# endif

/* p_header.multiple_threads is +12 from the pthread_descr struct start,
   We could have called __get_cr27() but we really want less overhead */
# define MULTIPLE_THREADS_OFFSET 0xC

/* cr27 has been initialized to 0x0 by kernel */
# define NO_THREAD_CR27 0x0

# ifdef IS_IN_libpthread
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define __local_multiple_threads __libc_multiple_threads
# else
#  define __local_multiple_threads __librt_multiple_threads
# endif

# ifndef __ASSEMBLER__
 extern int __local_multiple_threads attribute_hidden;
#  define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
# else
/* This ALT version requires newer kernel support */
#  define SINGLE_THREAD_P_MFCTL						\
	mfctl %cr27, %ret0					ASM_LINE_SEP	\
	cmpib,= NO_THREAD_CR27,%ret0,Lstp			ASM_LINE_SEP	\
	nop							ASM_LINE_SEP	\
	ldw MULTIPLE_THREADS_OFFSET(%sr0,%ret0),%ret0		ASM_LINE_SEP	\
 Lstp:								ASM_LINE_SEP
#  ifdef __PIC__
/* Slower version uses GOT to get value of __local_multiple_threads */
#   define SINGLE_THREAD_P							\
	addil LT%__local_multiple_threads, %r19			ASM_LINE_SEP	\
	ldw RT%__local_multiple_threads(%sr0,%r1), %ret0	ASM_LINE_SEP	\
	ldw 0(%sr0,%ret0), %ret0 				ASM_LINE_SEP
#  else
  /* Slow non-pic version using DP */
#   define SINGLE_THREAD_P								\
	addil LR%__local_multiple_threads-$global$,%r27  		ASM_LINE_SEP	\
	ldw RR%__local_multiple_threads-$global$(%sr0,%r1),%ret0	ASM_LINE_SEP
#  endif
# endif
#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
/* !defined NOT_IN_libc || defined IS_IN_libpthread */
