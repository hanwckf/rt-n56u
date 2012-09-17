/* Copyright (C) 1992,95,96,97,98,99,2000,2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.

   modified for uClibc by Erik Andersen <andersen@codepoet.org>
*/

#include <features.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include <bits/uClibc_mutex.h>
__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_MUTEX_INITIALIZER);


/* If this variable is not a null pointer we allocated the current
   environment.  */
static char **last_environ;


/* This function is used by `setenv' and `putenv'.  The difference between
   the two functions is that for the former must create a new string which
   is then placed in the environment, while the argument of `putenv'
   must be used directly.  This is all complicated by the fact that we try
   to reuse values once generated for a `setenv' call since we can never
   free the strings. [in uclibc, we do not]  */
static int __add_to_environ(const char *name, const char *value,
 		int replace)
{
	register char **ep;
	register size_t size;
	char *var_val;
	char **new_environ;
	/* name may come from putenv() and thus may contain "=VAL" part */
	const size_t namelen = strchrnul(name, '=') - name;
	int rv = -1;

	__UCLIBC_MUTEX_LOCK(mylock);

	/* We have to get the pointer now that we have the lock and not earlier
	   since another thread might have created a new environment.  */
	ep = __environ;

	size = 0;
	if (ep != NULL) {
		while (*ep != NULL) {
			if (!strncmp(*ep, name, namelen) && (*ep)[namelen] == '=') {
				/* Found */
				if (!replace)
					goto DONE_OK;
				goto REPLACE;
			}
			++size;
			++ep;
		}
	}

	/* Not found, add at the end */

	/* We allocated this space; we can extend it.  */
	new_environ = realloc(last_environ, (size + 2) * sizeof(char *));
	if (new_environ == NULL) {
		__set_errno(ENOMEM);
		goto DONE;
	}
	if (__environ != last_environ) {
		memcpy(new_environ, __environ, size * sizeof(char *));
	}
	last_environ = __environ = new_environ;

	ep = &new_environ[size];
	/* Ensure env is NULL terminated in case malloc below fails */
	ep[0] = NULL;
	ep[1] = NULL;

 REPLACE:
	var_val = (char*) name;
	/* Build VAR=VAL if we called by setenv, not putenv.  */
	if (value != NULL) {
		const size_t vallen = strlen(value) + 1;

		var_val = malloc(namelen + 1 + vallen);
		if (var_val == NULL) {
			__set_errno(ENOMEM);
			goto DONE;
		}
		memcpy(var_val, name, namelen);
		var_val[namelen] = '=';
		memcpy(&var_val[namelen + 1], value, vallen);
	}
	*ep = var_val;

 DONE_OK:
	rv = 0;

 DONE:
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return rv;
}

int setenv(const char *name, const char *value, int replace)
{
	/* NB: setenv("VAR", NULL, 1) inserts "VAR=" string */
	return __add_to_environ(name, value ? value : "", replace);
}
libc_hidden_def(setenv)

int unsetenv(const char *name)
{
	const char *eq;
	size_t len;
	char **ep;

	if (name == NULL || *name == '\0'
	 || *(eq = strchrnul(name, '=')) == '='
	) {
		__set_errno(EINVAL);
		return -1;
	}
	len = eq - name; /* avoiding strlen this way */

	__UCLIBC_MUTEX_LOCK(mylock);
	ep = __environ;
	/* NB: clearenv(); unsetenv("foo"); should not segfault */
	if (ep)	while (*ep != NULL) {
		if (!strncmp(*ep, name, len) && (*ep)[len] == '=') {
			/* Found it.  Remove this pointer by moving later ones back.  */
			char **dp = ep;
			do {
				dp[0] = dp[1];
			} while (*dp++);
			/* Continue the loop in case NAME appears again.  */
		} else {
			++ep;
		}
	}
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return 0;
}
libc_hidden_def(unsetenv)

/* The `clearenv' was planned to be added to POSIX.1 but probably
   never made it.  Nevertheless the POSIX.9 standard (POSIX bindings
   for Fortran 77) requires this function.  */
int clearenv(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	/* If we allocated this environment we can free it.
	 * If we did not allocate this environment, it's NULL already
	 * and is safe to free().  */
	free(last_environ);
	last_environ = NULL;
	/* Clearing environ removes the whole environment.  */
	__environ = NULL;
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return 0;
}

/* Put STRING, which is of the form "NAME=VALUE", in the environment.  */
int putenv(char *string)
{
	if (strchr(string, '=') != NULL) {
		return __add_to_environ(string, NULL, 1);
	}
	return unsetenv(string);
}
