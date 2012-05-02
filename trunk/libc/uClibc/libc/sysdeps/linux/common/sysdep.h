/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sys/syscall.h>

/* Not that using a `PASTE' macro loses.  */
#ifdef	__STDC__

#ifdef __ELF__

#define SYSCALL_WEAK_ALIAS(alias,orig) \
	.weak alias; \
	alias=__libc_##orig

/*
#ifdef _POSIX_THREADS
*/

#if 1

#ifdef PTHREAD_KERNEL

/* Use the regular ELF conventions about underscores, and provide the
   weak symbol, as required */
#define	SYSCALL__(name,args)	PSEUDO (__machdep_sys_##name, name, args) \
.weak machdep_sys_##name; \
  machdep_sys_##name = __machdep_sys_##name; \
.type __machdep_sys_##name,@function; \
.type machdep_sys_##name,@function; \
.L__machdep_sys_##name##end: .size __machdep_sys_##name,.L__machdep_sys_##name##end - __machdep_sys_##name

#define	SYSCALL(name,args)	PSEUDO (__machdep_sys_##name, name, args) \
.weak machdep_sys_##name; \
  machdep_sys_##name = __machdep_sys_##name; \
.type __machdep_sys_##name,@function; \
.type machdep_sys_##name,@function; \
.L__machdep_sys_##name##end: .size __machdep_sys_##name,.L__machdep_sys_##name##end - __machdep_sys_##name

#else /* PTHREAD_KERNEL */

/* Use the regular ELF conventions about underscores, and provide the
   weak symbol, as required */
#define	SYSCALL__(name,args)	PSEUDO (__libc_##name, name, args) \
.weak __##name; \
.weak name;  \
  __##name = __libc_##name; \
  name = __libc_##name; \
.type __libc_##name,@function; \
.type name,@function; \
.type __##name,@function; \
.L__libc_##name##end: .size __libc_##name,.L__libc_##name##end - __libc_##name

#define	SYSCALL(name,args)	PSEUDO (__libc_##name, name, args) \
.weak name; \
  name = __libc_##name; \
.type __libc_##name,@function; \
.type name,@function; \
.L__libc_##name##end: .size __libc_##name,.L__libc_##name##end - __libc_##name

#endif /* PTHREAD_KERNEL */

#else /* _POSIX_THREADS */

/* Use the regular ELF conventions about underscores, and provide the
   weak symbol, as required */
#define	SYSCALL__(name,args)	PSEUDO (__libc_##name, name, args) \
.weak name;  \
  __##name = __libc_##name; \
  name = __libc_##name; \
.type __libc_##name,@function; \
.type name,@function; \
.type __##name,@function; \
.L__libc_##name##end: .size __libc_##name,.L__libc_##name##end - __libc_##name

#define	SYSCALL(name,args)	PSEUDO (__libc_##name, name, args) \
  name = __libc_##name; \
.type __libc_##name,@function; \
.type name,@function; \
.L__libc_##name##end: .size __libc_##name,.L__libc_##name##end - __libc_##name

#endif /* _POSIX_THREADS */

#else /* __ELF__ */

#define SYSCALL_WEAK_ALIAS(alias,orig)

/* Regular a.out definition */
#define	SYSCALL__(name,args)	PSEUDO (__##name, name, args)
#define	SYSCALL(name,args)	PSEUDO (name, name, args)

#endif /* __ELF__ */

#else  /* __STDC__ */

#define	SYSCALL__(name,args)	PSEUDO (__/**/name, name, args)
#define	SYSCALL(name,args)	PSEUDO (name, name, args)

#endif /* __STDC__ */

/* Machine-dependent sysdep.h files are expected to define the macro
   PSEUDO (function_name, syscall_name) to emit assembly code to define the
   C-callable function FUNCTION_NAME to do system call SYSCALL_NAME.
   r0 and r1 are the system call outputs.  movl should be defined as
   an instruction such that "movl r1, r0" works.  ret should be defined
   as the return instruction.  */


#if !defined(HAVE_GNU_LD) && !defined (__ELF__)
#define	 ___errno	_errno
#endif

#define	HAVE_SYSCALLS
