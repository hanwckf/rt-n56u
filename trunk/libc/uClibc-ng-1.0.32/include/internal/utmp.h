/*
 * internal helper for utmp and utmpx handling
 *
 * Copyright (C) 2015 by Bernhard Reutner-Fischer
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */
#ifndef __INTERNAL_UTMP_H
#define __INTERNAL_UTMP_H

#include <utmpx.h>
#include <utmp.h>

/* Note: _PATH_UTMPX == _PATH_UTMP */

#if (defined __UCLIBC_HAS_UTMPX__ && defined __UCLIBC_HAS_UTMP__) \
	|| !defined __UCLIBC_HAS_UTMP__
/* implement the X and alias the non-X */
# define __set_unlocked __setutxent_unlocked
# define set setutxent
# define __get_unlocked __getutxent_unlocked
# define get getutxent
# define end endutxent
# define __getid_unlocked __getutxid_unlocked
# define getid getutxid
# define getline getutxline
# define putline pututxline
# define name utmpxname
# define updw updwtmpx
# define UT utmpx
# ifndef __DEFAULT_PATH_UTMP
#  define __DEFAULT_PATH_UTMP _PATH_UTMPX
# endif
# if defined __UCLIBC_HAS_UTMP__
#  define other(n,a) strong_alias_untyped(n,a)
# else
#  define other(n,a) /* nothing */
# endif
#elif defined __UCLIBC_HAS_UTMP__
# define __set_unlocked __setutent_unlocked
# define set setutent
# define __get_unlocked __getutent_unlocked
# define get getutent
# define end endutent
# define __getid_unlocked __getutid_unlocked
# define getid getutid
# define getline getutline
# define putline pututline
# define name utmpname
# define updw updwtmp
# define UT utmp
# ifndef __DEFAULT_PATH_UTMP
#  define __DEFAULT_PATH_UTMP _PATH_UTMP
# endif
# define other(n,a) /* nothing */
#else
#error You are supposed to either have UTMP or UTMPX or both here
#endif

/* not used in libc_hidden_proto(setutxent) */
/* not used in libc_hidden_proto(endutxent) */
/* not used in libc_hidden_proto(getutxent) */
/* not used in libc_hidden_proto(getutxid) */
/* not used in libc_hidden_proto(getutxline) */
/* not used in libc_hidden_proto(pututxline) */
/* not used in libc_hidden_proto(utmpxname) */
/* not used in libc_hidden_proto(updwtmpx) */

/* not used in libc_hidden_proto(setutent) */
/* not used in libc_hidden_proto(endutent) */
/* not used in libc_hidden_proto(getutent) */
/* not used in libc_hidden_proto(getutid) */
/* not used in libc_hidden_proto(getutline) */
/* not used in libc_hidden_proto(pututline) */
/* not used in libc_hidden_proto(utmpname) */
/* not used in libc_hidden_proto(updwtmp) */

#ifdef IS_IN_libutil
# if (defined __UCLIBC_HAS_UTMPX__ && defined __UCLIBC_HAS_UTMP__) \
	|| !defined __UCLIBC_HAS_UTMP__
	/* monkey-patch to use the POSIX interface */
#  define setutent setutxent
#  define getutline getutxline
#  define pututline pututxline
#  define endutent endutxent
#  define updwtmp updwtmpx
# endif
#endif /* IS_IN_libutil */

#endif /* __INTERNAL_UTMP_H */

