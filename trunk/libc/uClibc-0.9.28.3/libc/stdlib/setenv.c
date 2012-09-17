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

#define _GNU_SOURCE
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
   free the strings.  */
int __add_to_environ (const char *name, const char *value, 
	const char *combined, int replace)
{
    register char **ep;
    register size_t size;
    const size_t namelen = strlen (name);
    const size_t vallen = value != NULL ? strlen (value) + 1 : 0;
    int rv = -1;

    __UCLIBC_MUTEX_LOCK(mylock);

    /* We have to get the pointer now that we have the lock and not earlier
       since another thread might have created a new environment.  */
    ep = __environ;

    size = 0;
    if (ep != NULL) {
	for (; *ep != NULL; ++ep) {
	    if (!strncmp (*ep, name, namelen) && (*ep)[namelen] == '=')
		break;
	    else
		++size;
	}
    }

    if (ep == NULL || *ep == NULL) {
	char **new_environ;

	/* We allocated this space; we can extend it.  */
	new_environ = (char **) realloc (last_environ,
		(size + 2) * sizeof (char *));
	if (new_environ == NULL) {
			goto DONE;
	}

	/* If the whole entry is given add it.  */
	if (combined != NULL) {
	    /* We must not add the string to the search tree since it belongs
	       to the user.  */
	    new_environ[size] = (char *) combined;
	} else {
	    /* See whether the value is already known.  */
	    new_environ[size] = (char *) malloc (namelen + 1 + vallen);
	    if (new_environ[size] == NULL) {
		__set_errno (ENOMEM);
				goto DONE;
	    }

	    memcpy (new_environ[size], name, namelen);
	    new_environ[size][namelen] = '=';
	    memcpy (&new_environ[size][namelen + 1], value, vallen);
	}

	if (__environ != last_environ) {
	    memcpy ((char *) new_environ, (char *) __environ,
		    size * sizeof (char *));
	}

	new_environ[size + 1] = NULL;
	last_environ = __environ = new_environ;
    } else if (replace) {
	char *np;

	/* Use the user string if given.  */
	if (combined != NULL) {
	    np = (char *) combined;
	} else {
	    np = malloc (namelen + 1 + vallen);
	    if (np == NULL) {
				goto DONE;
	    }
	    memcpy (np, name, namelen);
	    np[namelen] = '=';
	    memcpy (&np[namelen + 1], value, vallen);
	}
	*ep = np;
    }

    rv = 0;

 DONE:
    __UCLIBC_MUTEX_UNLOCK(mylock);
    return rv;
}

int setenv (const char *name, const char *value, int replace)
{
    return __add_to_environ (name, value, NULL, replace);
}

int unsetenv (const char *name)
{
    size_t len;
    char **ep;

    if (name == NULL || *name == '\0' || strchr (name, '=') != NULL) {
	__set_errno (EINVAL);
	return -1;
    }

    len = strlen (name);
    __UCLIBC_MUTEX_LOCK(mylock);
    ep = __environ;
    while (*ep != NULL) {
	if (!strncmp (*ep, name, len) && (*ep)[len] == '=') {
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

/* The `clearenv' was planned to be added to POSIX.1 but probably
   never made it.  Nevertheless the POSIX.9 standard (POSIX bindings
   for Fortran 77) requires this function.  */
int clearenv (void)
{
    __UCLIBC_MUTEX_LOCK(mylock);
    if (__environ == last_environ && __environ != NULL) {
	/* We allocated this environment so we can free it.  */
	free (__environ);
	last_environ = NULL;
    }
    /* Clear the environment pointer removes the whole environment.  */
    __environ = NULL;
    __UCLIBC_MUTEX_UNLOCK(mylock);
    return 0;
}

/* Put STRING, which is of the form "NAME=VALUE", in the environment.  */
int putenv (char *string)
{
    int result;
    const char *const name_end = strchr (string, '=');

    if (name_end != NULL) {
	char *name = strndup(string, name_end - string);
	result = __add_to_environ (name, NULL, string, 1);
	free(name);
	return(result);
    }
    unsetenv (string);
    return 0;
}

