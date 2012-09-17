/* Copyright (C) 2003, 2004, 2005, 2006, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#ifndef _PTHREAD_FUNCTIONS_H
#define _PTHREAD_FUNCTIONS_H	1

#include <pthread.h>
#include <setjmp.h>
#include <internaltypes.h>
#include <sysdep.h>

struct xid_command;

/* Data type shared with libc.  The libc uses it to pass on calls to
   the thread functions.  */
struct pthread_functions
{
  int (*ptr_pthread_attr_destroy) (pthread_attr_t *);
  int (*ptr___pthread_attr_init_2_0) (pthread_attr_t *);
  int (*ptr___pthread_attr_init_2_1) (pthread_attr_t *);
  int (*ptr_pthread_attr_getdetachstate) (const pthread_attr_t *, int *);
  int (*ptr_pthread_attr_setdetachstate) (pthread_attr_t *, int);
  int (*ptr_pthread_attr_getinheritsched) (const pthread_attr_t *, int *);
  int (*ptr_pthread_attr_setinheritsched) (pthread_attr_t *, int);
  int (*ptr_pthread_attr_getschedparam) (const pthread_attr_t *,
					 struct sched_param *);
  int (*ptr_pthread_attr_setschedparam) (pthread_attr_t *,
					 const struct sched_param *);
  int (*ptr_pthread_attr_getschedpolicy) (const pthread_attr_t *, int *);
  int (*ptr_pthread_attr_setschedpolicy) (pthread_attr_t *, int);
  int (*ptr_pthread_attr_getscope) (const pthread_attr_t *, int *);
  int (*ptr_pthread_attr_setscope) (pthread_attr_t *, int);
  int (*ptr_pthread_condattr_destroy) (pthread_condattr_t *);
  int (*ptr_pthread_condattr_init) (pthread_condattr_t *);
  int (*ptr___pthread_cond_broadcast) (pthread_cond_t *);
  int (*ptr___pthread_cond_destroy) (pthread_cond_t *);
  int (*ptr___pthread_cond_init) (pthread_cond_t *,
				  const pthread_condattr_t *);
  int (*ptr___pthread_cond_signal) (pthread_cond_t *);
  int (*ptr___pthread_cond_wait) (pthread_cond_t *, pthread_mutex_t *);
  int (*ptr___pthread_cond_timedwait) (pthread_cond_t *, pthread_mutex_t *,
				       const struct timespec *);
  int (*ptr___pthread_cond_broadcast_2_0) (pthread_cond_2_0_t *);
  int (*ptr___pthread_cond_destroy_2_0) (pthread_cond_2_0_t *);
  int (*ptr___pthread_cond_init_2_0) (pthread_cond_2_0_t *,
				      const pthread_condattr_t *);
  int (*ptr___pthread_cond_signal_2_0) (pthread_cond_2_0_t *);
  int (*ptr___pthread_cond_wait_2_0) (pthread_cond_2_0_t *, pthread_mutex_t *);
  int (*ptr___pthread_cond_timedwait_2_0) (pthread_cond_2_0_t *,
					   pthread_mutex_t *,
					   const struct timespec *);
  int (*ptr_pthread_equal) (pthread_t, pthread_t);
  void (*ptr___pthread_exit) (void *);
  int (*ptr_pthread_getschedparam) (pthread_t, int *, struct sched_param *);
  int (*ptr_pthread_setschedparam) (pthread_t, int,
				    const struct sched_param *);
  int (*ptr_pthread_mutex_destroy) (pthread_mutex_t *);
  int (*ptr_pthread_mutex_init) (pthread_mutex_t *,
				 const pthread_mutexattr_t *);
  int (*ptr_pthread_mutex_lock) (pthread_mutex_t *);
  int (*ptr_pthread_mutex_unlock) (pthread_mutex_t *);
  pthread_t (*ptr_pthread_self) (void);
  int (*ptr_pthread_setcancelstate) (int, int *);
  int (*ptr_pthread_setcanceltype) (int, int *);
  void (*ptr___pthread_cleanup_upto) (__jmp_buf, char *);
  int (*ptr___pthread_once) (pthread_once_t *, void (*) (void));
  int (*ptr___pthread_rwlock_rdlock) (pthread_rwlock_t *);
  int (*ptr___pthread_rwlock_wrlock) (pthread_rwlock_t *);
  int (*ptr___pthread_rwlock_unlock) (pthread_rwlock_t *);
  int (*ptr___pthread_key_create) (pthread_key_t *, void (*) (void *));
  void *(*ptr___pthread_getspecific) (pthread_key_t);
  int (*ptr___pthread_setspecific) (pthread_key_t, const void *);
  void (*ptr__pthread_cleanup_push_defer) (struct _pthread_cleanup_buffer *,
					   void (*) (void *), void *);
  void (*ptr__pthread_cleanup_pop_restore) (struct _pthread_cleanup_buffer *,
					    int);
#define HAVE_PTR_NTHREADS
  unsigned int *ptr_nthreads;
  void (*ptr___pthread_unwind) (__pthread_unwind_buf_t *)
       __attribute ((noreturn)) __cleanup_fct_attribute;
  void (*ptr__nptl_deallocate_tsd) (void);
  int (*ptr__nptl_setxid) (struct xid_command *);
  void (*ptr_freeres) (void);
};

/* Variable in libc.so.  */
extern struct pthread_functions __libc_pthread_functions attribute_hidden;
extern int __libc_pthread_functions_init attribute_hidden;

#if 0 /* def PTR_DEMANGLE */ /* we did not mangle, so do not demangle */
# define PTHFCT_CALL(fct, params) \
  ({ __typeof (__libc_pthread_functions.fct) __p;			      \
     __p = __libc_pthread_functions.fct;				      \
     PTR_DEMANGLE (__p);						      \
     __p params; })
#else
# define PTHFCT_CALL(fct, params) \
  __libc_pthread_functions.fct params
#endif

#endif	/* pthread-functions.h */
