/* Copyright (C) 1991,92,95,96,97,98,2000,2001 Free Software Foundation, Inc.
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

#ifndef	_GLOB_H
#define	_GLOB_H	1

#ifdef	__cplusplus
extern "C" {
#endif

#undef	__ptr_t
#if defined __cplusplus || (defined __STDC__ && __STDC__) || defined WINDOWS32
# if !defined __GLIBC__ || !defined __P
#  undef __P
#  undef __PMT
#  define __P(protos)	protos
#  define __PMT(protos)	protos
#  if !defined __GNUC__ || __GNUC__ < 2
#   undef __const
#   define __const const
#  endif
# endif
# define __ptr_t	void *
#else /* Not C++ or ANSI C.  */
# undef	__P
# undef __PMT
# define __P(protos)	()
# define __PMT(protos)	()
# undef	__const
# define __const
# define __ptr_t	char *
#endif /* C++ or ANSI C.  */

/* We need `size_t' for the following definitions.  */
#ifndef __size_t
# if defined __GNUC__ && __GNUC__ >= 2
typedef __SIZE_TYPE__ __size_t;
#  ifdef _XOPEN_SOURCE
typedef __SIZE_TYPE__ size_t;
#  endif
# else
/* This is a guess.  */
typedef unsigned long int __size_t;
# endif
#else
/* The GNU CC stddef.h version defines __size_t as empty.  We need a real
   definition.  */
# undef __size_t
# define __size_t size_t
#endif

/* Bits set in the FLAGS argument to `glob'.  */
#define	GLOB_ERR	(1 << 0)/* Return on read errors.  */
#define	GLOB_MARK	(1 << 1)/* Append a slash to each name.  */
#define	GLOB_NOSORT	(1 << 2)/* Don't sort the names.  */
#define	GLOB_DOOFFS	(1 << 3)/* Insert PGLOB->gl_offs NULLs.  */
#define	GLOB_NOCHECK	(1 << 4)/* If nothing matches, return the pattern.  */
#define	GLOB_APPEND	(1 << 5)/* Append to results of a previous call.  */
#define	GLOB_NOESCAPE	(1 << 6)/* Backslashes don't quote metacharacters.  */
#define	GLOB_PERIOD	(1 << 7)/* Leading `.' can be matched by metachars.  */

#if (!defined _POSIX_C_SOURCE || _POSIX_C_SOURCE < 2 || defined _BSD_SOURCE \
     || defined _GNU_SOURCE)
# define GLOB_MAGCHAR	 (1 << 8)/* Set in gl_flags if any metachars seen.  */
# define GLOB_ALTDIRFUNC (1 << 9)/* Use gl_opendir et al functions.  */
# define GLOB_BRACE	 (1 << 10)/* Expand "{a,b}" to "a" "b".  */
# define GLOB_NOMAGIC	 (1 << 11)/* If no magic chars, return the pattern.  */
# define GLOB_TILDE	 (1 << 12)/* Expand ~user and ~ to home directories. */
# define GLOB_ONLYDIR	 (1 << 13)/* Match only directories.  */
# define GLOB_TILDE_CHECK (1 << 14)/* Like GLOB_TILDE but return an error
				      if the user name is not available.  */
# define __GLOB_FLAGS	(GLOB_ERR|GLOB_MARK|GLOB_NOSORT|GLOB_DOOFFS| \
			 GLOB_NOESCAPE|GLOB_NOCHECK|GLOB_APPEND|     \
			 GLOB_PERIOD|GLOB_ALTDIRFUNC|GLOB_BRACE|     \
			 GLOB_NOMAGIC|GLOB_TILDE|GLOB_ONLYDIR|GLOB_TILDE_CHECK)
#else
# define __GLOB_FLAGS	(GLOB_ERR|GLOB_MARK|GLOB_NOSORT|GLOB_DOOFFS| \
			 GLOB_NOESCAPE|GLOB_NOCHECK|GLOB_APPEND|     \
			 GLOB_PERIOD)
#endif

/* Error returns from `glob'.  */
#define	GLOB_NOSPACE	1	/* Ran out of memory.  */
#define	GLOB_ABORTED	2	/* Read error.  */
#define	GLOB_NOMATCH	3	/* No matches found.  */
#define GLOB_NOSYS	4	/* Not implemented.  */
#ifdef _GNU_SOURCE
/* Previous versions of this file defined GLOB_ABEND instead of
   GLOB_ABORTED.  Provide a compatibility definition here.  */
# define GLOB_ABEND GLOB_ABORTED
#endif

