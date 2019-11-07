/*
 * wtmp support rubbish (i.e. complete crap)
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include "internal/utmp.h"

void logwtmp(const char *line, const char *name, const char *host)
{
    struct UT lutmp;
    memset(&lutmp, 0, sizeof(lutmp));

    lutmp.ut_type = (name && *name) ? USER_PROCESS : DEAD_PROCESS;
    lutmp.ut_pid = getpid();
    strncpy(lutmp.ut_line, line, sizeof(lutmp.ut_line)-1);
    strncpy(lutmp.ut_name, name, sizeof(lutmp.ut_name)-1);
    strncpy(lutmp.ut_host, host, sizeof(lutmp.ut_host)-1);
#if !defined __WORDSIZE_TIME64_COMPAT32
    gettimeofday(&lutmp.ut_tv, NULL);
#else
    {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      lutmp.ut_tv.tv_sec = tv.tv_sec;
      lutmp.ut_tv.tv_usec = tv.tv_usec;
    }
#endif

    updwtmp(_PATH_WTMP, &lutmp);
}
