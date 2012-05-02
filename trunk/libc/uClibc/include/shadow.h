/* Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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

/* Declaration of types and functions for shadow password suite.  */

#ifndef _SHADOW_H
#define _SHADOW_H	1

#include <features.h>

#include <paths.h>

#define	__need_FILE
#include <stdio.h>
#define __need_size_t
#include <stddef.h>

/* Paths to the user database files.  */
#define	SHADOW _PATH_SHADOW


__BEGIN_DECLS

/* Structure of the password file.  */
struct spwd
  {
    char *sp_namp;		/* Login name.  */
    char *sp_pwdp;		/* Encrypted password.  */
    long int sp_lstchg;		/* Date of last change.  */
    long int sp_min;		/* Minimum number of days between changes.  */
    long int sp_max;		/* Maximum number of days between changes.  */
    long int sp_warn;		/* Number of days to warn user to change
				   the password.  */
    long int sp_inact;		/* Number of days the account may be
				   inactive.  */
    long int sp_expire;		/* Number of days since 1970-01-01 until
				   account expires.  */
    unsigned long int sp_flag;	/* Reserved.  */
  };


/* Open database for reading.  */
extern void setspent (void) __THROW;

/* Close database.  */
extern void endspent (void) __THROW;

/* Get next entry from database, perhaps after opening the file.  */
extern struct spwd *getspent (void) __THROW;

/* Get shadow entry matching NAME.  */
extern struct spwd *getspnam (__const char *__name) __THROW;

/* Read shadow entry from STRING.  */
extern struct spwd *sgetspent (__const char *__string) __THROW;

/* Read next shadow entry from STREAM.  */
extern struct spwd *fgetspent (FILE *__stream) __THROW;

/* Write line containing shadow password entry to stream.  */
extern int putspent (__const struct spwd *__p, FILE *__stream) __THROW;


#ifdef __USE_MISC
/* Reentrant versions of some of the functions above.  */
extern int getspent_r (struct spwd *__result_buf, char *__buffer,
		       size_t __buflen, struct spwd **__result) __THROW;

extern int getspnam_r (__const char *__name, struct spwd *__result_buf,
		       char *__buffer, size_t __buflen,
		       struct spwd **__result)__THROW;

extern int sgetspent_r (__const char *__string, struct spwd *__result_buf,
			char *__buffer, size_t __buflen,
			struct spwd **__result) __THROW;

extern int fgetspent_r (FILE *__stream, struct spwd *__result_buf,
			char *__buffer, size_t __buflen,
			struct spwd **__result) __THROW;
#endif	/* misc */

/* Protect password file against multi writers.  */
extern int lckpwdf (void) __THROW;

/* Unlock password file.  */
extern int ulckpwdf (void) __THROW;

__END_DECLS

#endif /* shadow.h */
