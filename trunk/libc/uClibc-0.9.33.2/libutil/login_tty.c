/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Make FD be the controlling terminal, stdin, stdout, and stderr;
   then close FD.  Returns 0 on success, nonzero on error.  */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <utmp.h>

libutil_hidden_proto(login_tty)
int login_tty(int fd)
{
	(void) setsid();
#ifdef TIOCSCTTY
	if (ioctl(fd, TIOCSCTTY, (char *)NULL) == -1)
		return (-1);
#else
	{
	  /* This might work.  */
	  char *fdname = ttyname (fd);
	  int newfd;
	  if (fdname)
	    {
	      if (fd != 0)
		(void) close (0);
	      if (fd != 1)
		(void) close (1);
	      if (fd != 2)
		(void) close (2);
	      newfd = open (fdname, O_RDWR);
	      (void) close (newfd);
	    }
	}
#endif
	(void) dup2(fd, 0);
	(void) dup2(fd, 1);
	(void) dup2(fd, 2);
	if (fd > 2)
		(void) close(fd);
	return (0);
}
libutil_hidden_def(login_tty)
