/*  Copyright (C) 2003     Manuel Novoa III
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

/*  Nov 6, 2003  Initial version.
 *
 *  NOTE: This implementation is quite strict about requiring all
 *    field seperators.  It also does not allow leading whitespace
 *    except when processing the numeric fields.  glibc is more
 *    lenient.  See the various glibc difference comments below.
 *
 *  TODO:
 *    Move to dynamic allocation of (currently staticly allocated)
 *      buffers; especially for the group-related functions since
 *      large group member lists will cause error returns.
 *
 */

#define _GNU_SOURCE
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
#include <shadow.h>

#include <bits/uClibc_mutex.h>

/**********************************************************************/
/* Sizes for staticly allocated buffers. */

/* If you change these values, also change _SC_GETPW_R_SIZE_MAX and
 * _SC_GETGR_R_SIZE_MAX in libc/unistd/sysconf.c to match */
#define PWD_BUFFER_SIZE 256
#define GRP_BUFFER_SIZE 256

/**********************************************************************/
/* Prototypes for internal functions. */

extern int __parsepwent(void *pw, char *line);
extern int __parsegrent(void *gr, char *line);
extern int __parsespent(void *sp, char *line);

extern int __pgsreader(int (*__parserfunc)(void *d, char *line), void *data,
					   char *__restrict line_buff, size_t buflen, FILE *f);

/**********************************************************************/
/* For the various fget??ent_r funcs, return
 *
 *  0: success
 *  ENOENT: end-of-file encountered
 *  ERANGE: buflen too small
 *  other error values possible. See __pgsreader.
 *
 * Also, *result == resultbuf on success and NULL on failure.
 *
 * NOTE: glibc difference - For the ENOENT case, glibc also sets errno.
 *   We do not, as it really isn't an error if we reach the end-of-file.
 *   Doing so is analogous to having fgetc() set errno on EOF.
 */
/**********************************************************************/
#ifdef L_fgetpwent_r

int fgetpwent_r(FILE *__restrict stream, struct passwd *__restrict resultbuf,
				char *__restrict buffer, size_t buflen,
				struct passwd **__restrict result)
{
	int rv;

	*result = NULL;

	if (!(rv = __pgsreader(__parsepwent, resultbuf, buffer, buflen, stream))) {
		*result = resultbuf;
	}

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_fgetgrent_r

int fgetgrent_r(FILE *__restrict stream, struct group *__restrict resultbuf,
				char *__restrict buffer, size_t buflen,
				struct group **__restrict result)
{
	int rv;

	*result = NULL;

	if (!(rv = __pgsreader(__parsegrent, resultbuf, buffer, buflen, stream))) {
		*result = resultbuf;
	}

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_fgetspent_r

int fgetspent_r(FILE *__restrict stream, struct spwd *__restrict resultbuf,
				char *__restrict buffer, size_t buflen,
				struct spwd **__restrict result)
{
	int rv;

	*result = NULL;

	if (!(rv = __pgsreader(__parsespent, resultbuf, buffer, buflen, stream))) {
		*result = resultbuf;
	}

	return rv;
}

#endif
/**********************************************************************/
/* For the various fget??ent funcs, return NULL on failure and a
 * pointer to the appropriate struct (staticly allocated) on success.
 */
/**********************************************************************/
#ifdef L_fgetpwent

struct passwd *fgetpwent(FILE *stream)
{
	static char buffer[PWD_BUFFER_SIZE];
	static struct passwd resultbuf;
	struct passwd *result;

