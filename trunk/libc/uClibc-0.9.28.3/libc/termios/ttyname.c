#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

/* Jan 1, 2004    Manuel Novoa III
 *
 * Kept the same approach, but rewrote the code for the most part.
 * Fixed some minor issues plus (as I recall) one SUSv3 errno case.
 */

/* This is a fairly slow approach.  We do a linear search through some
 * directories looking for a match.  Yes this is lame.  But it should
 * work, should be small, and will return names that match what is on
 * disk.  Another approach we could use would be to use the info in
 * /proc/self/fd, but that is even more lame since it requires /proc */

/* SUSv3 mandates TTY_NAME_MAX as 9.  This is obviously insufficient.
 * However, there is no need to waste space and support non-standard
 * tty names either.  So we compromise and use the following buffer
 * length.  (Erik and Manuel agreed that 32 was more than reasonable.)
 *
 * If you change this, also change _SC_TTY_NAME_MAX in libc/unistd/sysconf.c
 */
#define TTYNAME_BUFLEN		32

char *ttyname(int fd)
{
	static char name[TTYNAME_BUFLEN];

	return ttyname_r(fd, name, TTYNAME_BUFLEN) ? NULL : name;
}

static const char dirlist[] =
/*   12345670123 */
"\010/dev/vc/\0"	/* Try /dev/vc first (be devfs compatible) */
"\011/dev/tts/\0"	/* and /dev/tts next (be devfs compatible) */
"\011/dev/pty/\0"	/* and /dev/pty next (be devfs compatible) */
"\011/dev/pts/\0"	/* and try /dev/pts next */
"\005/dev/\0";		/* and try walking through /dev last */

int ttyname_r(int fd, char *ubuf, size_t ubuflen)
{
	struct dirent *d;
	struct stat st;
	struct stat dst;
	const char *p;
	char *s;
	DIR *fp;
	int rv;
	int len;
	char buf[TTYNAME_BUFLEN];

	if (fstat(fd, &st) < 0) {
		return errno;
	}

	rv = ENOTTY;				/* Set up the default return value. */

	if (!isatty(fd)) {
		goto DONE;
	}

	for (p = dirlist ; *p ; p += 1 + p[-1]) {
		len = *p++;

		assert(len + 2 <= TTYNAME_BUFLEN); /* dirname + 1 char + nul */

		strcpy(buf, p);
		s = buf + len;
		len =  (TTYNAME_BUFLEN-2) - len; /* Available non-nul space. */

		if (!(fp = opendir(p))) {
			continue;
		}

		while ((d = readdir(fp)) != NULL) {
			/* This should never trigger for standard names, but we
			 * check it to be safe.  */
			if (strlen(d->d_name) > len) { /* Too big? */
				continue;
			}

			strcpy(s, d->d_name);

			if ((lstat(buf, &dst) == 0)
#if 0
				/* Stupid filesystems like cramfs fail to guarantee that
				 * st_ino and st_dev uniquely identify a file, contrary to
				 * SuSv3, so we cannot be quite so precise as to require an
				 * exact match.  Settle for something less...  Grumble... */
				&& (st.st_dev == dst.st_dev) && (st.st_ino == dst.st_ino)
#else
				&& S_ISCHR(dst.st_mode) && (st.st_rdev == dst.st_rdev)
#endif
				) {				/* Found it! */
				closedir(fp);

				/* We treat NULL buf as ERANGE rather than EINVAL. */
				rv = ERANGE;
				if (ubuf && (strlen(buf) <= ubuflen)) {
					strcpy(ubuf, buf);
					rv = 0;
				}
				goto DONE;
			}
		}

		closedir(fp);
	}

 DONE:
	__set_errno(rv);

	return rv;
}
