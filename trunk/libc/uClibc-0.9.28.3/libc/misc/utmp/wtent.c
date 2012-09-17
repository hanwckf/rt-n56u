/* wtmp support rubbish (i.e. complete crap)
 *
 * Written by Erik Andersen <andersee@debian.org> 
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Library General Public License as 
 * published by the Free Software Foundation; either version 2 of the 
 * License, or (at your option) any later version.  
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Library General Public License for more details.  
 *
 * You should have received a copy of the GNU Library General Public 
 * License along with this library; see the file COPYING.LIB.  If not, 
 * write to the Free Software Foundation, Inc., 675 Mass Ave, 
 * Cambridge, MA 02139, USA.  */

#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>
#include <fcntl.h>
#include <sys/file.h>


#if 0
/* This is enabled in uClibc/libutil/logwtmp.c */
void logwtmp (const char *line, const char *name, const char *host)
{
    struct utmp lutmp;
    memset (&(lutmp), 0, sizeof (struct utmp));

    lutmp.ut_type = (name && *name)? USER_PROCESS : DEAD_PROCESS;
    lutmp.ut_pid = getpid();
    strncpy(lutmp.ut_line, line, sizeof(lutmp.ut_line)-1);
    strncpy(lutmp.ut_name, name, sizeof(lutmp.ut_name)-1);
    strncpy(lutmp.ut_host, host, sizeof(lutmp.ut_host)-1);
    gettimeofday(&(lutmp.ut_tv), NULL);

    updwtmp(_PATH_WTMP, &(lutmp));
}
#endif

extern void updwtmp(const char *wtmp_file, const struct utmp *lutmp)
{
    int fd;

    fd = open(wtmp_file, O_APPEND | O_WRONLY, 0);
    if (fd >= 0) {
	if (lockf(fd, F_LOCK, 0)==0) {
	    write(fd, (const char *) lutmp, sizeof(struct utmp));
	    lockf(fd, F_ULOCK, 0);
	    close(fd);
	}
    }
}

