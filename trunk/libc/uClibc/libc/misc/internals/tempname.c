/* Copyright (C) 1991,92,93,94,95,96,97,98,99 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* March 11, 2002       Manuel Novoa III
 *
 * Modify code to remove dependency on libgcc long long arith support funcs.
 */

/* June 6, 2004       Erik Andersen
 *
 * Don't use brain damaged getpid() based randomness.
 */

/* April 15, 2005     Mike Frysinger
 *
 * Use brain damaged getpid() if real random fails.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "tempname.h"


/* Return nonzero if DIR is an existent directory.  */
static int direxists (const char *dir)
{
    struct stat buf;
    return stat(dir, &buf) == 0 && S_ISDIR (buf.st_mode);
}

/* Path search algorithm, for tmpnam, tmpfile, etc.  If DIR is
   non-null and exists, uses it; otherwise uses the first of $TMPDIR,
   P_tmpdir, /tmp that exists.  Copies into TMPL a template suitable
   for use with mk[s]temp.  Will fail (-1) if DIR is non-null and
   doesn't exist, none of the searched dirs exists, or there's not
   enough space in TMPL. */
int __path_search (char *tmpl, size_t tmpl_len, const char *dir,
	const char *pfx, int try_tmpdir)
{
    //const char *d;
    size_t dlen, plen;

    if (!pfx || !pfx[0])
    {
	pfx = "file";
	plen = 4;
    }
    else
    {
	plen = strlen (pfx);
	if (plen > 5)
	    plen = 5;
    }

    /* Disable support for $TMPDIR */
#if 0
    if (try_tmpdir)
    {
	d = __secure_getenv ("TMPDIR");
	if (d != NULL && direxists (d))
	    dir = d;
	else if (dir != NULL && direxists (dir))
	    /* nothing */ ;
	else
	    dir = NULL;
    }
#endif
    if (dir == NULL)
    {
	if (direxists (P_tmpdir))
	    dir = P_tmpdir;
	else if (strcmp (P_tmpdir, "/tmp") != 0 && direxists ("/tmp"))
	    dir = "/tmp";
	else
	{
	    __set_errno (ENOENT);
	    return -1;
	}
    }

    dlen = strlen (dir);
    while (dlen > 1 && dir[dlen - 1] == '/')
	dlen--;			/* remove trailing slashes */

    /* check we have room for "${dir}/${pfx}XXXXXX\0" */
    if (tmpl_len < dlen + 1 + plen + 6 + 1)
    {
	__set_errno (EINVAL);
	return -1;
    }

    sprintf (tmpl, "%.*s/%.*sXXXXXX", (int) dlen, dir, (int) plen, pfx);
    return 0;
}

/* These are the characters used in temporary filenames.  */
static const char letters[] =
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static unsigned int fillrand(unsigned char *buf, unsigned int len)
{
    int fd;
    unsigned int result = -1;
    fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
	fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    }
    if (fd >= 0) {
	result = read(fd, buf, len);
	close(fd);
    }
    return result;
}

static void brain_damaged_fillrand(unsigned char *buf, unsigned int len)
{
	int i, k;
	struct timeval tv;
	uint32_t high, low, rh;
	static uint64_t value;
	gettimeofday(&tv, NULL);
	value += ((uint64_t) tv.tv_usec << 16) ^ tv.tv_sec ^ getpid();
	low = value & UINT32_MAX;
	high = value >> 32;
	for (i = 0; i < len; ++i) {
		rh = high % 62;
		high /= 62;
#define L ((UINT32_MAX % 62 + 1) % 62)
		k = (low % 62) + (L * rh);
#undef L
#define H ((UINT32_MAX / 62) + ((UINT32_MAX % 62 + 1) / 62))
		low = (low / 62) + (H * rh) + (k / 62);
#undef H
		k %= 62;
		buf[i] = letters[k];
	}
}

/* Generate a temporary file name based on TMPL.  TMPL must match the
   rules for mk[s]temp (i.e. end in "XXXXXX").  The name constructed
   does not exist at the time of the call to __gen_tempname.  TMPL is
   overwritten with the result.

   KIND may be one of:
   __GT_NOCREATE:       simply verify that the name does not exist
                        at the time of the call.
   __GT_FILE:           create the file using open(O_CREAT|O_EXCL)
                        and return a read-write fd.  The file is mode 0600.
   __GT_BIGFILE:        same as __GT_FILE but use open64().
   __GT_DIR:            create a directory, which will be mode 0700.

*/
int __gen_tempname (char *tmpl, int kind)
{
    char *XXXXXX;
    unsigned int k;
    int len, i, count, fd, save_errno = errno;
    unsigned char randomness[6];

    len = strlen (tmpl);
    if (len < 6 || strcmp (&tmpl[len - 6], "XXXXXX"))
    {
	__set_errno (EINVAL);
	return -1;
    }

    /* This is where the Xs start.  */
    XXXXXX = &tmpl[len - 6];

    /* Get some random data.  */
	if (fillrand(randomness, sizeof(randomness)) != sizeof(randomness)) {
		/* if random device nodes failed us, lets use the braindamaged ver */
		brain_damaged_fillrand(randomness, sizeof(randomness));
    }
    for (i = 0 ; i < sizeof(randomness) ; i++) {
	k = ((randomness[i]) % 62);
	XXXXXX[i] = letters[k];
    }

    for (count = 0; count < TMP_MAX; ++count)
    {
	switch(kind) {
	    case __GT_NOCREATE:
		{
		    struct stat st;
		    if (stat (tmpl, &st) < 0)
		    {
			if (errno == ENOENT)
			{
			    __set_errno (save_errno);
			    return 0;
			}
			else
			    /* Give up now. */
			    return -1;
		    }
		    else
			continue;
		}
	    case __GT_FILE:
		fd = open (tmpl, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
		break;
#if defined __UCLIBC_HAS_LFS__
	    case __GT_BIGFILE:
		fd = open64 (tmpl, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
		break;
#endif
	    case __GT_DIR:
		fd = mkdir (tmpl, S_IRUSR | S_IWUSR | S_IXUSR);
		break;
	    default:
		fd = -1;
		assert (! "invalid KIND in __gen_tempname");
	}

	if (fd >= 0) {
	    __set_errno (save_errno);
	    return fd;
	}
	else if (errno != EEXIST)
	    /* Any other error will apply also to other names we might
	       try, and there are 2^32 or so of them, so give up now. */
	    return -1;
    }

    /* We got out of the loop because we ran out of combinations to try.  */
    __set_errno (EEXIST);
    return -1;
}
