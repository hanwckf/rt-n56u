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
#include <malloc.h>
#include <assert.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <paths.h>
#ifdef __UCLIBC_HAS_SHADOW__
#include <shadow.h>
#endif

/**********************************************************************/
/* Prototypes for internal functions. */

#if !defined(GETXXKEY_R_FUNC) && !defined(GETXXKEY_FUNC)
#error GETXXKEY_R_FUNC/GETXXKEY_FUNC are not defined!
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

#endif /* GETXXKEY_R_FUNC */

/**********************************************************************/
#ifdef GETXXKEY_FUNC

#define REENTRANT_NAME APPEND_R(GETXXKEY_FUNC)
#define APPEND_R(name) APPEND_R1(name)
#define APPEND_R1(name) name##_r

GETXXKEY_ENTTYPE *GETXXKEY_FUNC(GETXXKEY_ADD_PARAMS)
{
	static char *buffer = NULL;
	static GETXXKEY_ENTTYPE resultbuf;
	GETXXKEY_ENTTYPE *result;

	if (buffer == NULL)
		buffer = (char *)__uc_malloc(GETXXKEY_BUFLEN);

# ifdef GETXXKEY_ADD_VARIABLES
	REENTRANT_NAME(GETXXKEY_ADD_VARIABLES, &resultbuf, buffer, GETXXKEY_BUFLEN, &result);
# else
	REENTRANT_NAME(&resultbuf, buffer, GETXXKEY_BUFLEN, &result);
# endif
	return result;
}
#ifdef GETXXKEY_FUNC_HIDDEN
libc_hidden_def(GETXXKEY_FUNC)
#endif

#undef REENTRANT_NAME
#undef APPEND_R
#undef APPEND_R1
#endif /* GETXXKEY_FUNC */

/**********************************************************************/
#undef GETXXKEY_FUNC
#undef GETXXKEY_ENTTYPE
#undef GETXXKEY_BUFLEN
#undef GETXXKEY_FUNC_HIDDEN
#undef GETXXKEY_ADD_PARAMS
#undef GETXXKEY_ADD_VARIABLES
#undef GETXXKEY_R_FUNC
#undef GETXXKEY_R_PARSER
#undef GETXXKEY_R_ENTTYPE
#undef GETXXKEY_R_TEST
#undef DO_GETXXKEY_R_KEYTYPE
#undef DO_GETXXKEY_R_PATHNAME
