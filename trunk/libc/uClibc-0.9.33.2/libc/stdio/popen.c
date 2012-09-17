/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

/* Jan 1, 2004
 *
 * Rewrite popen for SUSv3 compliance.
 *   Added a list of popen()'d to store pids and use waitpid() in pclose().
 *   Loop on waitpid() failure due to EINTR as required.
 *   Close parent's popen()'d FILEs in the {v}fork()'d child.
 *   Fix failure exit code for failed execve().
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <bits/uClibc_mutex.h>

#ifdef __UCLIBC_MJN3_ONLY__
#warning "hmm... susv3 says Pipe streams are byte-oriented."
#endif /* __UCLIBC_MJN3_ONLY__ */


/* uClinux-2.0 has vfork, but Linux 2.0 doesn't */
#include <sys/syscall.h>
#if ! defined __NR_vfork
# define vfork fork
# define VFORK_LOCK		((void) 0)
# define VFORK_UNLOCK		((void) 0)
#endif

#ifndef VFORK_LOCK
__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_MUTEX_INITIALIZER);
# define VFORK_LOCK		__UCLIBC_MUTEX_LOCK(mylock)
# define VFORK_UNLOCK		__UCLIBC_MUTEX_UNLOCK(mylock)
#endif

struct popen_list_item {
	struct popen_list_item *next;
	FILE *f;
	pid_t pid;
};

static struct popen_list_item *popen_list /* = NULL (bss initialized) */;

FILE *popen(const char *command, const char *modes)
{
	FILE *fp;
	struct popen_list_item *pi;
	struct popen_list_item *po;
	int pipe_fd[2];
	int parent_fd;
	int child_fd;
	int child_writing;			/* Doubles as the desired child fildes. */
	pid_t pid;

	child_writing = 0;			/* Assume child is writing. */
	if (modes[0] != 'w') {		/* Parent not writing... */
		++child_writing;		/* so child must be writing. */
		if (modes[0] != 'r') {	/* Oops!  Parent not reading either! */
			__set_errno(EINVAL);
			goto RET_NULL;
		}
	}

	if (!(pi = malloc(sizeof(struct popen_list_item)))) {
		goto RET_NULL;
	}

	if (pipe(pipe_fd)) {
		goto FREE_PI;
	}

	child_fd = pipe_fd[child_writing];
	parent_fd = pipe_fd[1-child_writing];

	if (!(fp = fdopen(parent_fd, modes))) {
		close(parent_fd);
		close(child_fd);
		goto FREE_PI;
	}

	VFORK_LOCK;
	if ((pid = vfork()) == 0) {	/* Child of vfork... */
		close(parent_fd);
		if (child_fd != child_writing) {
			dup2(child_fd, child_writing);
			close(child_fd);
		}

		/* SUSv3 requires that any previously popen()'d streams in the
		 * parent shall be closed in the child. */
		for (po = popen_list ; po ; po = po->next) {
			close(po->f->__filedes);
		}

		execl("/bin/sh", "sh", "-c", command, (char *)0);

		/* SUSv3 mandates an exit code of 127 for the child if the
		 * command interpreter can not be invoked. */
		_exit(127);
	}
	VFORK_UNLOCK;

	/* We need to close the child filedes whether vfork failed or
	 * it succeeded and we're in the parent. */
	close(child_fd);

	if (pid > 0) {				/* Parent of vfork... */
		pi->pid = pid;
		pi->f = fp;
		VFORK_LOCK;
		pi->next = popen_list;
		popen_list = pi;
		VFORK_UNLOCK;

		return fp;
	}

	/* If we get here, vfork failed. */
	fclose(fp);					/* Will close parent_fd. */

 FREE_PI:
	free(pi);

 RET_NULL:
	return NULL;
}

#warning is pclose correct wrt the new mutex semantics?

int pclose(FILE *stream)
{
	struct popen_list_item *p;
	int stat;
	pid_t pid;

	/* First, find the list entry corresponding to stream and remove it
	 * from the list.  Set p to the list item (NULL if not found). */
	VFORK_LOCK;
	if ((p = popen_list) != NULL) {
		if (p->f == stream) {
			popen_list = p->next;
		} else {
			struct popen_list_item *t;
			do {
				t = p;
				if (!(p = t->next)) {
					__set_errno(EINVAL); /* Not required by SUSv3. */
					break;
				}
				if (p->f == stream) {
					t->next = p->next;
					break;
				}
			} while (1);
		}
	}
	VFORK_UNLOCK;

	if (p) {
		pid = p->pid;			/* Save the pid we need */
		free(p);				/* and free the list item. */

		fclose(stream);	/* The SUSv3 example code ignores the return. */

		/* SUSv3 specificly requires that pclose not return before the child
		 * terminates, in order to disallow pclose from returning on EINTR. */
		do {
			if (waitpid(pid, &stat, 0) >= 0) {
				return stat;
			}
			if (errno != EINTR) {
				break;
			}
		} while (1);
	}

	return -1;
}
