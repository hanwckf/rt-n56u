/*  Copyright (C) 2004     Manuel Novoa III
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Jan 1, 2004
 *   Initial version of a SUSv3 compliant exec*() functions.
 * Feb 17, 2004
 *   Sigh... Fall back to alloca() if munmap() is broken on uClinux.
 */

/* NOTE: Strictly speaking, there could be problems from accessing
 * __environ in multithreaded programs.  The only way around this
 * that I see is to essentially lock __environ access (modifying
 * the setenv code), make a copy of the environment table (just the
 * pointers since the strings themselves are never freed), and then
 * unlock prior to the execve call.  If that fails, then we'd need
 * to free the storage allocated for the copy.  Better ideas anyone?
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <sys/mman.h>

extern char *__strchrnul(const char *s, int c);

/**********************************************************************/
#if defined(__ARCH_HAS_MMU__) || defined(__UCLIBC_UCLINUX_BROKEN_MUNMAP__)

/* We have an MMU, so use alloca() to grab space for buffers and
 * arg lists.  Also fall back to alloca() if munmap() is broken. */

# define EXEC_ALLOC_SIZE(VAR)	/* nothing to do */
# define EXEC_ALLOC(SIZE,VAR)	alloca((SIZE))
# define EXEC_FREE(PTR,VAR)		((void)0)

#else

/* We do not have an MMU, so using alloca() is not an option.
 * Less obviously, using malloc() is not an option either since
 * malloc()ed memory can leak in a vfork() and exec*() situation.
 * Therefore, we must use mmap() and unmap() directly.
 */

# define EXEC_ALLOC_SIZE(VAR)	size_t VAR;	/* Semicolon included! */
# define EXEC_ALLOC(SIZE,VAR)	__exec_alloc((VAR = (SIZE)))
# define EXEC_FREE(PTR,VAR)		__exec_free((PTR),(VAR))

extern void *__exec_alloc(size_t size);
extern void __exec_free(void *ptr, size_t size);

# ifdef L___exec_alloc

void *__exec_alloc(size_t size)
{
	void *p;

	p = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	return (p != MAP_FAILED) ? p : NULL;
}

void __exec_free(void *ptr, size_t size)
{
	if (ptr) {
		munmap(ptr, size);
	}
}

# endif

#endif
/**********************************************************************/
#ifdef L_execl

int execl(const char *path, const char *arg, ...)
{
	EXEC_ALLOC_SIZE(size)		/* Do NOT add a semicolon! */
	int n;
	char **argv;
	char **p;
	va_list args;
	
	n = 0;
	va_start(args, arg);
	do {
		++n;
	} while (va_arg(args, char *));
	va_end(args);

	p = argv = (char **) EXEC_ALLOC((n+1) * sizeof(char *), size);

	p[0] = (char *)arg;

	va_start(args, arg);
	do {
		*++p = va_arg(args, char *);
	} while (--n);
	va_end(args);

	n = execve(path, (char *const *) argv, __environ);

	EXEC_FREE(argv, size);

	return n;
}

#endif
/**********************************************************************/
#ifdef L_execv

int execv(__const char *path, char *__const argv[])
{
	return execve(path, argv, __environ);
}

#endif
/**********************************************************************/
#ifdef L_execle

int execle(const char *path, const char *arg, ...)
{
	EXEC_ALLOC_SIZE(size)		/* Do NOT add a semicolon! */
	int n;
	char **argv;
	char **p;
	char *const *envp;
	va_list args;
	
	n = 0;
	va_start(args, arg);
	do {
		++n;
	} while (va_arg(args, char *));
	envp = va_arg(args, char *const *);	/* Varies from execl and execlp. */
	va_end(args);

	p = argv = (char **) EXEC_ALLOC((n+1) * sizeof(char *), size);

	p[0] = (char *)arg;

	va_start(args, arg);
	do {
		*++p = va_arg(args, char *);
	} while (--n);
	va_end(args);

	n = execve(path, (char *const *) argv, envp);

	EXEC_FREE(argv, size);

	return n;
}

#endif
/**********************************************************************/
#ifdef L_execlp

int execlp(const char *file, const char *arg, ...)
{
	EXEC_ALLOC_SIZE(size)		/* Do NOT add a semicolon! */
	int n;
	char **argv;
	char **p;
	va_list args;
	
	n = 0;
	va_start(args, arg);
	do {
		++n;
	} while (va_arg(args, char *));
	va_end(args);

	p = argv = (char **) EXEC_ALLOC((n+1) * sizeof(char *), size);

	p[0] = (char *)arg;

	va_start(args, arg);
	do {
		*++p = va_arg(args, char *);
	} while (--n);
	va_end(args);

	n = execvp(file, (char *const *) argv);

	EXEC_FREE(argv, size);

	return n;
}

#endif
/**********************************************************************/
#ifdef L_execvp

/* Use a default path that matches glibc behavior, since SUSv3 says
 * this is implementation-defined.  The default is current working dir,
 * /bin, and then /usr/bin. */
static const char default_path[] = ":/bin:/usr/bin";

int execvp(const char *path, char *const argv[])
{
	char *buf = NULL;
	char *p;
	char *e;
	char *s0;
	char *s;
	EXEC_ALLOC_SIZE(size = 0)	/* Do NOT add a semicolon! */
	size_t len;
	size_t plen;

	if (!path || !*path) {		/* Comply with SUSv3. */
	BAD:
		__set_errno(ENOENT);
		return -1;
	}

	if (strchr(path, '/')) {
		execve(path, argv, __environ);
	CHECK_ENOEXEC:
		if (errno == ENOEXEC) {
			char **nargv;
			EXEC_ALLOC_SIZE(size2) /* Do NOT add a semicolon! */
			size_t n;
			/* Need the dimension - 1.  We omit counting the trailing
			 * NULL but we actually omit the first entry. */
			for (n=0 ; argv[n] ; n++) {}
			nargv = (char **) EXEC_ALLOC((n+2) * sizeof(char *), size2);
			nargv[0] = argv[0];
			nargv[1] = (char *)path;
			memcpy(nargv+2, argv+1, n*sizeof(char *));
			execve("/bin/sh", nargv, __environ);
			EXEC_FREE(nargv, size2);
		}
	} else {
		if ((p = getenv("PATH")) != NULL) {
			if (!*p) {
				goto BAD;
			}
		} else {
			p = (char *) default_path;
		}

		plen = strlen(path);
		if (plen > (FILENAME_MAX - 1)) {
		ALL_TOO_LONG:
			__set_errno(ENAMETOOLONG);
			return -1;
		}
		len = (FILENAME_MAX - 1) - plen;

		if ((buf = EXEC_ALLOC(FILENAME_MAX, size)) != NULL) {
			int seen_small = 0;
			s0 = buf + len;
			memcpy(s0, path, plen+1);

			do {
				s = s0;
				e = __strchrnul(p, ':');
				if (e > p) {
					plen = e - p;
					if (e[-1] != '/') {
						++plen;
					}
					if (plen > len) {
						goto NEXT;
					}
					s -= plen;
					memcpy(s, p, plen);
					s[plen-1] = '/';
				}

				execve(s, argv, __environ);

				seen_small = 1;

				if (errno != ENOENT) {
					path = s;
					goto CHECK_ENOEXEC;
				}

			NEXT:
				if (!*e) {
					if (!seen_small) {
						goto ALL_TOO_LONG;
					}
					break;
				}
				p = e + 1;
			} while (1);
		}
	}

	EXEC_FREE(buf, size);

	return -1;
}

#endif
/**********************************************************************/
