/* vi: set sw=4 ts=4: */
/*
 * select() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#ifdef __NR__newselect

extern int _newselect(int n, fd_set * readfds, fd_set * writefds,
					  fd_set * exceptfds, struct timeval *timeout);
_syscall5(int, _newselect, int, n, fd_set *, readfds, fd_set *, writefds,
		  fd_set *, exceptfds, struct timeval *, timeout);
weak_alias(_newselect, select);

#else

//Used as a fallback if _newselect isn't available...
extern int select(int n, fd_set * readfds, fd_set * writefds,
				  fd_set * exceptfds, struct timeval *timeout);
_syscall5(int, select, int, n, fd_set *, readfds, fd_set *, writefds,
		  fd_set *, exceptfds, struct timeval *, timeout);

#endif
