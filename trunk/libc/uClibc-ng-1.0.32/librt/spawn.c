/* Copyright (C) 2000, 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <alloca.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <fcntl.h>

#include <sys/resource.h>
#include <not-cancel.h>
#include <internal-signals.h>

#include <spawn.h>
#include "spawn_int.h"

/* The Unix standard contains a long explanation of the way to signal
   an error after the fork() was successful.  Since no new wait status
   was wanted there is no way to signal an error using one of the
   available methods.  The committee chose to signal an error by a
   normal program exit with the exit code 127.  */
#define SPAWN_ERROR	127

/* Execute file actions.
 * Returns true on error.
 */
inline static bool execute_file_actions(const posix_spawn_file_actions_t *fa)
{
	struct rlimit64 fdlimit;
	bool have_fdlimit = false;
	int cnt;

	for (cnt = 0; cnt < fa->__used; ++cnt) {
		struct __spawn_action *action = &fa->__actions[cnt];

		switch (action->tag) {
		case spawn_do_close:
			if (close_not_cancel(action->action.close_action.fd) != 0) {
				if (!have_fdlimit) {
					getrlimit64(RLIMIT_NOFILE, &fdlimit);
					have_fdlimit = true;
				}

				/* Only signal errors for file descriptors out of range.  */
				if (0 > action->action.close_action.fd
				    || action->action.close_action.fd >= fdlimit.rlim_cur)
					/* Signal the error.  */
					return true;
			}
			break;

		case spawn_do_open:;
			int new_fd = open_not_cancel(action->action.open_action.path,
						     action->action.open_action.oflag
						     | O_LARGEFILE,
						     action->action.open_action.mode);

			if (new_fd == -1)
				return true;

			/* Make sure the desired file descriptor is used.  */
			if (new_fd != action->action.open_action.fd) {
				if (dup2(new_fd, action->action.open_action.fd)
				    != action->action.open_action.fd)
					return true;

				if (close_not_cancel(new_fd) != 0)
					return true;
			}
			break;

		case spawn_do_dup2:
			if (dup2(action->action.dup2_action.fd,
				   action->action.dup2_action.newfd)
			    != action->action.dup2_action.newfd)
				return true;
			break;
		}
	}

	return false;
}

#define DANGEROUS (POSIX_SPAWN_SETSIGMASK		\
		   | POSIX_SPAWN_SETSIGDEF		\
		   | POSIX_SPAWN_SETSCHEDPARAM		\
		   | POSIX_SPAWN_SETSCHEDULER		\
		   | POSIX_SPAWN_SETPGROUP		\
		   | POSIX_SPAWN_RESETIDS)
inline static bool is_vfork_safe(short int flags)
{
	return ((flags & POSIX_SPAWN_USEVFORK) || !(flags & DANGEROUS));
}


/* Spawn a new process executing PATH with the attributes describes in *ATTRP.
   Before running the process perform the actions described in FILE-ACTIONS. */
