/*
 * Copyright (C) 2006 Rich Felker <dalias@aerifal.cx>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

#ifdef __UCLIBC_HAS_LFS__
# define BUILD_GLOB64
#endif

#include <glob.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>

#include <unistd.h>
#include <stdio.h>


struct match
{
	struct match *next;
	char name[1];
};

#ifdef BUILD_GLOB64
extern int __glob_is_literal(const char *p, int useesc) attribute_hidden;
extern int __glob_append(struct match **tail, const char *name, size_t len, int mark) attribute_hidden;
extern int __glob_ignore_err(const char *path, int err) attribute_hidden;
extern void __glob_freelist(struct match *head) attribute_hidden;
extern int __glob_sort(const void *a, const void *b) attribute_hidden;
extern int __glob_match_in_dir(const char *d, const char *p, int flags, int (*errfunc)(const char *path, int err), struct match **tail) attribute_hidden;
#endif

#ifdef __UCLIBC_HAS_LFS__
# define stat stat64
# define readdir_r readdir64_r
# define dirent dirent64
# define struct_stat struct stat64
#else
# define struct_stat struct stat
#endif

/* keep only one copy of these */
#ifndef __GLOB64

# ifndef BUILD_GLOB64
static
# endif
int __glob_is_literal(const char *p, int useesc)
{
	int bracket = 0;
	for (; *p; p++) {
		switch (*p) {
		case '\\':
			if (!useesc) break;
		case '?':
		case '*':
			return 0;
		case '[':
			bracket = 1;
			break;
		case ']':
			if (bracket) return 0;
			break;
		}
	}
	return 1;
}

# ifndef BUILD_GLOB64
static
# endif
int __glob_append(struct match **tail, const char *name, size_t len, int mark)
{
	struct match *new = malloc(sizeof(struct match) + len + 1);
	if (!new) return -1;
	(*tail)->next = new;
	new->next = NULL;
	strcpy(new->name, name);
	if (mark) strcat(new->name, "/");
	*tail = new;
	return 0;
}

# ifndef BUILD_GLOB64
static
# endif
int __glob_match_in_dir(const char *d, const char *p, int flags, int (*errfunc)(const char *path, int err), struct match **tail)
{
	DIR *dir;
	long long de_buf[(sizeof(struct dirent) + NAME_MAX + sizeof(long long))/sizeof(long long)];
	struct dirent *de;
	char pat[strlen(p)+1];
	char *p2;
	size_t l = strlen(d);
	int literal;
	int fnm_flags= ((flags & GLOB_NOESCAPE) ? FNM_NOESCAPE : 0) | FNM_PERIOD;
	int error;

	if ((p2 = strchr(p, '/'))) {
		strcpy(pat, p);
		pat[p2-p] = 0;
		for (; *p2 == '/'; p2++);
		p = pat;
	}
	literal = __glob_is_literal(p, !(flags & GLOB_NOESCAPE));
	if (*d == '/' && !*(d+1)) l = 0;

	/* rely on opendir failing for nondirectory objects */
	dir = opendir(*d ? d : ".");
	error = errno;
	if (!dir) {
		/* this is not an error -- we let opendir call stat for us */
		if (error == ENOTDIR) return 0;
		if (error == EACCES && !*p) {
			struct_stat st;
			if (!stat(d, &st) && S_ISDIR(st.st_mode)) {
				if (__glob_append(tail, d, l, l))
					return GLOB_NOSPACE;
				return 0;
			}
		}
		if (errfunc(d, error) || (flags & GLOB_ERR))
			return GLOB_ABORTED;
		return 0;
	}
	if (!*p) {
		error = __glob_append(tail, d, l, l) ? GLOB_NOSPACE : 0;
		closedir(dir);
		return error;
	}
	while (!(error = readdir_r(dir, (void *)de_buf, &de)) && de) {
		char namebuf[l+de->d_reclen+2], *name = namebuf;
		if (!literal && fnmatch(p, de->d_name, fnm_flags))
			continue;
		if (literal && strcmp(p, de->d_name))
			continue;
		if (p2 && de->d_type && !S_ISDIR(de->d_type<<12) && !S_ISLNK(de->d_type<<12))
			continue;
		if (*d) {
			memcpy(name, d, l);
			name[l] = '/';
			strcpy(name+l+1, de->d_name);
		} else {
			name = de->d_name;
		}
		if (p2) {
			if ((error = __glob_match_in_dir(name, p2, flags, errfunc, tail))) {
				closedir(dir);
				return error;
			}
		} else {
			int mark = 0;
			if (flags & GLOB_MARK) {
				if (de->d_type)
					mark = S_ISDIR(de->d_type<<12);
				else {
					struct_stat st;
					stat(name, &st);
					mark = S_ISDIR(st.st_mode);
				}
			}
			if (__glob_append(tail, name, l+de->d_reclen+1, mark)) {
				closedir(dir);
				return GLOB_NOSPACE;
			}
		}
	}
	closedir(dir);
	if (error && (errfunc(d, error) || (flags & GLOB_ERR)))
		return GLOB_ABORTED;
	return 0;
}

