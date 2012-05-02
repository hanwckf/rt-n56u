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

#include <signal.h>
#include <sys/syscall.h>

/* Primitives for controlling thread execution */

static inline void restart(pthread_descr th)
{
  /* See pthread.c */
#ifdef __NR_rt_sigaction
  __pthread_restart_new(th);
#else
  __pthread_restart(th);
#endif
}

static inline void suspend(pthread_descr self)
{
  /* See pthread.c */
#ifdef __NR_rt_sigaction
  __pthread_wait_for_restart_signal(self);
#else
  __pthread_suspend(self);
#endif
}

static inline int timedsuspend(pthread_descr self,
		const struct timespec *abstime)
{
  /* See pthread.c */
#ifdef __NR_rt_sigaction
  return __pthread_timedsuspend_new(self, abstime);
#else
  return __pthread_timedsuspend(self, abstime);
#endif
}
