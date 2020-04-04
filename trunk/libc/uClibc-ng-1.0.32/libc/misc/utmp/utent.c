/* utent.c <ndf@linux.mit.edu> */
/* Let it be known that this is very possibly the worst standard ever.  HP-UX
   does one thing, someone else does another, linux another... If anyone
   actually has the standard, please send it to me.

   Note that because of the way this stupid stupid standard works, you
   have to call endutent() to close the file even if you've not called
   setutent -- getutid and family use the same file descriptor.

   Modified by Erik Andersen for uClibc...
*/

#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <paths.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include "internal/utmp.h"
#include <not-cancel.h>
#include <bits/uClibc_mutex.h>

__UCLIBC_MUTEX_STATIC(utmplock, PTHREAD_MUTEX_INITIALIZER);

/* Some global crap */
static int static_fd = -1;
static struct UT *static_utmp = NULL;
static const char default_file[] = __DEFAULT_PATH_UTMP;
static const char *current_file = default_file;

/* This function must be called with the LOCK held */
static void __set_unlocked(void)
{
	if (static_fd < 0) {
		static_fd = open_not_cancel_2(current_file, O_RDWR | O_CLOEXEC);
		if (static_fd < 0) {
			static_fd = open_not_cancel_2(current_file, O_RDONLY | O_CLOEXEC);
			if (static_fd < 0) {
				return; /* static_fd remains < 0 */
			}
		}
#ifndef __ASSUME_O_CLOEXEC
		/* Make sure the file will be closed on exec()  */
		fcntl_not_cancel(static_fd, F_SETFD, FD_CLOEXEC);
#endif
		return;
	}
	lseek(static_fd, 0, SEEK_SET);
}
#if defined __UCLIBC_HAS_THREADS__
void set(void)
{
	__UCLIBC_MUTEX_LOCK(utmplock);
	__set_unlocked();
	__UCLIBC_MUTEX_UNLOCK(utmplock);
}
#else
strong_alias(__set_unlocked,set)
#endif
/* not used in libc_hidden_def(set) */
other(setutxent,setutent)

/* This function must be called with the LOCK held */
static struct UT *__get_unlocked(void)
{
	if (static_fd < 0) {
		__set_unlocked();
		if (static_fd < 0)
			return NULL;
	}

	if (static_utmp == NULL)
		static_utmp = (struct UT *)__uc_malloc(sizeof(struct UT));

	if (read_not_cancel(static_fd, static_utmp,
			sizeof(struct UT)) == sizeof(struct UT)) {
	return static_utmp;
	}

	return NULL;
}
#if defined __UCLIBC_HAS_THREADS__
struct UT *get(void)
{
	struct UT *ret;

	__UCLIBC_MUTEX_LOCK(utmplock);
	ret = __get_unlocked();
	__UCLIBC_MUTEX_UNLOCK(utmplock);
	return ret;
}
#else
strong_alias(__get_unlocked,get)
#endif
/* not used in libc_hidden_def(get) */
other(getutxent,getutent)

void end(void)
{
	__UCLIBC_MUTEX_LOCK(utmplock);
	if (static_fd >= 0)
		close_not_cancel_no_status(static_fd);
	static_fd = -1;
	__UCLIBC_MUTEX_UNLOCK(utmplock);
}
/* not used in libc_hidden_def(end) */
other(endutxent,endutent)

/* This function must be called with the LOCK held */
static struct UT *__getid_unlocked(const struct UT *utmp_entry)
{
	struct UT *lutmp;
	unsigned type;

	/* We use the fact that constants we are interested in are: */
	/* RUN_LVL=1, ... OLD_TIME=4; INIT_PROCESS=5, ... USER_PROCESS=8 */
	type = utmp_entry->ut_type - 1;
	type /= 4;

	while ((lutmp = __get_unlocked()) != NULL) {
		if (type == 0 && lutmp->ut_type == utmp_entry->ut_type)	{
			/* one of RUN_LVL, BOOT_TIME, NEW_TIME, OLD_TIME */
			return lutmp;
		}
		if (type == 1
			&& strncmp(lutmp->ut_id, utmp_entry->ut_id,
						sizeof(lutmp->ut_id)) == 0) {
			/* INIT_PROCESS, LOGIN_PROCESS, USER_PROCESS, DEAD_PROCESS */
			return lutmp;
		}
	}

	return NULL;
}
#if defined __UCLIBC_HAS_THREADS__
struct UT *getid(const struct UT *utmp_entry)
{
	struct UT *ret;

	__UCLIBC_MUTEX_LOCK(utmplock);
	ret = __getid_unlocked(utmp_entry);
	__UCLIBC_MUTEX_UNLOCK(utmplock);
	return ret;
}
#else
strong_alias(__getid_unlocked,getid)
#endif
/* not used in libc_hidden_def(getid) */
other(getutxid,getutid)

struct UT *getline(const struct UT *utmp_entry)
{
	struct UT *lutmp;

	__UCLIBC_MUTEX_LOCK(utmplock);
	while ((lutmp = __get_unlocked()) != NULL) {
		if (lutmp->ut_type == USER_PROCESS || lutmp->ut_type == LOGIN_PROCESS) {
			if (strncmp(lutmp->ut_line, utmp_entry->ut_line,
						sizeof(lutmp->ut_line)) == 0) {
				break;
			}
		}
	}
	__UCLIBC_MUTEX_UNLOCK(utmplock);
	return lutmp;
}
/* libc_hidden_def(getline) */
other(getutxline,getutline)

struct UT *putline(const struct UT *utmp_entry)
{
	__UCLIBC_MUTEX_LOCK(utmplock);
	/* Ignore the return value.  That way, if they've already positioned
	   the file pointer where they want it, everything will work out. */
	lseek(static_fd, (off_t) - sizeof(struct UT), SEEK_CUR);

	if (__getid_unlocked(utmp_entry) != NULL)
		lseek(static_fd, (off_t) - sizeof(struct UT), SEEK_CUR);
	else
		lseek(static_fd, (off_t) 0, SEEK_END);
	if (write(static_fd, utmp_entry, sizeof(struct UT))
			!= sizeof(struct UT))
		utmp_entry = NULL;

	__UCLIBC_MUTEX_UNLOCK(utmplock);
	return (struct UT *)utmp_entry;
}
/* not used in libc_hidden_def(putline) */
other(pututxline,pututline)

int name(const char *new_file)
{
	__UCLIBC_MUTEX_LOCK(utmplock);
	if (new_file != NULL) {
		if (current_file != default_file)
			free((char *)current_file);
		current_file = strdup(new_file);
		if (current_file == NULL) {
			/* We should probably whine about out-of-memory
			 * errors here...  Instead just reset to the default */
			current_file = default_file;
		}
	}

	if (static_fd >= 0) {
		close_not_cancel_no_status(static_fd);
		static_fd = -1;
	}
	__UCLIBC_MUTEX_UNLOCK(utmplock);
	return 0; /* or maybe return -(current_file != new_file)? */
}
/* not used in libc_hidden_def(name) */
other(utmpxname,utmpname)

void updw(const char *wtmp_file, const struct UT *lutmp)
{
	int fd;

	fd = open_not_cancel_2(wtmp_file, O_APPEND | O_WRONLY);
	if (fd >= 0) {
		if (lockf(fd, F_LOCK, 0) == 0) {
			write_not_cancel(fd, lutmp, sizeof(struct UT));
			lockf(fd, F_ULOCK, 0);
			close_not_cancel_no_status(fd);
		}
	}
}
/* not used in libc_hidden_def(updw) */
other(updwtmpx,updwtmp)