	fgetpwent_r(stream, &resultbuf, buffer, sizeof(buffer), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_fgetgrent

struct group *fgetgrent(FILE *stream)
{
	static char buffer[GRP_BUFFER_SIZE];
	static struct group resultbuf;
	struct group *result;

	fgetgrent_r(stream, &resultbuf, buffer, sizeof(buffer), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_fgetspent

struct spwd *fgetspent(FILE *stream)
{
	static char buffer[PWD_BUFFER_SIZE];
	static struct spwd resultbuf;
	struct spwd *result;

	fgetspent_r(stream, &resultbuf, buffer, sizeof(buffer), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_sgetspent_r

int sgetspent_r(const char *string, struct spwd *result_buf,
				char *buffer, size_t buflen, struct spwd **result)
{
	int rv = ERANGE;

	*result = NULL;

	if (buflen < PWD_BUFFER_SIZE) {
	DO_ERANGE:
		__set_errno(rv);
		goto DONE;
	}

	if (string != buffer) {
		if (strlen(string) >= buflen) {
			goto DO_ERANGE;
		}
		strcpy(buffer, string);
	}

	if (!(rv = __parsespent(result_buf, buffer))) {
		*result = result_buf;
	}

 DONE:
	return rv;
}

#endif
/**********************************************************************/

#ifdef GETXXKEY_R_FUNC
#error GETXXKEY_R_FUNC is already defined!
#endif

#ifdef L_getpwnam_r
#define GETXXKEY_R_FUNC			getpwnam_r
#define GETXXKEY_R_PARSER   	__parsepwent
#define GETXXKEY_R_ENTTYPE		struct passwd
#define GETXXKEY_R_TEST(ENT)	(!strcmp((ENT)->pw_name, key))
#define DO_GETXXKEY_R_KEYTYPE	const char *__restrict
#define DO_GETXXKEY_R_PATHNAME  _PATH_PASSWD
#endif

#ifdef L_getgrnam_r
#define GETXXKEY_R_FUNC			getgrnam_r
#define GETXXKEY_R_PARSER   	__parsegrent
#define GETXXKEY_R_ENTTYPE		struct group
#define GETXXKEY_R_TEST(ENT)	(!strcmp((ENT)->gr_name, key))
#define DO_GETXXKEY_R_KEYTYPE	const char *__restrict
#define DO_GETXXKEY_R_PATHNAME  _PATH_GROUP
#endif

#ifdef L_getspnam_r
#define GETXXKEY_R_FUNC			getspnam_r
#define GETXXKEY_R_PARSER   	__parsespent
#define GETXXKEY_R_ENTTYPE		struct spwd
#define GETXXKEY_R_TEST(ENT)	(!strcmp((ENT)->sp_namp, key))
#define DO_GETXXKEY_R_KEYTYPE	const char *__restrict
#define DO_GETXXKEY_R_PATHNAME  _PATH_SHADOW
#endif

#ifdef L_getpwuid_r
#define GETXXKEY_R_FUNC			getpwuid_r
#define GETXXKEY_R_PARSER   	__parsepwent
#define GETXXKEY_R_ENTTYPE		struct passwd
#define GETXXKEY_R_TEST(ENT)	((ENT)->pw_uid == key)
#define DO_GETXXKEY_R_KEYTYPE	uid_t
#define DO_GETXXKEY_R_PATHNAME  _PATH_PASSWD
#endif

#ifdef L_getgrgid_r
#define GETXXKEY_R_FUNC			getgrgid_r
#define GETXXKEY_R_PARSER   	__parsegrent
#define GETXXKEY_R_ENTTYPE		struct group
#define GETXXKEY_R_TEST(ENT)	((ENT)->gr_gid == key)
#define DO_GETXXKEY_R_KEYTYPE	gid_t
#define DO_GETXXKEY_R_PATHNAME  _PATH_GROUP
#endif

/**********************************************************************/
#ifdef GETXXKEY_R_FUNC

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

#endif
/**********************************************************************/
#ifdef L_getpwuid

struct passwd *getpwuid(uid_t uid)
{
	static char buffer[PWD_BUFFER_SIZE];
	static struct passwd resultbuf;
	struct passwd *result;

	getpwuid_r(uid, &resultbuf, buffer, sizeof(buffer), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_getgrgid

struct group *getgrgid(gid_t gid)
{
	static char buffer[GRP_BUFFER_SIZE];
	static struct group resultbuf;
	struct group *result;

	getgrgid_r(gid, &resultbuf, buffer, sizeof(buffer), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_getspuid_r

/* This function is non-standard and is currently not built.  It seems
 * to have been created as a reentrant version of the non-standard
 * functions getspuid.  Why getspuid was added, I do not know. */

int getspuid_r(uid_t uid, struct spwd *__restrict resultbuf,
		       char *__restrict buffer, size_t buflen,
		       struct spwd **__restrict result)
{
	int rv;
	struct passwd *pp;
	struct passwd password;
	char pwd_buff[PWD_BUFFER_SIZE];

	*result = NULL;
	if (!(rv = getpwuid_r(uid, &password, pwd_buff, sizeof(pwd_buff), &pp))) {
		rv = getspnam_r(password.pw_name, resultbuf, buffer, buflen, result);
	}

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_getspuid

/* This function is non-standard and is currently not built.
 * Why it was added, I do not know. */

struct spwd *getspuid(uid_t uid)
{
	static char buffer[PWD_BUFFER_SIZE];
	static struct spwd resultbuf;
	struct spwd *result;

	getspuid_r(uid, &resultbuf, buffer, sizeof(buffer), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_getpwnam

struct passwd *getpwnam(const char *name)
{
	static char buffer[PWD_BUFFER_SIZE];
	static struct passwd resultbuf;
	struct passwd *result;

	getpwnam_r(name, &resultbuf, buffer, sizeof(buffer), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_getgrnam

struct group *getgrnam(const char *name)
{
	static char buffer[GRP_BUFFER_SIZE];
	static struct group resultbuf;
	struct group *result;

	getgrnam_r(name, &resultbuf, buffer, sizeof(buffer), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_getspnam

struct spwd *getspnam(const char *name)
{
	static char buffer[PWD_BUFFER_SIZE];
	static struct spwd resultbuf;
	struct spwd *result;

	getspnam_r(name, &resultbuf, buffer, sizeof(buffer), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_getpw

int getpw(uid_t uid, char *buf)
{
	struct passwd resultbuf;
	struct passwd *result;
	char buffer[PWD_BUFFER_SIZE];

	if (!buf) {
		__set_errno(EINVAL);
	} else if (!getpwuid_r(uid, &resultbuf, buffer, sizeof(buffer), &result)) {
		if (sprintf(buf, "%s:%s:%lu:%lu:%s:%s:%s\n",
					resultbuf.pw_name, resultbuf.pw_passwd,
					(unsigned long)(resultbuf.pw_uid),
					(unsigned long)(resultbuf.pw_gid),
					resultbuf.pw_gecos, resultbuf.pw_dir,
					resultbuf.pw_shell) >= 0
			) {
			return 0;
		}
	}

	return -1;
}

#endif
/**********************************************************************/
#ifdef L_getpwent_r

__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_MUTEX_INITIALIZER);

static FILE *pwf /*= NULL*/;

void setpwent(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (pwf) {
		rewind(pwf);
	}
	__UCLIBC_MUTEX_UNLOCK(mylock);
}

void endpwent(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (pwf) {
		fclose(pwf);
		pwf = NULL;
	}
	__UCLIBC_MUTEX_UNLOCK(mylock);
}


int getpwent_r(struct passwd *__restrict resultbuf, 
			   char *__restrict buffer, size_t buflen,
			   struct passwd **__restrict result)
{
	int rv;

	__UCLIBC_MUTEX_LOCK(mylock);

	*result = NULL;				/* In case of error... */

	if (!pwf) {
		if (!(pwf = fopen(_PATH_PASSWD, "r"))) {
			rv = errno;
			goto ERR;
		}
		__STDIO_SET_USER_LOCKING(pwf);
	}

	if (!(rv = __pgsreader(__parsepwent, resultbuf,
						   buffer, buflen, pwf))) {
		*result = resultbuf;
	}

 ERR:
	__UCLIBC_MUTEX_UNLOCK(mylock);

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_getgrent_r

__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_MUTEX_INITIALIZER);

static FILE *grf /*= NULL*/;

void setgrent(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (grf) {
		rewind(grf);
	}
	__UCLIBC_MUTEX_UNLOCK(mylock);
}

void endgrent(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (grf) {
		fclose(grf);
		grf = NULL;
	}
	__UCLIBC_MUTEX_UNLOCK(mylock);
}

int getgrent_r(struct group *__restrict resultbuf,
			   char *__restrict buffer, size_t buflen,
			   struct group **__restrict result)
{
	int rv;

	__UCLIBC_MUTEX_LOCK(mylock);

	*result = NULL;				/* In case of error... */

	if (!grf) {
		if (!(grf = fopen(_PATH_GROUP, "r"))) {
			rv = errno;
			goto ERR;
		}
		__STDIO_SET_USER_LOCKING(grf);
	}

	if (!(rv = __pgsreader(__parsegrent, resultbuf,
						   buffer, buflen, grf))) {
		*result = resultbuf;
	}

 ERR:
	__UCLIBC_MUTEX_UNLOCK(mylock);

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_getspent_r

__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_MUTEX_INITIALIZER);

static FILE *spf /*= NULL*/;

void setspent(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (spf) {
		rewind(spf);
	}
	__UCLIBC_MUTEX_UNLOCK(mylock);
}

void endspent(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (spf) {
		fclose(spf);
		spf = NULL;
	}
	__UCLIBC_MUTEX_UNLOCK(mylock);
}

int getspent_r(struct spwd *resultbuf, char *buffer, 
			   size_t buflen, struct spwd **result)
{
	int rv;

	__UCLIBC_MUTEX_LOCK(mylock);

	*result = NULL;				/* In case of error... */

	if (!spf) {
		if (!(spf = fopen(_PATH_SHADOW, "r"))) {
			rv = errno;
			goto ERR;
		}
		__STDIO_SET_USER_LOCKING(spf);
	}

	if (!(rv = __pgsreader(__parsespent, resultbuf,
						   buffer, buflen, spf))) {
		*result = resultbuf;
	}

 ERR:
	__UCLIBC_MUTEX_UNLOCK(mylock);

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_getpwent

struct passwd *getpwent(void)
{
	static char line_buff[PWD_BUFFER_SIZE];
	static struct passwd pwd;
	struct passwd *result;

	getpwent_r(&pwd, line_buff, sizeof(line_buff), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_getgrent

struct group *getgrent(void)
{
	static char line_buff[GRP_BUFFER_SIZE];
	static struct group gr;
	struct group *result;

	getgrent_r(&gr, line_buff, sizeof(line_buff), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_getspent

struct spwd *getspent(void)
{
	static char line_buff[PWD_BUFFER_SIZE];
	static struct spwd spwd;
	struct spwd *result;

	getspent_r(&spwd, line_buff, sizeof(line_buff), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_sgetspent

struct spwd *sgetspent(const char *string)
{
	static char line_buff[PWD_BUFFER_SIZE];
	static struct spwd spwd;
	struct spwd *result;

	sgetspent_r(string, &spwd, line_buff, sizeof(line_buff), &result);
	return result;
}

#endif
/**********************************************************************/
#ifdef L_initgroups

int initgroups(const char *user, gid_t gid)
{
	FILE *grf;
	gid_t *group_list;
	int num_groups, rv;
	char **m;
	struct group group;
	char buff[PWD_BUFFER_SIZE];

	rv = -1;

	/* We alloc space for 8 gids at a time. */
	if (((group_list = (gid_t *) malloc(8*sizeof(gid_t *))) != NULL)
		&& ((grf = fopen(_PATH_GROUP, "r")) != NULL)
		) {

		__STDIO_SET_USER_LOCKING(grf);

		*group_list = gid;
		num_groups = 1;

		while (!__pgsreader(__parsegrent, &group, buff, sizeof(buff), grf)) {
			assert(group.gr_mem); /* Must have at least a NULL terminator. */
			if (group.gr_gid != gid) {
				for (m=group.gr_mem ; *m ; m++) {
					if (!strcmp(*m, user)) {
						if (!(num_groups & 7)) {
							gid_t *tmp = (gid_t *)
								realloc(group_list,
										(num_groups+8) * sizeof(gid_t *));
							if (!tmp) {
								rv = -1;
								goto DO_CLOSE;
							}
							group_list = tmp;
						}
						group_list[num_groups++] = group.gr_gid;
						break;
					}
				}
			}
		}

		rv = setgroups(num_groups, group_list);
	DO_CLOSE:
		fclose(grf);
	}

	/* group_list will be NULL if initial malloc failed, which may trigger
	 * warnings from various malloc debuggers. */
	free(group_list);
	return rv;
}

#endif
/**********************************************************************/
#ifdef L_putpwent

int putpwent(const struct passwd *__restrict p, FILE *__restrict f)
{
	int rv = -1;

	if (!p || !f) {
		__set_errno(EINVAL);
	} else {
		/* No extra thread locking is needed above what fprintf does. */
		if (fprintf(f, "%s:%s:%lu:%lu:%s:%s:%s\n",
					p->pw_name, p->pw_passwd,
					(unsigned long)(p->pw_uid),
					(unsigned long)(p->pw_gid),
					p->pw_gecos, p->pw_dir, p->pw_shell) >= 0
			) {
			rv = 0;
		}
	}

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_putgrent

int putgrent(const struct group *__restrict p, FILE *__restrict f)
{
	static const char format[] = ",%s";
	char **m;
	const char *fmt;
	int rv = -1;
	__STDIO_AUTO_THREADLOCK_VAR;

	if (!p || !f) {				/* Sigh... glibc checks. */
		__set_errno(EINVAL);
	} else {
		__STDIO_AUTO_THREADLOCK(f);

		if (fprintf(f, "%s:%s:%lu:",
					p->gr_name, p->gr_passwd,
					(unsigned long)(p->gr_gid)) >= 0
			) {

			fmt = format + 1;

			assert(p->gr_mem);
			m = p->gr_mem;

			do {
				if (!*m) {
					if (fputc_unlocked('\n', f) >= 0) {
						rv = 0;
					}
					break;
				}
				if (fprintf(f, fmt, *m) < 0) {
					break;
				}
				++m;
				fmt = format;
			} while (1);

		}

		__STDIO_AUTO_THREADUNLOCK(f);
	}

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_putspent

static const unsigned char sp_off[] = {
	offsetof(struct spwd, sp_lstchg),	/* 2 - not a char ptr */
	offsetof(struct spwd, sp_min), 		/* 3 - not a char ptr */
	offsetof(struct spwd, sp_max),		/* 4 - not a char ptr */
	offsetof(struct spwd, sp_warn), 	/* 5 - not a char ptr */
	offsetof(struct spwd, sp_inact), 	/* 6 - not a char ptr */
	offsetof(struct spwd, sp_expire), 	/* 7 - not a char ptr */
};

int putspent(const struct spwd *p, FILE *stream)
{
	static const char ld_format[] = "%ld:";
	const char *f;
	long int x;
	int i;
	int rv = -1;
	__STDIO_AUTO_THREADLOCK_VAR;

	/* Unlike putpwent and putgrent, glibc does not check the args. */

	__STDIO_AUTO_THREADLOCK(stream);

	if (fprintf(stream, "%s:%s:", p->sp_namp,
				(p->sp_pwdp ? p->sp_pwdp : "")) < 0
		) {
		goto DO_UNLOCK;
	}

	for (i=0 ; i < sizeof(sp_off) ; i++) {
		f = ld_format;
		if ((x = *(const long int *)(((const char *) p) + sp_off[i])) == -1) {
			f += 3;
		}
		if (fprintf(stream, f, x) < 0) {
			goto DO_UNLOCK;
		}
	}

	if ((p->sp_flag != ~0UL) && (fprintf(stream, "%lu", p->sp_flag) < 0)) {
		goto DO_UNLOCK;
	}

	if (fputc_unlocked('\n', stream) > 0) {
		rv = 0;
	}

 DO_UNLOCK:
	__STDIO_AUTO_THREADUNLOCK(stream);

	return rv;
}

#endif
/**********************************************************************/
/* Internal uClibc functions.                                         */
/**********************************************************************/
#ifdef L___parsepwent

static const unsigned char pw_off[] = {
	offsetof(struct passwd, pw_name), 	/* 0 */
	offsetof(struct passwd, pw_passwd),	/* 1 */
	offsetof(struct passwd, pw_uid),	/* 2 - not a char ptr */
	offsetof(struct passwd, pw_gid), 	/* 3 - not a char ptr */
	offsetof(struct passwd, pw_gecos),	/* 4 */
	offsetof(struct passwd, pw_dir), 	/* 5 */
	offsetof(struct passwd, pw_shell) 	/* 6 */
};

int __parsepwent(void *data, char *line)
{
	char *endptr;
	char *p;
	int i;

	i = 0;
	do {
		p = ((char *) ((struct passwd *) data)) + pw_off[i];

		if ((i & 6) ^ 2) { 	/* i!=2 and i!=3 */
			*((char **) p) = line;
			if (i==6) {
				return 0;
			}
			/* NOTE: glibc difference - glibc allows omission of
			 * ':' seperators after the gid field if all remaining
			 * entries are empty.  We require all separators. */
			if (!(line = strchr(line, ':'))) {
				break;
			}
		} else {
			unsigned long t = strtoul(line, &endptr, 10);
			/* Make sure we had at least one digit, and that the
			 * failing char is the next field seperator ':'.  See
			 * glibc difference note above. */
			/* TODO: Also check for leading whitespace? */
			if ((endptr == line) || (*endptr != ':')) {
				break;
			}
			line = endptr;
			if (i & 1) {		/* i == 3 -- gid */
				*((gid_t *) p) = t;
			} else {			/* i == 2 -- uid */
				*((uid_t *) p) = t;
			}
		}

		*line++ = 0;
		++i;
	} while (1);

	return -1;
}

#endif
/**********************************************************************/
#ifdef L___parsegrent

static const unsigned char gr_off[] = {
	offsetof(struct group, gr_name), 	/* 0 */
	offsetof(struct group, gr_passwd),	/* 1 */
	offsetof(struct group, gr_gid)		/* 2 - not a char ptr */
};

int __parsegrent(void *data, char *line)
{
	char *endptr;
	char *p;
	int i;
	char **members;
	char *end_of_buf;

	end_of_buf = ((struct group *) data)->gr_name; /* Evil hack! */
	i = 0;
	do {
		p = ((char *) ((struct group *) data)) + gr_off[i];

		if (i < 2) {
			*((char **) p) = line;
			if (!(line = strchr(line, ':'))) {
				break;
			}
			*line++ = 0;
			++i;
		} else {
			*((gid_t *) p) = strtoul(line, &endptr, 10);

			/* NOTE: glibc difference - glibc allows omission of the
			 * trailing colon when there is no member list.  We treat
			 * this as an error. */

			/* Make sure we had at least one digit, and that the
			 * failing char is the next field seperator ':'.  See
			 * glibc difference note above. */
			if ((endptr == line) || (*endptr != ':')) {
				break;
			}

			i = 1;				/* Count terminating NULL ptr. */
			p = endptr;

			if (p[1]) { /* We have a member list to process. */
				/* Overwrite the last ':' with a ',' before counting.
				 * This allows us to test for initial ',' and adds
				 * one ',' so that the ',' count equals the member
				 * count. */
				*p = ',';
				do {
					/* NOTE: glibc difference - glibc allows and trims leading
					 * (but not trailing) space.  We treat this as an error. */
					/* NOTE: glibc difference - glibc allows consecutive and
					 * trailing commas, and ignores "empty string" users.  We
					 * treat this as an error. */
					if (*p == ',') {
						++i;
						*p = 0;	/* nul-terminate each member string. */
						if (!*++p || (*p == ',') || isspace(*p)) {
							goto ERR;
						}
					}
				} while (*++p);
			}

			/* Now align (p+1), rounding up. */
			/* Assumes sizeof(char **) is a power of 2. */
			members = (char **)( (((intptr_t) p) + sizeof(char **))
								 & ~((intptr_t)(sizeof(char **) - 1)) );

			if (((char *)(members + i)) > end_of_buf) {	/* No space. */
				break;
			}

			((struct group *) data)->gr_mem = members;

			if (--i) {
				p = endptr;	/* Pointing to char prior to first member. */
				do {
					*members++ = ++p;
					if (!--i) break;
					while (*++p) {}
				} while (1);
			}				
			*members = NULL;

			return 0;
		}
	} while (1);

 ERR:
	return -1;
}

#endif
/**********************************************************************/
#ifdef L___parsespent

static const unsigned char sp_off[] = {
	offsetof(struct spwd, sp_namp),		/* 0 */
	offsetof(struct spwd, sp_pwdp),		/* 1 */
	offsetof(struct spwd, sp_lstchg),	/* 2 - not a char ptr */
	offsetof(struct spwd, sp_min), 		/* 3 - not a char ptr */
	offsetof(struct spwd, sp_max),		/* 4 - not a char ptr */
	offsetof(struct spwd, sp_warn), 	/* 5 - not a char ptr */
	offsetof(struct spwd, sp_inact), 	/* 6 - not a char ptr */
	offsetof(struct spwd, sp_expire), 	/* 7 - not a char ptr */
	offsetof(struct spwd, sp_flag) 		/* 8 - not a char ptr */
};

int __parsespent(void *data, char * line)
{
	char *endptr;
	char *p;
	int i;

	i = 0;
	do {
		p = ((char *) ((struct spwd *) data)) + sp_off[i];
		if (i < 2) {
			*((char **) p) = line;
			if (!(line = strchr(line, ':'))) {
				break;
			}
		} else {
#if 0
			if (i==5) {			/* Support for old format. */
				while (isspace(*line)) ++line; /* glibc eats space here. */
				if (!*line) {
					((struct spwd *) data)->sp_warn = -1;
					((struct spwd *) data)->sp_inact = -1;
					((struct spwd *) data)->sp_expire = -1;
					((struct spwd *) data)->sp_flag = ~0UL;
					return 0;
				}
			}
#endif

			*((long *) p) = (long) strtoul(line, &endptr, 10);

			if (endptr == line) {
				*((long *) p) = ((i != 8) ? -1L : ((long)(~0UL)));
			}

			line = endptr;

			if (i == 8) {
				if (!*endptr) {
					return 0;
				}
				break;
			}

			if (*endptr != ':') {
				break;
			}

		}

		*line++ = 0;
		++i;
	} while (1);

	return EINVAL;
}

#endif
/**********************************************************************/
#ifdef L___pgsreader

/* Reads until if EOF, or until if finds a line which fits in the buffer
 * and for which the parser function succeeds.
 *
 * Returns 0 on success and ENOENT for end-of-file (glibc concession).
 */

int __pgsreader(int (*__parserfunc)(void *d, char *line), void *data,
				char *__restrict line_buff, size_t buflen, FILE *f)
{
	int line_len;
	int skip;
	int rv = ERANGE;
	__STDIO_AUTO_THREADLOCK_VAR;

	if (buflen < PWD_BUFFER_SIZE) {
		__set_errno(rv);
	} else {
		__STDIO_AUTO_THREADLOCK(f);

		skip = 0;
		do {
			if (!fgets_unlocked(line_buff, buflen, f)) {
				if (feof_unlocked(f)) {
					rv = ENOENT;
				}
				break;
			}

			line_len = strlen(line_buff) - 1; /* strlen() must be > 0. */
			if (line_buff[line_len] == '\n') {
				line_buff[line_len] = 0;
			} else if (line_len + 2 == buflen) { /* line too long */
				++skip;
				continue;
			}

			if (skip) {
				--skip;
				continue;
			}

			/* NOTE: glibc difference - glibc strips leading whitespace from
			 * records.  We do not allow leading whitespace. */

			/* Skip empty lines, comment lines, and lines with leading
			 * whitespace. */
			if (*line_buff && (*line_buff != '#') && !isspace(*line_buff)) {
				if (__parserfunc == __parsegrent) {	/* Do evil group hack. */
					/* The group entry parsing function needs to know where
					 * the end of the buffer is so that it can construct the
					 * group member ptr table. */
					((struct group *) data)->gr_name = line_buff + buflen;
				}

				if (!__parserfunc(data, line_buff)) {
					rv = 0;
					break;
				}
			}
		} while (1);

		__STDIO_AUTO_THREADUNLOCK(f);
	}

	return rv;
}

#endif
/**********************************************************************/