# ifndef BUILD_GLOB64
static
# endif
int __glob_ignore_err(const char * path attribute_unused,
			int err attribute_unused)
{
	return 0;
}

# ifndef BUILD_GLOB64
static
# endif
void __glob_freelist(struct match *head)
{
	struct match *match, *next;
	for (match=head->next; match; match=next) {
		next = match->next;
		free(match);
	}
}

# ifndef BUILD_GLOB64
static
# endif
int __glob_sort(const void *a, const void *b)
{
	return strcmp(*(const char **)a, *(const char **)b);
}
#endif /* !__GLOB64 */

int glob(const char *pat, int flags, int (*errfunc)(const char *path, int err), glob_t *g)
{
	const char *p=pat, *d;
	struct match head = { .next = NULL }, *tail = &head;
	size_t cnt, i;
	size_t offs = (flags & GLOB_DOOFFS) ? g->gl_offs : 0;
	int error = 0;

	if (*p == '/') {
		for (; *p == '/'; p++);
		d = "/";
	} else {
		d = "";
	}

	if (!errfunc) errfunc = __glob_ignore_err;

	if (!(flags & GLOB_APPEND)) {
		g->gl_offs = offs;
		g->gl_pathc = 0;
		g->gl_pathv = NULL;
	}

	if (*p) error = __glob_match_in_dir(d, p, flags, errfunc, &tail);
	if (error == GLOB_NOSPACE) {
		__glob_freelist(&head);
		return error;
	}

	for (cnt=0, tail=head.next; tail; tail=tail->next, cnt++);
	if (!cnt) {
		if (flags & GLOB_NOCHECK) {
			tail = &head;
			if (__glob_append(&tail, pat, strlen(pat), 0))
				return GLOB_NOSPACE;
			cnt++;
		} else
			return GLOB_NOMATCH;
	}

	if (flags & GLOB_APPEND) {
		char **pathv = realloc(g->gl_pathv, (offs + g->gl_pathc + cnt + 1) * sizeof(char *));
		if (!pathv) {
			__glob_freelist(&head);
			return GLOB_NOSPACE;
		}
		g->gl_pathv = pathv;
		offs += g->gl_pathc;
	} else {
		g->gl_pathv = malloc((offs + cnt + 1) * sizeof(char *));
		if (!g->gl_pathv) {
			__glob_freelist(&head);
			return GLOB_NOSPACE;
		}
		for (i=0; i<offs; i++)
			g->gl_pathv[i] = NULL;
	}
	for (i=0, tail=head.next; i<cnt; tail=tail->next, i++)
		g->gl_pathv[offs + i] = tail->name;
	g->gl_pathv[offs + i] = NULL;
	g->gl_pathc += cnt;

	if (!(flags & GLOB_NOSORT))
		qsort(g->gl_pathv+offs, cnt, sizeof(char *), __glob_sort);

	return error;
}
#ifdef __GLOB64
libc_hidden_def(glob64)
#else
libc_hidden_def(glob)
#endif

void globfree(glob_t *g)
{
	size_t i;
	for (i=0; i<g->gl_pathc; i++)
		free(g->gl_pathv[g->gl_offs + i] - offsetof(struct match, name));
	free(g->gl_pathv);
	g->gl_pathc = 0;
	g->gl_pathv = NULL;
}
#ifdef __GLOB64
libc_hidden_def(globfree64)
#else
libc_hidden_def(globfree)
#endif