/* Structure describing a globbing run.  */
#if !defined _AMIGA && !defined VMS /* Buggy compiler.   */
# ifdef _GNU_SOURCE
struct stat;
# endif
#endif
typedef struct
  {
    __size_t gl_pathc;		/* Count of paths matched by the pattern.  */
    char **gl_pathv;		/* List of matched pathnames.  */
    __size_t gl_offs;		/* Slots to reserve in `gl_pathv'.  */
    int gl_flags;		/* Set to FLAGS, maybe | GLOB_MAGCHAR.  */

    /* If the GLOB_ALTDIRFUNC flag is set, the following functions
       are used instead of the normal file access functions.  */
    void (*gl_closedir) __PMT ((void *));
#ifdef _GNU_SOURCE
    struct dirent *(*gl_readdir) __PMT ((void *));
#else
    void *(*gl_readdir) __PMT ((void *));
#endif
    __ptr_t (*gl_opendir) __PMT ((__const char *));
#ifdef _GNU_SOURCE
    int (*gl_lstat) __PMT ((__const char *__restrict,
			    struct stat *__restrict));
    int (*gl_stat) __PMT ((__const char *__restrict, struct stat *__restrict));
#else
    int (*gl_lstat) __PMT ((__const char *__restrict, void *__restrict));
    int (*gl_stat) __PMT ((__const char *__restrict, void *__restrict));
#endif
  } glob_t;

#ifdef _LARGEFILE64_SOURCE
# ifdef _GNU_SOURCE
struct stat64;
# endif
typedef struct
  {
    __size_t gl_pathc;
    char **gl_pathv;
    __size_t gl_offs;
    int gl_flags;

    /* If the GLOB_ALTDIRFUNC flag is set, the following functions
       are used instead of the normal file access functions.  */
    void (*gl_closedir) __PMT ((void *));
# ifdef _GNU_SOURCE
    struct dirent64 *(*gl_readdir) __PMT ((void *));
# else
    void *(*gl_readdir) __PMT ((void *));
# endif
    __ptr_t (*gl_opendir) __PMT ((__const char *));
# ifdef _GNU_SOURCE
    int (*gl_lstat) __PMT ((__const char *__restrict,
			    struct stat64 *__restrict));
    int (*gl_stat) __PMT ((__const char *__restrict,
			   struct stat64 *__restrict));
# else
    int (*gl_lstat) __PMT ((__const char *__restrict, void *__restrict));
    int (*gl_stat) __PMT ((__const char *__restrict, void *__restrict));
# endif
  } glob64_t;
#endif

#if _FILE_OFFSET_BITS == 64 && __GNUC__ < 2
# define glob glob64
# define globfree globfree64
#endif

/* Do glob searching for PATTERN, placing results in PGLOB.
   The bits defined above may be set in FLAGS.
   If a directory cannot be opened or read and ERRFUNC is not nil,
   it is called with the pathname that caused the error, and the
   `errno' value from the failing call; if it returns non-zero
   `glob' returns GLOB_ABEND; if it returns zero, the error is ignored.
   If memory cannot be allocated for PGLOB, GLOB_NOSPACE is returned.
   Otherwise, `glob' returns zero.  */
#if _FILE_OFFSET_BITS != 64 || __GNUC__ < 2
extern int glob __P ((__const char *__restrict __pattern, int __flags,
		      int (*__errfunc) (__const char *, int),
		      glob_t *__restrict __pglob));

/* Free storage allocated in PGLOB by a previous `glob' call.  */
extern void globfree __P ((glob_t *__pglob));
#else
extern int glob __P ((__const char *__restrict __pattern, int __flags,
		      int (*__errfunc) (__const char *, int),
		      glob_t *__restrict __pglob)) __asm__ ("glob64");

extern void globfree __P ((glob_t *__pglob)) __asm__ ("globfree64");
#endif

#ifdef _LARGEFILE64_SOURCE
extern int glob64 __P ((__const char *__restrict __pattern, int __flags,
			int (*__errfunc) (__const char *, int),
			glob64_t *__restrict __pglob));

extern void globfree64 __P ((glob64_t *__pglob));
#endif


#ifdef _GNU_SOURCE
/* Return nonzero if PATTERN contains any metacharacters.
   Metacharacters can be quoted with backslashes if QUOTE is nonzero.

   This function is not part of the interface specified by POSIX.2
   but several programs want to use it.  */
extern int glob_pattern_p __P ((__const char *__pattern, int __quote));
#endif

#ifdef	__cplusplus
}
#endif

#endif /* glob.h  */
