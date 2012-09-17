/*
 * Copyright (C) 2003     Manuel Novoa III <mjn3@uclibc.org>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*  Nov 6, 2003  Initial version.
 *
 *  NOTE: This implementation is quite strict about requiring all
 *    field seperators.  It also does not allow leading whitespace
 *    except when processing the numeric fields.  glibc is more
 *    lenient.  See the various glibc difference comments below.
 *
 *  TODO:
 *    Move to dynamic allocation of (currently statically allocated)
 *      buffers; especially for the group-related functions since
 *      large group member lists will cause error returns.
 *
 */

#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <paths.h>
#ifdef __UCLIBC_HAS_SHADOW__
#include <shadow.h>
#endif
#ifdef __UCLIBC_HAS_THREADS__
#include <pthread.h>
#endif

/**********************************************************************/
/* Sizes for statically allocated buffers. */

/* If you change these values, also change _SC_GETPW_R_SIZE_MAX and
 * _SC_GETGR_R_SIZE_MAX in libc/unistd/sysconf.c to match */
#define PWD_BUFFER_SIZE 256
#define GRP_BUFFER_SIZE 256

/**********************************************************************/
/* Prototypes for internal functions. */

#ifndef GETXXKEY_R_FUNC
#error GETXXKEY_R_FUNC is not defined!
#endif
/**********************************************************************/
#ifdef GETXXKEY_R_FUNC

libc_hidden_proto(GETXXKEY_R_FUNC)
int GETXXKEY_R_FUNC(DO_GETXXKEY_R_KEYTYPE key,
					GETXXKEY_R_ENTTYPE *__restrict resultbuf,
					char *__restrict buffer, size_t buflen,
					GETXXKEY_R_ENTTYPE **__restrict result)
{
	FILE *stream;
	int rv;

	*result = NULL;

	if (!(stream = fopen(DO_GETXXKEY_R_PATHNAME, "r"))) {
		rv = errno;
	} else {
		__STDIO_SET_USER_LOCKING(stream);
		do {
			if (!(rv = __pgsreader(GETXXKEY_R_PARSER, resultbuf,
								   buffer, buflen, stream))
				) {
				if (GETXXKEY_R_TEST(resultbuf)) { /* Found key? */
					*result = resultbuf;
					break;
				}
			} else {
				if (rv == ENOENT) {	/* end-of-file encountered. */
					rv = 0;
				}
				break;
			}
		} while (1);
		fclose(stream);
	}

	return rv;
}
libc_hidden_def(GETXXKEY_R_FUNC)

#endif
/**********************************************************************/
#undef GETXXKEY_R_FUNC_HIDDEN
#undef GETXXKEY_R_FUNC
#undef GETXXKEY_R_PARSER
#undef GETXXKEY_R_ENTTYPE
#undef GETXXKEY_R_TEST
#undef DO_GETXXKEY_R_KEYTYPE
#undef DO_GETXXKEY_R_PATHNAME
