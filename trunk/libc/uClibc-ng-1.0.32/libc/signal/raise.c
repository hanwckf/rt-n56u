/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <signal.h>

int raise(int signo)
{
	return kill(getpid(), signo);
}
libc_hidden_def(raise)
