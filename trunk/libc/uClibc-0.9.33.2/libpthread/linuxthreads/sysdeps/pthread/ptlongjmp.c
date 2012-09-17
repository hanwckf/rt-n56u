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

#include <setjmp.h>
#include "pthread.h"
#include "internals.h"

/* These functions are not declared anywhere since they shouldn't be
   used at another place but here.  */
extern void __libc_siglongjmp (sigjmp_buf env, int val)
     __attribute__ ((noreturn));
extern void __libc_longjmp (sigjmp_buf env, int val)
     __attribute__ ((noreturn));

#ifdef SHARED
void siglongjmp (sigjmp_buf env, int val)
{
  __libc_siglongjmp (env, val);
}

void longjmp (jmp_buf env, int val)
{
  __libc_longjmp (env, val);
}
#endif
