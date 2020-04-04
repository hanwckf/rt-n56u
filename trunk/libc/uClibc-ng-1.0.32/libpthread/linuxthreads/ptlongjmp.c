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
#include <bits/stackinfo.h>
#include <jmpbuf-unwind.h>

static void pthread_cleanup_upto(__jmp_buf target)
{
  pthread_descr self = thread_self();
  struct _pthread_cleanup_buffer * c;
  char *currentframe = CURRENT_STACK_FRAME;

  for (c = THREAD_GETMEM(self, p_cleanup);
       c != NULL && _JMPBUF_UNWINDS(target, c);
       c = c->__prev)
    {
#ifdef _STACK_GROWS_DOWN
      if ((char *) c <= currentframe)
	{
	  c = NULL;
	  break;
	}
#elif defined _STACK_GROWS_UP
      if ((char *) c >= currentframe)
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
      && _JMPBUF_UNWINDS(target, THREAD_GETMEM(self, p_in_sighandler)))
    THREAD_SETMEM(self, p_in_sighandler, NULL);
}

void siglongjmp(sigjmp_buf env, int val)
{
  pthread_cleanup_upto(env->__jmpbuf);
  __libc_siglongjmp(env, val);
}

void longjmp(jmp_buf env, int val)
{
  pthread_cleanup_upto(env->__jmpbuf);
  __libc_longjmp(env, val);
}
