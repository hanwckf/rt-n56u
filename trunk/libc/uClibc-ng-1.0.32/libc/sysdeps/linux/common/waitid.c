/*
 * Copyright (C) 2007 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

#if defined __USE_SVID || defined __USE_XOPEN

#include <sys/syscall.h>
#include <sys/wait.h>
#include <cancel.h>
#ifndef __NR_waitid
# include <string.h>
#endif

static int __NC(waitid)(idtype_t idtype, id_t id, siginfo_t *infop, int options)
{
#ifdef __NR_waitid
	return INLINE_SYSCALL(waitid, 5, idtype, id, infop, options, NULL);
#else
	switch (idtype) {
		case P_PID:
			if (id <= 0)
				goto invalid;
			break;
		case P_PGID:
			if (id < 0 || id == 1)
				goto invalid;
			id = -id;
			break;
		case P_ALL:
			id = -1;
			break;
		default:
		invalid:
			__set_errno(EINVAL);
			return -1;
	}

	memset(infop, 0, sizeof *infop);
	infop->si_pid = __NC(waitpid)(id, &infop->si_status, options
# ifdef WEXITED
					   &~ WEXITED
# endif
					  );
	if (infop->si_pid < 0)
		return infop->si_pid;
	return 0;
#endif
}
CANCELLABLE_SYSCALL(int, waitid, (idtype_t idtype, id_t id, siginfo_t *infop, int options),
		    (idtype, id, infop, options))

#endif
