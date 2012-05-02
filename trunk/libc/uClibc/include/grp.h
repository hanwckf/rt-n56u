/* Copyright (C) 1991,92,95,96,97,98,99,2000,01 Free Software Foundation, Inc.
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
   02111-1307 USA.  */

/*
 *	POSIX Standard: 9.2.1 Group Database Access	<grp.h>
 */

#ifndef	_GRP_H
#define	_GRP_H	1

#include <features.h>

__BEGIN_DECLS

#include <bits/types.h>

#define __need_size_t
#include <stddef.h>


/* For the Single Unix specification we must define this type here.  */
#if defined __USE_XOPEN && !defined __gid_t_defined
typedef __gid_t gid_t;
# define __gid_t_defined
#endif

/* The group structure.	 */
struct group
  {
    char *gr_name;		/* Group name.	*/
    char *gr_passwd;		/* Password.	*/
    __gid_t gr_gid;		/* Group ID.	*/
    char **gr_mem;		/* Member list.	*/
  };


#if defined __USE_SVID || defined __USE_GNU
# define __need_FILE
# include <stdio.h>
#endif


#if defined __USE_SVID || defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Rewind the group-file stream.  */
extern void setgrent (void) __THROW;

/* Close the group-file stream.  */
extern void endgrent (void) __THROW;

/* Read an entry from the group-file stream, opening it if necessary.  */
extern struct group *getgrent (void) __THROW;
#endif

#ifdef	__USE_SVID
/* Read a group entry from STREAM.  */
extern struct group *fgetgrent (FILE *__stream) __THROW;
#endif

#ifdef __USE_GNU
/* Write the given entry onto the given stream.  */
extern int putgrent (__const struct group *__restrict __p,
		     FILE *__restrict __f) __THROW;
#endif

/* Search for an entry with a matching group ID.  */
extern struct group *getgrgid (__gid_t __gid) __THROW;

/* Search for an entry with a matching group name.  */
extern struct group *getgrnam (__const char *__name) __THROW;

#if defined __USE_POSIX || defined __USE_MISC

# ifdef __USE_MISC
/* Reasonable value for the buffer sized used in the reentrant
   functions below.  But better use `sysconf'.  */
#  define NSS_BUFLEN_GROUP	1024
# endif

/* Reentrant versions of some of the functions above.

   PLEASE NOTE: the `getgrent_r' function is not (yet) standardized.
   The interface may change in later versions of this library.  But
   the interface is designed following the principals used for the
   other reentrant functions so the chances are good this is what the
   POSIX people would choose.  */

# ifdef __USE_GNU
extern int getgrent_r (struct group *__restrict __resultbuf,
		       char *__restrict __buffer, size_t __buflen,
		       struct group **__restrict __result) __THROW;
# endif

/* Search for an entry with a matching group ID.  */
extern int getgrgid_r (__gid_t __gid, struct group *__restrict __resultbuf,
		       char *__restrict __buffer, size_t __buflen,
		       struct group **__restrict __result) __THROW;

/* Search for an entry with a matching group name.  */
extern int getgrnam_r (__const char *__restrict __name,
		       struct group *__restrict __resultbuf,
		       char *__restrict __buffer, size_t __buflen,
		       struct group **__restrict __result) __THROW;

# ifdef	__USE_SVID
/* Read a group entry from STREAM.  This function is not standardized
   an probably never will.  */
extern int fgetgrent_r (FILE *__restrict __stream,
			struct group *__restrict __resultbuf,
			char *__restrict __buffer, size_t __buflen,
			struct group **__restrict __result) __THROW;
# endif

#endif	/* POSIX or reentrant */


#ifdef	__USE_BSD

# define __need_size_t
# include <stddef.h>

/* Set the group set for the current user to GROUPS (N of them).  */
extern int setgroups (size_t __n, __const __gid_t *__groups) __THROW;

/* Initialize the group set for the current user
   by reading the group database and using all groups
   of which USER is a member.  Also include GROUP.  */
extern int initgroups (__const char *__user, __gid_t __group) __THROW;

#endif /* Use BSD.  */

__END_DECLS

#endif /* grp.h  */
