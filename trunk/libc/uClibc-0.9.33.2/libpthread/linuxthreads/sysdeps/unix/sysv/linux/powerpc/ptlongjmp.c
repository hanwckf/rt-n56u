/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1998 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Redefine siglongjmp and longjmp so that they interact correctly
   with cleanup handlers */
/* Derived from linuxthreads/ptlongjmp.c & added AltiVec/VMX versioning. */
#include "pthread.h"
#include <setjmp.h>
#include <bits/wordsize.h>
#include <shlib-compat.h>
#if defined SHARED
# if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_3_4)

/* These functions are not declared anywhere since they shouldn't be
   used at another place but here.  */
extern void __novmx__libc_siglongjmp (sigjmp_buf env, int val)
     __attribute__ ((noreturn));
extern void __novmx__libc_longjmp (sigjmp_buf env, int val)
     __attribute__ ((noreturn));


void __novmx_siglongjmp (sigjmp_buf env, int val)
{
  __novmx__libc_siglongjmp (env, val);
}

void __novmx_longjmp (jmp_buf env, int val)
{
  __novmx__libc_longjmp (env, val);
}

#  if __WORDSIZE == 64
symbol_version (__novmx_longjmp,longjmp,GLIBC_2.3);
symbol_version (__novmx_siglongjmp,siglongjmp,GLIBC_2.3);
#  else
symbol_version (__novmx_longjmp,longjmp,GLIBC_2.0);
symbol_version (__novmx_siglongjmp,siglongjmp,GLIBC_2.0);
#  endif
# endif  /* SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_3_4) ) */

/* These functions are not declared anywhere since they shouldn't be
   used at another place but here.  */
extern void __vmx__libc_siglongjmp (sigjmp_buf env, int val)
     __attribute__ ((noreturn));
extern void __vmx__libc_longjmp (sigjmp_buf env, int val)
     __attribute__ ((noreturn));

void __vmx_siglongjmp (sigjmp_buf env, int val)
{
  __vmx__libc_siglongjmp (env, val);
}

void __vmx_longjmp (jmp_buf env, int val)
{
  __vmx__libc_longjmp (env, val);
}
default_symbol_version (__vmx_longjmp,longjmp,GLIBC_2.3.4);
default_symbol_version (__vmx_siglongjmp,siglongjmp,GLIBC_2.3.4);
#endif /* SHARED */
