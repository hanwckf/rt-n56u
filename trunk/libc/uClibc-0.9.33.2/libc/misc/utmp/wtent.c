/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* wtmp support rubbish (i.e. complete crap) */

#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>
#include <fcntl.h>
#include <sys/file.h>
#include <not-cancel.h>

#if 0
/* This is enabled in uClibc/libutil/logwtmp.c */
void logwtmp (const char *line, const char *name, const char *host)
{
    struct utmp lutmp;
    memset(&lutmp, 0, sizeof(lutmp));

    lutmp.ut_type = (name && *name) ? USER_PROCESS : DEAD_PROCESS;
    lutmp.ut_pid = getpid();
    strncpy(lutmp.ut_line, line, sizeof(lutmp.ut_line)-1);
    strncpy(lutmp.ut_name, name, sizeof(lutmp.ut_name)-1);
    strncpy(lutmp.ut_host, host, sizeof(lutmp.ut_host)-1);
    gettimeofday(&(lutmp.ut_tv), NULL);

    updwtmp(_PATH_WTMP, &lutmp);
}
#endif

void updwtmp(const char *wtmp_file, const struct utmp *lutmp)
{
    int fd;

    fd = open_not_cancel(wtmp_file, O_APPEND | O_WRONLY, 0);
    if (fd >= 0) {
	if (lockf(fd, F_LOCK, 0) == 0) {
	    write_not_cancel(fd, lutmp, sizeof(struct utmp));
	    lockf(fd, F_ULOCK, 0);
	    close_not_cancel_no_status(fd);
	}
    }
}
libc_hidden_def(updwtmp)
