/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1998 Xavier Leroy (Xavier.Leroy@inria.fr)	        */
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

#define NO_PTR_DEMANGLE

#include <setjmp.h>
#include "pthread.h"
#include "internals.h"
#ifndef NO_PTR_DEMANGLE
#include <jmpbuf-unwind.h>
#define __JMPBUF_UNWINDS(a,b,c) _JMPBUF_UNWINDS(a,b,c)
#else
#define __JMPBUF_UNWINDS(a,b,c) _JMPBUF_UNWINDS(a,b)
#endif

#ifndef NO_PTR_DEMANGLE
static __inline__ uintptr_t
demangle_ptr (uintptr_t x)
{
#ifdef PTR_DEMANGLE
  PTR_DEMANGLE (x);
#endif
  return x;
}
#else
#define demangle_ptr(x) x
#endif

void __pthread_cleanup_upto (__jmp_buf target, char *targetframe)
{
  pthread_descr self = thread_self();
  struct _pthread_cleanup_buffer * c;

  for (c = THREAD_GETMEM(self, p_cleanup);
       c != NULL && __JMPBUF_UNWINDS(target, c, demangle_ptr);
       c = c->__prev)
    {
#ifdef _STACK_GROWS_DOWN
      if ((char *) c <= targetframe)
	{
	  c = NULL;
	  break;
	}
#elif defined _STACK_GROWS_UP
      if ((char *) c >= targetframe)
	{
	  c = NULL;
	  break;
	}
#else
# error "Define either _STACK_GROWS_DOWN or _STACK_GROWS_UP"
#endif
      c->__routine(c->__arg);
    }
  THREAD_SETMEM(self, p_cleanup, c);
  if (THREAD_GETMEM(self, p_in_sighandler)
      && __JMPBUF_UNWINDS(target, THREAD_GETMEM(self, p_in_sighandler),
			 demangle_ptr))
    THREAD_SETMEM(self, p_in_sighandler, NULL);
}
