/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <paths.h>
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <sched.h>
#include <errno.h>
#include <bits/libc-lock.h>
#include <sysdep-cancel.h>
#endif

extern __typeof(system) __libc_system;
#if !defined __UCLIBC_HAS_THREADS_NATIVE__
#include <sys/syscall.h>

int __libc_system(const char *command)
{
	int wait_val, pid;
	struct sigaction sa, save_quit, save_int;
	sigset_t save_mask;

	if (command == 0)
		return 1;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	/* __sigemptyset(&sa.sa_mask); - done by memset() */
	/* sa.sa_flags = 0; - done by memset() */

	sigaction(SIGQUIT, &sa, &save_quit);
	sigaction(SIGINT, &sa, &save_int);
	__sigaddset(&sa.sa_mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sa.sa_mask, &save_mask);

	if ((pid = vfork()) < 0) {
		wait_val = -1;
		goto out;
	}
	if (pid == 0) {
		sigaction(SIGQUIT, &save_quit, NULL);
		sigaction(SIGINT, &save_int, NULL);
		sigprocmask(SIG_SETMASK, &save_mask, NULL);

		execl(_PATH_BSHELL, "sh", "-c", command, (char *) 0);
		_exit(127);
	}

#if 0
	__printf("Waiting for child %d\n", pid);
#endif

	if (__wait4_nocancel(pid, &wait_val, 0, 0) == -1)
		wait_val = -1;

out:
	sigaction(SIGQUIT, &save_quit, NULL);
	sigaction(SIGINT, &save_int, NULL);
	sigprocmask(SIG_SETMASK, &save_mask, NULL);
	return wait_val;
}
#else
/* We have to and actually can handle cancelable system().  The big
   problem: we have to kill the child process if necessary.  To do
   this a cleanup handler has to be registered and is has to be able
   to find the PID of the child.  The main problem is to reliable have
   the PID when needed.  It is not necessary for the parent thread to
   return.  It might still be in the kernel when the cancellation
   request comes.  Therefore we have to use the clone() calls ability
   to have the kernel write the PID into the user-level variable.  */

libc_hidden_proto(sigaction)
libc_hidden_proto(waitpid)

#ifdef __ARCH_USE_MMU__
#if defined __ia64__
# define FORK() \
  INLINE_SYSCALL (clone2, 6, CLONE_PARENT_SETTID | SIGCHLD, NULL, 0, \
		  &pid, NULL, NULL)
#elif defined __sparc__
# define FORK() \
  INLINE_CLONE_SYSCALL (CLONE_PARENT_SETTID | SIGCHLD, 0, &pid, NULL, NULL)
#else
# define FORK() \
  INLINE_SYSCALL (clone, 3, CLONE_PARENT_SETTID | SIGCHLD, 0, &pid)
#endif
#else
# define FORK() \
    vfork()
#endif

static void cancel_handler (void *arg);

# define CLEANUP_HANDLER \
  __libc_cleanup_region_start (1, cancel_handler, &pid)

# define CLEANUP_RESET \
  __libc_cleanup_region_end (0)

static struct sigaction intr, quit;
static int sa_refcntr;
__libc_lock_define_initialized (static, lock);

# define DO_LOCK() __libc_lock_lock (lock)
# define DO_UNLOCK() __libc_lock_unlock (lock)
# define INIT_LOCK() ({ __libc_lock_init (lock); sa_refcntr = 0; })
# define ADD_REF() sa_refcntr++
# define SUB_REF() --sa_refcntr

/* Execute LINE as a shell command, returning its status.  */
static int
do_system (const char *line)
{
  int status, save;
  pid_t pid;
  struct sigaction sa;
  sigset_t omask;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  /*sa.sa_flags = 0; - done by memset */
  /*__sigemptyset (&sa.sa_mask); - done by memset */

  DO_LOCK ();
  if (ADD_REF () == 0)
    {
      if (sigaction (SIGINT, &sa, &intr) < 0)
	{
	  SUB_REF ();
	  goto out;
	}
      if (sigaction (SIGQUIT, &sa, &quit) < 0)
	{
	  save = errno;
	  SUB_REF ();
	  goto out_restore_sigint;
	}
    }
  DO_UNLOCK ();

  /* We reuse the bitmap in the 'sa' structure.  */
  __sigaddset (&sa.sa_mask, SIGCHLD);
  save = errno;
  if (sigprocmask (SIG_BLOCK, &sa.sa_mask, &omask) < 0)
    {
	{
	  DO_LOCK ();
	  if (SUB_REF () == 0)
	    {
	      save = errno;
	      (void) sigaction (SIGQUIT, &quit, (struct sigaction *) NULL);
	    out_restore_sigint:
	      (void) sigaction (SIGINT, &intr, (struct sigaction *) NULL);
	      __set_errno (save);
	    }
	out:
	  DO_UNLOCK ();
	  return -1;
	}
    }

  CLEANUP_HANDLER;

  pid = FORK ();
  if (pid == (pid_t) 0)
    {
      /* Child side.  */
      const char *new_argv[4];
      new_argv[0] = _PATH_BSHELL;
      new_argv[1] = "-c";
      new_argv[2] = line;
      new_argv[3] = NULL;

      /* Restore the signals.  */
      (void) sigaction (SIGINT, &intr, (struct sigaction *) NULL);
      (void) sigaction (SIGQUIT, &quit, (struct sigaction *) NULL);
      (void) sigprocmask (SIG_SETMASK, &omask, (sigset_t *) NULL);
      INIT_LOCK ();

      /* Exec the shell.  */
      (void) execve (_PATH_BSHELL, (char *const *) new_argv, __environ);
      _exit (127);
    }
  else if (pid < (pid_t) 0)
    /* The fork failed.  */
    status = -1;
  else
    /* Parent side.  */
    {
      /* Note the system() is a cancellation point.  But since we call
	 waitpid() which itself is a cancellation point we do not
	 have to do anything here.  */
      if (TEMP_FAILURE_RETRY (waitpid (pid, &status, 0)) != pid)
	status = -1;
    }

  CLEANUP_RESET;

  save = errno;
  DO_LOCK ();
  if ((SUB_REF () == 0
       && (sigaction (SIGINT, &intr, (struct sigaction *) NULL)
	   | sigaction (SIGQUIT, &quit, (struct sigaction *) NULL)) != 0)
      || sigprocmask (SIG_SETMASK, &omask, (sigset_t *) NULL) != 0)
    {
	status = -1;
    }
  DO_UNLOCK ();

  return status;
}


int
__libc_system (const char *line)
{
  if (line == NULL)
    /* Check that we have a command processor available.  It might
       not be available after a chroot(), for example.  */
    return do_system ("exit 0") == 0;

  if (SINGLE_THREAD_P)
    return do_system (line);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = do_system (line);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}


/* The cancellation handler.  */
static void
cancel_handler (void *arg)
{
  pid_t child = *(pid_t *) arg;

  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL (kill, err, 2, child, SIGKILL);

  TEMP_FAILURE_RETRY (waitpid (child, NULL, 0));

  DO_LOCK ();

  if (SUB_REF () == 0)
    {
      (void) sigaction (SIGQUIT, &quit, (struct sigaction *) NULL);
      (void) sigaction (SIGINT, &intr, (struct sigaction *) NULL);
    }

  DO_UNLOCK ();
}
#endif
#ifdef IS_IN_libc
weak_alias(__libc_system,system)
#endif
