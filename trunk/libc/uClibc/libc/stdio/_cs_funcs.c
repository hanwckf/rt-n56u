/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/**********************************************************************/
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
/**********************************************************************/

ssize_t _cs_read(void *cookie, char *buf, size_t bufsize)
{
	return read(*((int *) cookie), buf, bufsize);
}

/**********************************************************************/

ssize_t _cs_write(void *cookie, const char *buf, size_t bufsize)
{
	return write(*((int *) cookie), (char *) buf, bufsize);
}

/**********************************************************************/

int _cs_seek(void *cookie, register __offmax_t *pos, int whence)
{
	__offmax_t res;

#ifdef __UCLIBC_HAS_LFS__
	res = lseek64(*((int *) cookie), *pos, whence);
#else
	res = lseek(*((int *) cookie), *pos, whence);
#endif

	return (res >= 0) ? ((*pos = res), 0) : ((int) res);
}

/**********************************************************************/

int _cs_close(void *cookie)
{
	return close(*((int *) cookie));
}

/**********************************************************************/
#else
/**********************************************************************/

int __stdio_seek(FILE *stream, register __offmax_t *pos, int whence)
{
	__offmax_t res;

#ifdef __UCLIBC_HAS_LFS__
	res = lseek64(stream->__filedes, *pos, whence);
#else
	res = lseek(stream->__filedes, *pos, whence);
#endif

	return (res >= 0) ? ((*pos = res), 0) : ((int) res);
}

/**********************************************************************/
#endif
/**********************************************************************/
