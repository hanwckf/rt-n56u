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
#include <string.h>
#include <utmp.h>
#include <not-cancel.h>

#include <bits/uClibc_mutex.h>
__UCLIBC_MUTEX_STATIC(utmplock, PTHREAD_MUTEX_INITIALIZER);


/* Do not create extra unlocked functions if no locking is needed */
#if defined __UCLIBC_HAS_THREADS__
# define static_if_threaded static
#else
# define static_if_threaded /* nothing */
# define __setutent setutent
# define __getutent getutent
# define __getutid getutid
#endif


/* Some global crap */
static int static_fd = -1;
static struct utmp static_utmp;
static const char default_file_name[] = _PATH_UTMP;
static const char *static_ut_name = default_file_name;


/* This function must be called with the LOCK held */
static_if_threaded void __setutent(void)
{
    if (static_fd < 0) {
	static_fd = open_not_cancel_2(static_ut_name, O_RDWR | O_CLOEXEC);
	if (static_fd < 0) {
	    static_fd = open_not_cancel_2(static_ut_name, O_RDONLY | O_CLOEXEC);
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
void setutent(void)
{
    __UCLIBC_MUTEX_LOCK(utmplock);
    __setutent();
    __UCLIBC_MUTEX_UNLOCK(utmplock);
}
#endif
libc_hidden_def(setutent)

/* This function must be called with the LOCK held */
static_if_threaded struct utmp *__getutent(void)
{
    if (static_fd < 0) {
	__setutent();
	if (static_fd < 0) {
	    return NULL;
	}
    }

    if (read_not_cancel(static_fd, &static_utmp, sizeof(static_utmp)) == sizeof(static_utmp)) {
	return &static_utmp;
    }

    return NULL;
}
#if defined __UCLIBC_HAS_THREADS__
struct utmp *getutent(void)
{
    struct utmp *ret;

    __UCLIBC_MUTEX_LOCK(utmplock);
    ret = __getutent();
    __UCLIBC_MUTEX_UNLOCK(utmplock);
    return ret;
}
#endif
libc_hidden_def(getutent)

void endutent(void)
{
    __UCLIBC_MUTEX_LOCK(utmplock);
    if (static_fd >= 0)
	close_not_cancel_no_status(static_fd);
    static_fd = -1;
    __UCLIBC_MUTEX_UNLOCK(utmplock);
}
libc_hidden_def(endutent)

/* This function must be called with the LOCK held */
static_if_threaded struct utmp *__getutid(const struct utmp *utmp_entry)
{
    struct utmp *lutmp;
    unsigned type;

    /* We use the fact that constants we are interested in are: */
    /* RUN_LVL=1, ... OLD_TIME=4; INIT_PROCESS=5, ... USER_PROCESS=8 */
    type = utmp_entry->ut_type - 1;
    type /= 4;

    while ((lutmp = __getutent()) != NULL) {
	if (type == 0 && lutmp->ut_type == utmp_entry->ut_type)	{
	    /* one of RUN_LVL, BOOT_TIME, NEW_TIME, OLD_TIME */
	    return lutmp;
	}
	if (type == 1 && strncmp(lutmp->ut_id, utmp_entry->ut_id, sizeof(lutmp->ut_id)) == 0) {
	    /* INIT_PROCESS, LOGIN_PROCESS, USER_PROCESS, DEAD_PROCESS */
	    return lutmp;
	}
    }

    return NULL;
}
#if defined __UCLIBC_HAS_THREADS__
struct utmp *getutid(const struct utmp *utmp_entry)
{
    struct utmp *ret;

    __UCLIBC_MUTEX_LOCK(utmplock);
    ret = __getutid(utmp_entry);
    __UCLIBC_MUTEX_UNLOCK(utmplock);
    return ret;
}
#endif
libc_hidden_def(getutid)

struct utmp *getutline(const struct utmp *utmp_entry)
{
    struct utmp *lutmp;

    __UCLIBC_MUTEX_LOCK(utmplock);
    while ((lutmp = __getutent()) != NULL) {
	if (lutmp->ut_type == USER_PROCESS || lutmp->ut_type == LOGIN_PROCESS) {
	    if (strncmp(lutmp->ut_line, utmp_entry->ut_line, sizeof(lutmp->ut_line)) == 0) {
		break;
	    }
	}
    }
    __UCLIBC_MUTEX_UNLOCK(utmplock);
    return lutmp;
}
libc_hidden_def(getutline)

struct utmp *pututline(const struct utmp *utmp_entry)
{
    __UCLIBC_MUTEX_LOCK(utmplock);
    /* Ignore the return value.  That way, if they've already positioned
       the file pointer where they want it, everything will work out. */
    lseek(static_fd, (off_t) - sizeof(struct utmp), SEEK_CUR);

    if (__getutid(utmp_entry) != NULL)
	lseek(static_fd, (off_t) - sizeof(struct utmp), SEEK_CUR);
    else
	lseek(static_fd, (off_t) 0, SEEK_END);
    if (write(static_fd, utmp_entry, sizeof(struct utmp)) != sizeof(struct utmp))
	utmp_entry = NULL;

    __UCLIBC_MUTEX_UNLOCK(utmplock);
    return (struct utmp *)utmp_entry;
}
libc_hidden_def(pututline)

int utmpname(const char *new_ut_name)
{
    __UCLIBC_MUTEX_LOCK(utmplock);
    if (new_ut_name != NULL) {
	if (static_ut_name != default_file_name)
	    free((char *)static_ut_name);
	static_ut_name = strdup(new_ut_name);
	if (static_ut_name == NULL) {
	    /* We should probably whine about out-of-memory
	     * errors here...  Instead just reset to the default */
	    static_ut_name = default_file_name;
	}
    }

    if (static_fd >= 0) {
	close_not_cancel_no_status(static_fd);
	static_fd = -1;
    }
    __UCLIBC_MUTEX_UNLOCK(utmplock);
    return 0; /* or maybe return -(static_ut_name != new_ut_name)? */
}
libc_hidden_def(utmpname)
