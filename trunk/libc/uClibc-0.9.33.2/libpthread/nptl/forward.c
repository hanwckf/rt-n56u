/* Copyright (C) 2002, 2003, 2004, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <dlfcn.h>
#include <pthreadP.h>
#include <signal.h>
#include <stdlib.h>

#include <atomic.h>
#include <sysdep.h>


/* Pointers to the libc functions.  */
struct pthread_functions __libc_pthread_functions attribute_hidden;
int __libc_pthread_functions_init attribute_hidden;


#define FORWARD2(name, rettype, decl, params, defaction) \
rettype									      \
name decl								      \
{									      \
  if (!__libc_pthread_functions_init) {					      \
    defaction;								      \
  } else {								      \
    return PTHFCT_CALL (ptr_##name, params);				      \
  }									      \
}

#define FORWARD(name, decl, params, defretval) \
  FORWARD2 (name, int, decl, params, return defretval)


FORWARD (pthread_attr_destroy, (pthread_attr_t *attr), (attr), 0)

FORWARD (__pthread_attr_init_2_1, (pthread_attr_t *attr), (attr), 0)
weak_alias(__pthread_attr_init_2_1, pthread_attr_init)

FORWARD (pthread_attr_getdetachstate,
	 (const pthread_attr_t *attr, int *detachstate), (attr, detachstate),
	 0)
FORWARD (pthread_attr_setdetachstate, (pthread_attr_t *attr, int detachstate),
	 (attr, detachstate), 0)

FORWARD (pthread_attr_getinheritsched,
	 (const pthread_attr_t *attr, int *inherit), (attr, inherit), 0)
FORWARD (pthread_attr_setinheritsched, (pthread_attr_t *attr, int inherit),
	 (attr, inherit), 0)

FORWARD (pthread_attr_getschedparam,
	 (const pthread_attr_t *attr, struct sched_param *param),
	 (attr, param), 0)
FORWARD (pthread_attr_setschedparam,
	 (pthread_attr_t *attr, const struct sched_param *param),
	 (attr, param), 0)

FORWARD (pthread_attr_getschedpolicy,
	 (const pthread_attr_t *attr, int *policy), (attr, policy), 0)
FORWARD (pthread_attr_setschedpolicy, (pthread_attr_t *attr, int policy),
	 (attr, policy), 0)

FORWARD (pthread_attr_getscope,
	 (const pthread_attr_t *attr, int *scope), (attr, scope), 0)
FORWARD (pthread_attr_setscope, (pthread_attr_t *attr, int scope),
	 (attr, scope), 0)


FORWARD (pthread_condattr_destroy, (pthread_condattr_t *attr), (attr), 0)
FORWARD (pthread_condattr_init, (pthread_condattr_t *attr), (attr), 0)

FORWARD (__pthread_cond_broadcast, (pthread_cond_t *cond), (cond), 0)
weak_alias(__pthread_cond_broadcast, pthread_cond_broadcast)

FORWARD (__pthread_cond_destroy, (pthread_cond_t *cond), (cond), 0)
weak_alias(__pthread_cond_destroy, pthread_cond_destroy)

FORWARD (__pthread_cond_init,
	 (pthread_cond_t *cond, const pthread_condattr_t *cond_attr),
	 (cond, cond_attr), 0)
weak_alias(__pthread_cond_init, pthread_cond_init)

FORWARD (__pthread_cond_signal, (pthread_cond_t *cond), (cond), 0)
weak_alias(__pthread_cond_signal, pthread_cond_signal)

FORWARD (__pthread_cond_wait, (pthread_cond_t *cond, pthread_mutex_t *mutex),
	 (cond, mutex), 0)
weak_alias(__pthread_cond_wait, pthread_cond_wait)

FORWARD (__pthread_cond_timedwait,
	 (pthread_cond_t *cond, pthread_mutex_t *mutex,
	  const struct timespec *abstime), (cond, mutex, abstime), 0)
weak_alias(__pthread_cond_timedwait, pthread_cond_timedwait)


FORWARD (pthread_equal, (pthread_t thread1, pthread_t thread2),
	 (thread1, thread2), 1)


/* Use an alias to avoid warning, as pthread_exit is declared noreturn.  */
FORWARD2 (__pthread_exit, void, (void *retval), (retval), exit (EXIT_SUCCESS))
strong_alias (__pthread_exit, pthread_exit);


FORWARD (pthread_getschedparam,
	 (pthread_t target_thread, int *policy, struct sched_param *param),
	 (target_thread, policy, param), 0)
FORWARD (pthread_setschedparam,
	 (pthread_t target_thread, int policy,
	  const struct sched_param *param), (target_thread, policy, param), 0)


FORWARD (pthread_mutex_destroy, (pthread_mutex_t *mutex), (mutex), 0)

FORWARD (pthread_mutex_init,
	 (pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr),
	 (mutex, mutexattr), 0)

FORWARD (pthread_mutex_lock, (pthread_mutex_t *mutex), (mutex), 0)
weak_alias (pthread_mutex_lock, __pthread_mutex_lock)

FORWARD (pthread_mutex_unlock, (pthread_mutex_t *mutex), (mutex), 0)
weak_alias (pthread_mutex_unlock, __pthread_mutex_unlock)

FORWARD2 (pthread_self, pthread_t, (void), (), return 0)


FORWARD (pthread_setcancelstate, (int state, int *oldstate), (state, oldstate),
	 0)

FORWARD (pthread_setcanceltype, (int type, int *oldtype), (type, oldtype), 0)

#define return /* value is void */
FORWARD2(_pthread_cleanup_push_defer,
	 void, (struct _pthread_cleanup_buffer *buffer, void (*routine)(void *), void *arg),
	 (buffer, routine, arg),
	 { buffer->__routine = routine; buffer->__arg = arg; });

FORWARD2(_pthread_cleanup_pop_restore,
	 void, (struct _pthread_cleanup_buffer *buffer, int execute),
	 (buffer, execute),
	 if (execute) { buffer->__routine(buffer->__arg); });

FORWARD2(__pthread_unwind,
	 void attribute_hidden __attribute ((noreturn)) __cleanup_fct_attribute,
	 (__pthread_unwind_buf_t *buf), (buf), {
		       /* We cannot call abort() here.  */
		       INTERNAL_SYSCALL_DECL (err);
		       INTERNAL_SYSCALL (kill, err, 1, SIGKILL);
		     })
#undef return
