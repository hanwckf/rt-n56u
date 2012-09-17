/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Zack Weinberg <zack@rabi.phys.columbia.edu>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <assert.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "pty-private.h"


/* uClinux-2.0 has vfork, but Linux 2.0 doesn't */
#include <sys/syscall.h>
#if ! defined __NR_vfork
#define vfork fork
#endif

/* Return the result of ptsname_r in the buffer pointed to by PTS,
   which should be of length BUF_LEN.  If it is too long to fit in
   this buffer, a sufficiently long buffer is allocated using malloc,
   and returned in PTS.  0 is returned upon success, -1 otherwise.  */
static int
pts_name (int fd, char **pts, size_t buf_len)
{
  int rv;
  char *buf = *pts;

  for (;;)
    {
      char *new_buf;

      if (buf_len)
	{
	  rv = ptsname_r (fd, buf, buf_len);

	  if (rv != 0 || memchr (buf, '\0', buf_len))
	    /* We either got an error, or we succeeded and the
	       returned name fit in the buffer.  */
	    break;

	  /* Try again with a longer buffer.  */
	  buf_len += buf_len;	/* Double it */
	}
      else
	/* No initial buffer; start out by mallocing one.  */
	buf_len = 128;		/* First time guess.  */

      if (buf != *pts)
	/* We've already malloced another buffer at least once.  */
	new_buf = realloc (buf, buf_len);
      else
	new_buf = malloc (buf_len);
      if (! new_buf)
	{
	  rv = -1;
	  errno = ENOMEM;
	  break;
	}
      buf = new_buf;
    }

  if (rv == 0)
    *pts = buf;		/* Return buffer to the user.  */
  else if (buf != *pts)
    free (buf);		/* Free what we malloced when returning an error.  */

  return rv;
}

/* Change the ownership and access permission of the slave pseudo
   terminal associated with the master pseudo terminal specified
   by FD.  */
int
grantpt (int fd)
{
  int retval = -1;
#ifdef PATH_MAX
  char _buf[PATH_MAX];
#else
  char _buf[512];
#endif
  char *buf = _buf;
  struct stat st;
  uid_t uid;
  gid_t gid;
  pid_t pid;

  if (pts_name (fd, &buf, sizeof (_buf)))
    return -1;

  if (stat(buf, &st) < 0)
    goto cleanup;

  /* Make sure that we own the device.  */
  uid = getuid ();
  if (st.st_uid != uid)
    {
      if (chown (buf, uid, st.st_gid) < 0)
	goto helper;
    }

  gid = getgid ();

  /* Make sure the group of the device is that special group.  */
  if (st.st_gid != gid)
    {
      if (chown (buf, uid, gid) < 0)
	goto helper;
    }

  /* Make sure the permission mode is set to readable and writable by
     the owner, and writable by the group.  */
  if ((st.st_mode & ACCESSPERMS) != (S_IRUSR|S_IWUSR|S_IWGRP))
    {
      if (chmod (buf, S_IRUSR|S_IWUSR|S_IWGRP) < 0)
	goto helper;
    }

  retval = 0;
  goto cleanup;

  /* We have to use the helper program.  */
 helper:

  pid = vfork ();
  if (pid == -1)
    goto cleanup;
  else if (pid == 0)
    {
      /* Disable core dumps.  */
      struct rlimit rl = { 0, 0 };
      setrlimit (RLIMIT_CORE, &rl);

      /* We pase the master pseudo terminal as file descriptor PTY_FILENO.  */
      if (fd != PTY_FILENO)
	if (dup2 (fd, PTY_FILENO) < 0)
	  _exit (FAIL_EBADF);

      execle (_PATH_PT_CHOWN, _PATH_PT_CHOWN, NULL, NULL);
      _exit (FAIL_EXEC);
    }
  else
    {
      int w;

      if (waitpid (pid, &w, 0) == -1)
	goto cleanup;
      if (!WIFEXITED (w))
	errno = ENOEXEC;
      else
	switch (WEXITSTATUS(w))
	  {
	  case 0:
	    retval = 0;
	    break;
	  case FAIL_EBADF:
	    errno = EBADF;
	    break;
	  case FAIL_EINVAL:
	    errno = EINVAL;
	    break;
	  case FAIL_EACCES:
	    errno = EACCES;
	    break;
	  case FAIL_EXEC:
	    errno = ENOEXEC;
	    break;

	  default:
	    assert(! "getpt: internal error: invalid exit code from pt_chown");
	  }
    }

 cleanup:
  if (buf != _buf)
    free (buf);

  return retval;
}
