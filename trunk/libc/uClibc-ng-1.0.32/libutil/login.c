#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "internal/utmp.h"

/* Write the given entry into utmp and wtmp.
 * Note: the match in utmp is done against ut_id field,
 * which is NOT set by this function - caller must set it.
 */
void login(const struct utmp *entry)
{
	struct UT copy;
	char tty_name[sizeof(copy.ut_line) + 6];
	int fd;

// Manpage:
// login() takes the argument ut struct, fills the field ut->ut_type
// (if there is such a field) with the value USER_PROCESS,
// and fills the field ut->ut_pid (if there is such a field)
// with the process ID of the calling process.
	copy = *((const struct UT *)(entry));
#if _HAVE_UT_TYPE - 0
	copy.ut_type = USER_PROCESS;
#endif
#if _HAVE_UT_PID - 0
	copy.ut_pid = getpid();
#endif

// Then it tries to fill the field ut->ut_line. It takes the first of stdin,
// stdout, stderr that is a tty, and stores the corresponding pathname minus
// a possible leading /dev/ into this field, and then writes the struct
// to the utmp file. On the other hand, if no tty name was found,
// this field is filled with "???" and the struct is not written
// to the utmp file.
	fd = 0;
	while (fd != 3 && ttyname_r(fd, tty_name, sizeof(tty_name)) != 0)
		fd++;
	if (fd != 3) {
		if (strncmp(tty_name, "/dev/", 5) == 0)
			strncpy(copy.ut_line, tty_name + 5, sizeof(copy.ut_line)-1);
		else
			strncpy(copy.ut_line, tty_name, sizeof(copy.ut_line)-1);
		copy.ut_line[sizeof(copy.ut_line)-1] = '\0';

		/* utmpname(_PATH_UTMP); - why?
		 * this makes it impossible for caller to use other file!
		 * Does any standard or historical precedent says this must be done? */
		setutent();
		/* Replaces record with matching ut_id, or appends new one: */
		pututline(&copy);
		endutent();
	} else {
		strncpy(copy.ut_line, "???", sizeof(copy.ut_line));
	}

// After this, the struct is written to the wtmp file.
	updwtmp(_PATH_WTMP, &copy);
}
