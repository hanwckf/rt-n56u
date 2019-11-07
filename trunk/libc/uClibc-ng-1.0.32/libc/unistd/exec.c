/*  Copyright (C) 2004     Manuel Novoa III
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
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

#include <stdio.h>
#include <stdlib.h>
#include <paths.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <sys/mman.h>



/**********************************************************************/
#define EXEC_FUNC_COMMON 0
#define EXEC_FUNC_EXECVP 1
#define EXEC_FUNC_EXECVPE 2

#if defined(__ARCH_USE_MMU__)

/* We have an MMU, so use alloca() to grab space for buffers and arg lists. */

# define EXEC_ALLOC_SIZE(VAR)      /* nothing to do */
# define EXEC_ALLOC(SIZE,VAR,FUNC) alloca((SIZE))
# define EXEC_FREE(PTR,VAR)        ((void)0)

#else

/* We do not have an MMU, so using alloca() is not an option (as this will
 * easily overflow the stack in most setups).  Less obviously, using malloc()
 * is not an option either since malloc()ed memory can leak in from a vfork()ed
 * child into the parent as no one is around after the child calls exec*() to
 * free() the memory.  Therefore, we must use mmap() and unmap() directly,
 * caching the result as we go.  This way we minimize the leak by reusing the
 * memory with every call to an exec*().
 *
 * To prevent recursive use of the same cached memory, we have to give execvp()
 * its own cache.  Here are the nested exec calls (a/-: alloc/no-alloc):
 *  execve(-) -> calls straight to kernel
 *  execl(a)  -> execve(-)
 *  execlp(a) -> execvp(a)	!! recursive usage !!
 *  execle(a) -> execve(-)
 *  execv(-)  -> execve(-)
 *  execvp(a) -> execve(-)
 *  execvpe(a) -> execve(-)
 */

# define EXEC_ALLOC_SIZE(VAR)      /* nothing to do */
# define EXEC_ALLOC(SIZE,VAR,FUNC) __exec_alloc((SIZE), FUNC)
# define EXEC_FREE(PTR,VAR)        ((void)0)

extern void *__exec_alloc(size_t size, int func) attribute_hidden;

# ifdef L___exec_alloc

void *__exec_alloc(size_t size, int func)
{
	static void *common_cache, *execvp_cache;
	static size_t common_size, execvp_size;

	void **cache = (func ? &execvp_cache : &common_cache);
	size_t *cache_size = (func ? &execvp_size : &common_size);

	if (*cache_size >= size)
		return *cache;
	else if (*cache)
		munmap(*cache, *cache_size);

	*cache_size = size;
	return *cache = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	/* We don't actually handle OOM in the exec funcs ...
	if (*cache != MAP_FAILED)
		return *cache;
	else
		return (*cache = NULL);
	*/
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

	p = argv = (char **) EXEC_ALLOC((n+1) * sizeof(char *), size, EXEC_FUNC_COMMON);

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
libc_hidden_def(execl)

#endif
/**********************************************************************/
#ifdef L_execv

int execv(const char *path, char *const argv[])
{
	return execve(path, argv, __environ);
}
libc_hidden_def(execv)

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

	p = argv = (char **) EXEC_ALLOC((n+1) * sizeof(char *), size, EXEC_FUNC_COMMON);

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
libc_hidden_def(execle)

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

	p = argv = (char **) EXEC_ALLOC((n+1) * sizeof(char *), size, EXEC_FUNC_COMMON);

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
libc_hidden_def(execlp)

#endif
/**********************************************************************/
#if defined (L_execvp) || defined(L_execvpe)


/* Use a default path that matches glibc behavior, since SUSv3 says
 * this is implementation-defined.  The default is current working dir,
 * /bin, and then /usr/bin. */
static const char default_path[] = ":/bin:/usr/bin";
#if defined (L_execvp)
int execvp(const char *path, char *const argv[])
#elif defined (L_execvpe)
int execvpe(const char *path, char *const argv[], char *const envp[])
#endif
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
#if defined (L_execvp)
		execve(path, argv, __environ);
#elif defined (L_execvpe)
		execve(path, argv, envp);
#endif
		if (errno == ENOEXEC) {
			char **nargv;
			EXEC_ALLOC_SIZE(size2) /* Do NOT add a semicolon! */
			size_t n;
	RUN_BIN_SH:
			/* Need the dimension - 1.  We omit counting the trailing
			 * NULL but we actually omit the first entry. */
			for (n=0 ; argv[n] ; n++) {}
#if defined (L_execvp)
			nargv = (char **) EXEC_ALLOC((n+2) * sizeof(char *), size2, EXEC_FUNC_EXECVP);
#elif defined (L_execvpe)
			nargv = (char **) EXEC_ALLOC((n+2) * sizeof(char *), size2, EXEC_FUNC_EXECVPE);
#endif
			nargv[0] = argv[0];
			nargv[1] = (char *)path;
			memcpy(nargv+2, argv+1, n*sizeof(char *));
#if defined (L_execvp)
			execve(_PATH_BSHELL, nargv, __environ);
#elif defined (L_execvpe)
			execve(_PATH_BSHELL, nargv, envp);
#endif
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
#if defined (L_execvp)
		buf = EXEC_ALLOC(FILENAME_MAX, size, EXEC_FUNC_EXECVP);
#elif defined (L_execvpe)
		buf = EXEC_ALLOC(FILENAME_MAX, size, EXEC_FUNC_EXECVPE);
#endif
		{
			int seen_small = 0;
			s0 = buf + len;
			memcpy(s0, path, plen+1);

			do {
				s = s0;
				e = strchrnul(p, ':');
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

#if defined (L_execvp)
				execve(s, argv, __environ);
#elif defined (L_execvpe)
				execve(s, argv, envp);
#endif
				seen_small = 1;

				if (errno == ENOEXEC) {
					path = s;
					goto RUN_BIN_SH;
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
#if defined (L_execvp)
libc_hidden_def(execvp)
#elif defined (L_execvpe)
libc_hidden_def(execvpe)
#endif

#endif /* #if defined (L_execvp) || defined(L_execvpe) */
/**********************************************************************/
