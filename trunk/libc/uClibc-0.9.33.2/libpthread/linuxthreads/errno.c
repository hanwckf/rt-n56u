/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
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

/* Define the location of errno for the remainder of the C library */

#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include "pthread.h"
#include "internals.h"

#if !USE_TLS || !HAVE___THREAD
/* The definition in libc is sufficient if we use TLS.  */
int *
__errno_location (void)
{
  pthread_descr self = thread_self();
  return THREAD_GETMEM (self, p_errnop);
}

int *
__h_errno_location (void)
{
  pthread_descr self = thread_self();
  return THREAD_GETMEM (self, p_h_errnop);
}

#if 0
/* Return thread specific resolver state.  */
struct __res_state *
__res_state (void)
{
  pthread_descr self = thread_self();
  return THREAD_GETMEM (self, p_resp);
}
#endif
#endif