static int
__spawni(pid_t *pid, const char *file,
	 const posix_spawn_file_actions_t *fa,
	 const posix_spawnattr_t *attrp, char *const argv[],
	 char *const envp[], const char *path)
{
	short int flags = attrp ? attrp->__flags : 0;

	pid_t new_pid;
	if (is_vfork_safe(flags) && !fa)
		new_pid = vfork();
	else {
#ifdef __ARCH_USE_MMU__
		new_pid = fork();
#else
		return ENOSYS;
#endif
	}

	if (new_pid) {
		if (new_pid < 0)
			return errno;

		if (pid)
			*pid = new_pid;

		return 0;
	}

	if (flags & POSIX_SPAWN_SETSIGMASK) {
		if (sigprocmask(SIG_SETMASK, &attrp->__ss, NULL) != 0)
			goto error;
	}

	if (flags & POSIX_SPAWN_SETSIGDEF) {
		/* We have to iterate over all signals.  This could possibly be
		   done better but it requires system specific solutions since
		   the sigset_t data type can be very different on different
		   architectures.  */
		struct sigaction sa;
		int sig;

		memset(&sa, 0, sizeof(sa));

		sigset_t hset;
		sigprocmask (SIG_BLOCK, 0, &hset);

		for (sig = 1; sig < _NSIG; ++sig) {
		  if ((flags & POSIX_SPAWN_SETSIGDEF)
		  && sigismember (&attrp->__sd, sig))
		  {
		    sa.sa_handler = SIG_DFL;
		  }
	          else if (sigismember (&hset, sig))
		  {
		    if (__is_internal_signal (sig))
		      sa.sa_handler = SIG_IGN;
		    else
		    {
		      __libc_sigaction (sig, 0, &sa);
		      if (sa.sa_handler == SIG_IGN)
			continue;
		      sa.sa_handler = SIG_DFL;
		    }
		  }
	        else
		  continue;

		__libc_sigaction (sig, &sa, 0);
		}
	}

	if (flags & POSIX_SPAWN_SETSCHEDULER) {
		if (sched_setscheduler(0, attrp->__policy, &attrp->__sp) == -1)
			goto error;
	} else if (flags & POSIX_SPAWN_SETSCHEDPARAM) {
		if (sched_setparam(0, &attrp->__sp) == -1)
			goto error;
	}

	if (flags & POSIX_SPAWN_SETPGROUP) {
		if (setpgid(0, attrp->__pgrp) != 0)
			goto error;
	}

	if (flags & POSIX_SPAWN_RESETIDS) {
		if (seteuid(getuid()) || setegid(getgid()))
			goto error;
	}

	if (fa && execute_file_actions(fa))
		goto error;

	if (!path || strchr(file, '/')) {
		execve(file, argv, envp);
		goto error;
	}


	char *name;
	{
		size_t filelen = strlen(file) + 1;
		size_t pathlen = strlen(path) + 1;
		name = alloca(pathlen + filelen);

		/* Copy the file name at the top. */
		name = (char *) memcpy(name + pathlen, file, filelen);

		/* And add the slash.  */
		*--name = '/';
	}

	char *p = (char *)path;
	do {
		char *startp;
		path = p;
		p = strchrnul(path, ':');

		/* Two adjacent colons, or a colon at the beginning or the end
		   of `PATH' means to search the current directory.  */
		if (p == path)
			startp = name + 1;
		else
			startp = (char *) memcpy(name - (p - path), path, p - path);

		execve(startp, argv, envp);

		switch (errno) {
		case EACCES:
		case ENOENT:
		case ESTALE:
		case ENOTDIR:
			/* Those errors indicate the file is missing or not
			   executable by us, in which case we want to just try
			   the next path directory. */
			break;
		default:
			/* Some other error means we found an executable file,
			   but something went wrong executing it; return the
			   error to our caller. */
			goto error;
		}

	} while (*p++ != '\0');

error:
	_exit(SPAWN_ERROR);
}

/* Spawn a new process executing PATH with the attributes describes in *ATTRP.
   Before running the process perform the actions described in FILE-ACTIONS. */
int posix_spawn (pid_t *pid, const char *path,
	       const posix_spawn_file_actions_t *fa,
	       const posix_spawnattr_t *attrp, char *const argv[],
	       char *const envp[])
{
	return __spawni(pid, path, fa, attrp, argv, envp, NULL);
}

/* Spawn a new process executing FILE with the attributes describes in *ATTRP.
   Before running the process perform the actions described in FILE-ACTIONS. */
int
posix_spawnp(pid_t *pid, const char *file,
	     const posix_spawn_file_actions_t *fa,
	     const posix_spawnattr_t *attrp, char *const argv[],
	     char *const envp[])
{
	const char *path = getenv("PATH");

	if (!path)
		path = ":/bin:/usr/bin";

	return __spawni(pid, file, fa, attrp, argv, envp, path);
}
