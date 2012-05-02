#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <utmp.h>

/* Write the given entry into utmp and wtmp.  */
void login (const struct utmp *entry)
{
    struct utmp copy = *entry;

    utmpname(_PATH_UTMP);
    setutent();
#if _HAVE_UT_TYPE - 0 
    copy.ut_type = USER_PROCESS;
#endif  
#if _HAVE_UT_PID - 0
    copy.ut_pid = getpid();
#endif
    strncpy (copy.ut_line, entry->ut_line, UT_LINESIZE);
    pututline(entry);
    endutent();
}

