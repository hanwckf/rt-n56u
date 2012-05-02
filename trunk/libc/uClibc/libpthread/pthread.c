/* vi: set sw=4 ts=4: */
/*
 * A _very_ simple clone based pthread-like implementation
 *
 * Copyright (C) 2001,2002 by Erik Andersen <andersee@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#define STACKSIZE 8096

#define CSIGNAL         0x000000ff      /* signal mask to be sent at exit */
#define CLONE_VM        0x00000100      /* set if VM shared between processes */
#define CLONE_FS        0x00000200      /* set if fs info shared between proces ses */
#define CLONE_FILES     0x00000400      /* set if open files shared between pro cesses */
#define CLONE_SIGHAND   0x00000800      /* set if signal handlers shared */



/* Lame home-grown clone based threading */
int pthread_mutex_init (pthread_mutex_t *mutex, const pthread_mutexattr_t *mutex_attr)
{
    mutex->__m_lock.__spinlock = 1;
	return 0;
}

int pthread_mutex_lock (pthread_mutex_t *mutex)
{
	while (mutex->__m_lock.__spinlock == 0) {
		usleep(10000);
	}
	--(mutex->__m_lock.__spinlock);
	return 0;
}

int pthread_mutex_unlock (pthread_mutex_t *mutex)
{
    ++(mutex->__m_lock.__spinlock);
	return 0;
}

int pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	++(mutex->__m_lock.__spinlock);
	while (cond->__c_lock.__spinlock == 0) {
		usleep(10000);
	}
	--(cond->__c_lock.__spinlock);
	return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    ++(cond->__c_lock.__spinlock);
	return 0;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *cond_attr)
{
    cond->__c_lock.__spinlock = 1;
	return 0;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void* (*fn)(void *), void *data)
{
	long retval;
	void **newstack;
	int (*clonefunc)(void *) = (int (*)(void *))(fn);

	newstack = (void **) malloc(STACKSIZE);
	if (!newstack)
		return -1;
	newstack = (void **) (STACKSIZE + (char *) newstack);
	*--newstack = data;
	retval = clone(clonefunc, newstack, 
			CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGCHLD, data);  
	if (retval < 0) {
		errno = -retval;
		*thread = 0;
		retval = -1;
	} else {
		*thread = retval;
		retval = 0;
	}
	return retval;
}

int pthread_join (pthread_t thread, void **thread_return)
{
	int retval;
	/* Fixme -- wait for thread and get its return value */
	retval = EXIT_SUCCESS;
	if (thread_return)
		(int)*thread_return = retval;
	_exit(retval);
}
link_warning(pthread_join, "pthread_join is a stub and does not behave properly");

void pthread_exit (void *retval)
{
	_exit(*(int *)retval);
}
