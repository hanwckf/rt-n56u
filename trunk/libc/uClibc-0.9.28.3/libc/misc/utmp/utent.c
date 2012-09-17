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

#include <bits/uClibc_mutex.h>

__UCLIBC_MUTEX_STATIC(utmplock, PTHREAD_MUTEX_INITIALIZER);

/* Some global crap */
static int static_fd = -1;
static struct utmp static_utmp;
static const char default_file_name[] = _PATH_UTMP;
static const char *static_ut_name = (const char *) default_file_name;



static struct utmp *__getutent(int utmp_fd)

{
    if (utmp_fd == -1) {
	setutent();
    }
    if (utmp_fd == -1) {
	return NULL;
    }

    __UCLIBC_MUTEX_LOCK(utmplock);
    if (read(utmp_fd, (char *) &static_utmp, sizeof(struct utmp)) != sizeof(struct utmp)) 
    {
	return NULL;
    }

    __UCLIBC_MUTEX_UNLOCK(utmplock);
    return &static_utmp;
}

void setutent(void)
{
    int ret;

    __UCLIBC_MUTEX_LOCK(utmplock);
    if (static_fd == -1) {
	if ((static_fd = open(static_ut_name, O_RDWR)) < 0) {
	    if ((static_fd = open(static_ut_name, O_RDONLY)) < 0) {
		goto bummer;
	    }
	}
	/* Make sure the file will be closed on exec()  */
	ret = fcntl(static_fd, F_GETFD, 0);
	if (ret >= 0) {
	    ret = fcntl(static_fd, F_GETFD, 0);
	}
	if (ret < 0) {
bummer:
	    close(static_fd);
			static_fd = -1;
			goto DONE;
	}
    }
    lseek(static_fd, 0, SEEK_SET);
 DONE:
    __UCLIBC_MUTEX_UNLOCK(utmplock);
    return;
}

void endutent(void)
{
    __UCLIBC_MUTEX_LOCK(utmplock);
    if (static_fd != -1) {
	close(static_fd);
    }
    static_fd = -1;
    __UCLIBC_MUTEX_UNLOCK(utmplock);
}

/* Locking is done in __getutent */
struct utmp *getutent(void)
{
    return __getutent(static_fd);
}

/* Locking is done in __getutent */
struct utmp *getutid (const struct utmp *utmp_entry)
{
    struct utmp *lutmp;

    while ((lutmp = __getutent(static_fd)) != NULL) {
	if (	(utmp_entry->ut_type == RUN_LVL ||
		 utmp_entry->ut_type == BOOT_TIME ||
		 utmp_entry->ut_type == NEW_TIME ||
		 utmp_entry->ut_type == OLD_TIME) &&
		lutmp->ut_type == utmp_entry->ut_type)  
	{
	    return lutmp;
	}
	if (	(utmp_entry->ut_type == INIT_PROCESS ||
		 utmp_entry->ut_type == DEAD_PROCESS ||
		 utmp_entry->ut_type == LOGIN_PROCESS ||
		 utmp_entry->ut_type == USER_PROCESS) &&
		!strncmp(lutmp->ut_id, utmp_entry->ut_id, sizeof(lutmp->ut_id))) 
	{
	    return lutmp;
	}
    }

    return NULL;
}

/* Locking is done in __getutent */
struct utmp *getutline(const struct utmp *utmp_entry)
{
    struct utmp *lutmp;

    while ((lutmp = __getutent(static_fd)) != NULL) {
	if ((lutmp->ut_type == USER_PROCESS || lutmp->ut_type == LOGIN_PROCESS) &&
		!strcmp(lutmp->ut_line, utmp_entry->ut_line))
	{
	    return lutmp;
	}
    }

    return NULL;
}

struct utmp *pututline (const struct utmp *utmp_entry)
{
    __UCLIBC_MUTEX_LOCK(utmplock);
    /* Ignore the return value.  That way, if they've already positioned
       the file pointer where they want it, everything will work out. */
    lseek(static_fd, (off_t) - sizeof(struct utmp), SEEK_CUR);

    if (getutid(utmp_entry) != NULL) {
	lseek(static_fd, (off_t) - sizeof(struct utmp), SEEK_CUR);
	if (write(static_fd, utmp_entry, sizeof(struct utmp)) != sizeof(struct utmp))
	    return NULL;
    } else {
	lseek(static_fd, (off_t) 0, SEEK_END);
	if (write(static_fd, utmp_entry, sizeof(struct utmp)) != sizeof(struct utmp))
	    return NULL;
    }

    __UCLIBC_MUTEX_UNLOCK(utmplock);
    return (struct utmp *)utmp_entry;
}

int utmpname (const char *new_ut_name)
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

    if (static_fd != -1)
	close(static_fd);
    __UCLIBC_MUTEX_UNLOCK(utmplock);
    return 0;
}

