/* Copyright (C) 1991-2002,2003,2004,2005,2006 Free Software Foundation, Inc.
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

#undef ENABLE_GLOB_BRACE_EXPANSION
#undef ENABLE_GLOB_TILDE_EXPANSION

#include <features.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <malloc.h>
#include <fnmatch.h>
#include <glob.h>



#ifdef ENABLE_GLOB_TILDE_EXPANSION
#include <pwd.h>
#endif

#ifdef COMPILE_GLOB64
#undef stat
#define stat stat64
#define struct_stat64          struct stat64
#define __stat64(fname, buf)   stat64 (fname, buf)
#define dirent dirent64
#define __readdir readdir64
#define __readdir64 readdir64
#define glob_t glob64_t
#define glob(pattern, flags, errfunc, pglob) glob64 (pattern, flags, errfunc, pglob)
#define globfree(pglob) globfree64 (pglob)
#else
#define __readdir readdir
#ifdef __UCLIBC_HAS_LFS__
#define __readdir64 readdir64
#else
#define __readdir64 readdir
#endif
#define struct_stat64          struct stat
#define __stat64(fname, buf)   stat (fname, buf)
#endif


/* When used in the GNU libc the symbol _DIRENT_HAVE_D_TYPE is available
   if the `d_type' member for `struct dirent' is available.
   HAVE_STRUCT_DIRENT_D_TYPE plays the same role in GNULIB.  */
#if defined _DIRENT_HAVE_D_TYPE
/* True if the directory entry D must be of type T.  */
# define DIRENT_MUST_BE(d, t)	((d)->d_type == (t))

/* True if the directory entry D might be a symbolic link.  */
# define DIRENT_MIGHT_BE_SYMLINK(d) \
    ((d)->d_type == DT_UNKNOWN || (d)->d_type == DT_LNK)

/* True if the directory entry D might be a directory.  */
# define DIRENT_MIGHT_BE_DIR(d)	 \
    ((d)->d_type == DT_DIR || DIRENT_MIGHT_BE_SYMLINK (d))

#else /* !HAVE_D_TYPE */
# define DIRENT_MUST_BE(d, t)		false
# define DIRENT_MIGHT_BE_SYMLINK(d)	true
# define DIRENT_MIGHT_BE_DIR(d)		true
#endif /* HAVE_D_TYPE */


# define NAMLEN(dirent) strlen((dirent)->d_name)
#ifdef _D_NAMLEN
# undef NAMLEN
# define NAMLEN(d) _D_NAMLEN(d)
#endif

# if defined _DIRENT_HAVE_D_NAMLEN
#  define CONVERT_D_NAMLEN(d64, d32)    (d64)->d_namlen = (d32)->d_namlen;
# else
#  define CONVERT_D_NAMLEN(d64, d32)
# endif

#  define CONVERT_D_INO(d64, d32)	(d64)->d_ino = (d32)->d_ino;

# ifdef _DIRENT_HAVE_D_TYPE
#  define CONVERT_D_TYPE(d64, d32)	(d64)->d_type = (d32)->d_type;
# else
#  define CONVERT_D_TYPE(d64, d32)
# endif

# define CONVERT_DIRENT_DIRENT64(d64, d32) \
  memcpy ((d64)->d_name, (d32)->d_name, NAMLEN (d32) + 1);		      \
  CONVERT_D_NAMLEN (d64, d32)						      \
  CONVERT_D_INO (d64, d32)						      \
  CONVERT_D_TYPE (d64, d32)

extern int __collated_compare (const void *a, const void *b) attribute_hidden;
extern int __prefix_array (const char *dirname, char **array, size_t n) attribute_hidden;
#if defined ENABLE_GLOB_BRACE_EXPANSION
extern const char *__next_brace_sub (const char *cp, int flags) attribute_hidden;
#endif

#ifndef COMPILE_GLOB64
/* Return nonzero if PATTERN contains any metacharacters.
   Metacharacters can be quoted with backslashes if QUOTE is nonzero.  */
int glob_pattern_p(const char *pattern, int quote)
{
  register const char *p;
  int open = 0;

  for (p = pattern; *p != '\0'; ++p)
    switch (*p)
      {
      case '?':
      case '*':
	return 1;

      case '\\':
	if (quote && p[1] != '\0')
	  ++p;
	break;

      case '[':
	open = 1;
	break;

      case ']':
	if (open)
	  return 1;
	break;
      }

  return 0;
}
libc_hidden_def(glob_pattern_p)


/* Do a collated comparison of A and B.  */
int __collated_compare (const void *a, const void *b)
{
  const char *const s1 = *(const char *const * const) a;
  const char *const s2 = *(const char *const * const) b;

  if (s1 == s2)
    return 0;
  if (s1 == NULL)
    return 1;
  if (s2 == NULL)
    return -1;
  return strcoll (s1, s2);
}


/* Prepend DIRNAME to each of N members of ARRAY, replacing ARRAY's
   elements in place.  Return nonzero if out of memory, zero if successful.
   A slash is inserted between DIRNAME and each elt of ARRAY,
   unless DIRNAME is just "/".  Each old element of ARRAY is freed.
   If ADD_SLASH is non-zero, allocate one character more than
   necessary, so that a slash can be appended later.  */
int __prefix_array (const char *dirname, char **array, size_t n)
{
  register size_t i;
  size_t dirlen = strlen (dirname);
# define DIRSEP_CHAR '/'

  if (dirlen == 1 && dirname[0] == '/')
    /* DIRNAME is just "/", so normal prepending would get us "//foo".
       We want "/foo" instead, so don't prepend any chars from DIRNAME.  */
    dirlen = 0;

  for (i = 0; i < n; ++i)
    {
      size_t eltlen = strlen (array[i]) + 1;
      char *new = (char *) malloc (dirlen + 1 + eltlen);
      if (new == NULL)
	{
	  while (i > 0)
	    free (array[--i]);
	  return 1;
	}

      {
	char *endp = mempcpy (new, dirname, dirlen);
	*endp++ = DIRSEP_CHAR;
	mempcpy (endp, array[i], eltlen);
      }
      free (array[i]);
      array[i] = new;
    }

  return 0;
}

#if defined ENABLE_GLOB_BRACE_EXPANSION
/* Find the end of the sub-pattern in a brace expression.  */
const char *
__next_brace_sub (const char *cp, int flags)
{
  unsigned int depth = 0;
  while (*cp != '\0')
    if ((flags & GLOB_NOESCAPE) == 0 && *cp == '\\')
      {
	if (*++cp == '\0')
	  break;
	++cp;
      }
    else
      {
	if ((*cp == '}' && depth-- == 0) || (*cp == ',' && depth == 0))
	  break;

	if (*cp++ == '{')
	  depth++;
      }

  return *cp != '\0' ? cp : NULL;
}
#endif
#endif


static int
link_exists_p (const char *dir, size_t dirlen, const char *fname,
	       glob_t *pglob, int flags)
{
  size_t fnamelen = strlen (fname);
  char *fullname = (char *) alloca (dirlen + 1 + fnamelen + 1);
  struct stat st;
  struct_stat64 st64;

  mempcpy (mempcpy (mempcpy (fullname, dir, dirlen), "/", 1),
	   fname, fnamelen + 1);

  return (((flags & GLOB_ALTDIRFUNC)
	   ? (*pglob->gl_stat) (fullname, &st)
	   : __stat64 (fullname, &st64)) == 0);
}

/* Like `glob', but PATTERN is a final pathname component,
   and matches are searched for in DIRECTORY.
   The GLOB_NOSORT bit in FLAGS is ignored.  No sorting is ever done.
   The GLOB_APPEND flag is assumed to be set (always appends).  */
static int glob_in_dir (const char *pattern, const char *directory, int flags,
	     int (*errfunc) (const char *, int),
	     glob_t *pglob)
{
  size_t dirlen = strlen (directory);
  void *stream = NULL;
  struct globlink
    {
      struct globlink *next;
      char *name;
    };
  struct globlink *names = NULL;
  size_t nfound;
  int meta;
  int save;

  meta = glob_pattern_p (pattern, !(flags & GLOB_NOESCAPE));
  if (meta == 0 && (flags & (GLOB_NOCHECK|GLOB_NOMAGIC)))
    {
      /* We need not do any tests.  The PATTERN contains no meta
	 characters and we must not return an error therefore the
	 result will always contain exactly one name.  */
      flags |= GLOB_NOCHECK;
      nfound = 0;
    }
  else if (meta == 0 &&
	   ((flags & GLOB_NOESCAPE) || strchr (pattern, '\\') == NULL))
    {
      /* Since we use the normal file functions we can also use stat()
	 to verify the file is there.  */
      struct stat st;
      struct_stat64 st64;
      size_t patlen = strlen (pattern);
      char *fullname = (char *) alloca (dirlen + 1 + patlen + 1);

      mempcpy (mempcpy (mempcpy (fullname, directory, dirlen),
			"/", 1),
	       pattern, patlen + 1);
      if (((flags & GLOB_ALTDIRFUNC)
	   ? (*pglob->gl_stat) (fullname, &st)
	   : __stat64 (fullname, &st64)) == 0)
	/* We found this file to be existing.  Now tell the rest
	   of the function to copy this name into the result.  */
	flags |= GLOB_NOCHECK;

      nfound = 0;
    }
  else
    {
      if (pattern[0] == '\0')
	{
	  /* This is a special case for matching directories like in
	     "*a/".  */
	  names = (struct globlink *) alloca (sizeof (struct globlink));
	  names->name = (char *) malloc (1);
	  if (names->name == NULL)
	    goto memory_error;
	  names->name[0] = '\0';
	  names->next = NULL;
	  nfound = 1;
	  meta = 0;
	}
      else
	{
	  stream = ((flags & GLOB_ALTDIRFUNC)
		    ? (*pglob->gl_opendir) (directory)
		    : opendir (directory));
	  if (stream == NULL)
	    {
	      if (errno != ENOTDIR
		  && ((errfunc != NULL && (*errfunc) (directory, errno))
		      || (flags & GLOB_ERR)))
		return GLOB_ABORTED;
	      nfound = 0;
	      meta = 0;
	    }
	  else
	    {
	      int fnm_flags = ((!(flags & GLOB_PERIOD) ? FNM_PERIOD : 0)
			       | ((flags & GLOB_NOESCAPE) ? FNM_NOESCAPE : 0)
			       );
	      nfound = 0;
	      flags |= GLOB_MAGCHAR;

	      while (1)
		{
		  const char *name;
		  size_t len;
#if defined __UCLIBC_HAS_LFS__ && !defined COMPILE_GLOB64
		  struct dirent64 *d;
		  union
		    {
		      struct dirent64 d64;
		      char room [offsetof (struct dirent64, d_name[0])
				 + NAME_MAX + 1];
		    }
		  d64buf;

		  if (flags & GLOB_ALTDIRFUNC)
		    {
		      struct dirent *d32 = (*pglob->gl_readdir) (stream);
		      if (d32 != NULL)
			{
			  CONVERT_DIRENT_DIRENT64 (&d64buf.d64, d32);
			  d = &d64buf.d64;
			}
		      else
			d = NULL;
		    }
		  else
		    d = __readdir64 (stream);
#else
		  struct dirent *d = ((flags & GLOB_ALTDIRFUNC)
				      ? ((struct dirent *)
					 (*pglob->gl_readdir) (stream))
				      : __readdir (stream));
#endif
		  if (d == NULL)
		    break;
# define REAL_DIR_ENTRY(dp) (dp->d_ino != 0)
		  if (! REAL_DIR_ENTRY (d))
		    continue;

		  /* If we shall match only directories use the information
		     provided by the dirent call if possible.  */
		  if ((flags & GLOB_ONLYDIR) && !DIRENT_MIGHT_BE_DIR (d))
		    continue;

		  name = d->d_name;

		  if (fnmatch (pattern, name, fnm_flags) == 0)
		    {
		      /* If the file we found is a symlink we have to
			 make sure the target file exists.  */
		      if (!DIRENT_MIGHT_BE_SYMLINK (d)
			  || link_exists_p (directory, dirlen, name, pglob,
					    flags))
			{
			  struct globlink *new = (struct globlink *)
			    alloca (sizeof (struct globlink));
			  len = NAMLEN (d);
			  new->name = (char *) malloc (len + 1);
			  if (new->name == NULL)
			    goto memory_error;
			  *((char *) mempcpy (new->name, name, len)) = '\0';
			  new->next = names;
			  names = new;
			  ++nfound;
			}
		    }
		}
	    }
	}
    }

  if (nfound == 0 && (flags & GLOB_NOCHECK))
    {
      size_t len = strlen (pattern);
      nfound = 1;
      names = (struct globlink *) alloca (sizeof (struct globlink));
      names->next = NULL;
      names->name = (char *) malloc (len + 1);
      if (names->name == NULL)
	goto memory_error;
      *((char *) mempcpy (names->name, pattern, len)) = '\0';
    }

  if (nfound != 0)
    {
      char **new_gl_pathv;

      new_gl_pathv
	= (char **) realloc (pglob->gl_pathv,
			     (pglob->gl_pathc + pglob->gl_offs + nfound + 1)
			     * sizeof (char *));
      if (new_gl_pathv == NULL)
	goto memory_error;
      pglob->gl_pathv = new_gl_pathv;

      for (; names != NULL; names = names->next)
	pglob->gl_pathv[pglob->gl_offs + pglob->gl_pathc++] = names->name;
      pglob->gl_pathv[pglob->gl_offs + pglob->gl_pathc] = NULL;

      pglob->gl_flags = flags;
    }

  save = errno;
  if (stream != NULL)
    {
      if (flags & GLOB_ALTDIRFUNC)
	(*pglob->gl_closedir) (stream);
      else
	closedir (stream);
    }
  __set_errno (save);

  return nfound == 0 ? GLOB_NOMATCH : 0;

 memory_error:
  {
    int save2 = errno;
    if (flags & GLOB_ALTDIRFUNC)
      (*pglob->gl_closedir) (stream);
    else
      closedir (stream);
    __set_errno (save2);
  }
  while (names != NULL)
    {
      free (names->name);
      names = names->next;
    }
  return GLOB_NOSPACE;
}

/* Do glob searching for PATTERN, placing results in PGLOB.
   The bits defined above may be set in FLAGS.
   If a directory cannot be opened or read and ERRFUNC is not nil,
   it is called with the pathname that caused the error, and the
   `errno' value from the failing call; if it returns non-zero
   `glob' returns GLOB_ABEND; if it returns zero, the error is ignored.
   If memory cannot be allocated for PGLOB, GLOB_NOSPACE is returned.
   Otherwise, `glob' returns zero.  */
int
glob (
     const char *pattern,
     int flags,
     int (*errfunc) (const char *, int),
     glob_t *pglob)
{
  const char *filename;
  const char *dirname;
  size_t dirlen;
  int status;
  size_t oldcount;

  if (pattern == NULL || pglob == NULL || (flags & ~__GLOB_FLAGS) != 0)
    {
      __set_errno (EINVAL);
      return -1;
    }


  if (!(flags & GLOB_DOOFFS))
    /* Have to do this so `globfree' knows where to start freeing.  It
       also makes all the code that uses gl_offs simpler. */
    pglob->gl_offs = 0;

#if defined ENABLE_GLOB_BRACE_EXPANSION
  if (flags & GLOB_BRACE)
    {
      const char *begin;

      if (flags & GLOB_NOESCAPE)
	begin = strchr (pattern, '{');
      else
	{
	  begin = pattern;
	  while (1)
	    {
	      if (*begin == '\0')
		{
		  begin = NULL;
		  break;
		}

	      if (*begin == '\\' && begin[1] != '\0')
		++begin;
	      else if (*begin == '{')
		break;

	      ++begin;
	    }
	}

      if (begin != NULL)
	{
	  /* Allocate working buffer large enough for our work.  Note that
	    we have at least an opening and closing brace.  */
	  size_t firstc;
	  char *alt_start;
	  const char *p;
	  const char *next;
	  const char *rest;
	  size_t rest_len;
	  char onealt[strlen (pattern) - 1];

	  /* We know the prefix for all sub-patterns.  */
	  alt_start = mempcpy (onealt, pattern, begin - pattern);

	  /* Find the first sub-pattern and at the same time find the
	     rest after the closing brace.  */
	  next = __next_brace_sub (begin + 1, flags);
	  if (next == NULL)
	    {
	      /* It is an illegal expression.  */
	      return glob (pattern, flags & ~GLOB_BRACE, errfunc, pglob);
	    }

	  /* Now find the end of the whole brace expression.  */
	  rest = next;
	  while (*rest != '}')
	    {
	      rest = __next_brace_sub (rest + 1, flags);
	      if (rest == NULL)
		{
		  /* It is an illegal expression.  */
		  return glob (pattern, flags & ~GLOB_BRACE, errfunc, pglob);
		}
	    }
	  /* Please note that we now can be sure the brace expression
	     is well-formed.  */
	  rest_len = strlen (++rest) + 1;

	  /* We have a brace expression.  BEGIN points to the opening {,
	     NEXT points past the terminator of the first element, and END
	     points past the final }.  We will accumulate result names from
	     recursive runs for each brace alternative in the buffer using
	     GLOB_APPEND.  */

	  if (!(flags & GLOB_APPEND))
	    {
	      /* This call is to set a new vector, so clear out the
		 vector so we can append to it.  */
	      pglob->gl_pathc = 0;
	      pglob->gl_pathv = NULL;
	    }
	  firstc = pglob->gl_pathc;

	  p = begin + 1;
	  while (1)
	    {
	      int result;

	      /* Construct the new glob expression.  */
	      mempcpy (mempcpy (alt_start, p, next - p), rest, rest_len);

	      result = glob (onealt,
			     ((flags & ~(GLOB_NOCHECK | GLOB_NOMAGIC))
			      | GLOB_APPEND), errfunc, pglob);

	      /* If we got an error, return it.  */
	      if (result && result != GLOB_NOMATCH)
		{
		  if (!(flags & GLOB_APPEND))
		    {
		      globfree (pglob);
		      pglob->gl_pathc = 0;
		    }
		  return result;
		}

	      if (*next == '}')
		/* We saw the last entry.  */
		break;

	      p = next + 1;
	      next = __next_brace_sub (p, flags);
	      /* assert (next != NULL); */
	    }


	  if (pglob->gl_pathc != firstc)
	    /* We found some entries.  */
	    return 0;
	  else if (!(flags & (GLOB_NOCHECK|GLOB_NOMAGIC)))
	    return GLOB_NOMATCH;
	}
    }
#endif

  /* Find the filename.  */
  filename = strrchr (pattern, '/');
  if (filename == NULL)
    {
      /* This can mean two things: a simple name or "~name".  The latter
	 case is nothing but a notation for a directory.  */
      if ((flags & (GLOB_TILDE|GLOB_TILDE_CHECK)) && pattern[0] == '~')
	{
	  dirname = pattern;
	  dirlen = strlen (pattern);

	  /* Set FILENAME to NULL as a special flag.  This is ugly but
	     other solutions would require much more code.  We test for
	     this special case below.  */
	  filename = NULL;
	}
      else
	{
	  filename = pattern;
	  dirname = ".";
	  dirlen = 0;
	}
    }
  else if (filename == pattern)
    {
      /* "/pattern".  */
      dirname = "/";
      dirlen = 1;
      ++filename;
    }
  else
    {
      char *newp;
      dirlen = filename - pattern;
      newp = (char *) alloca (dirlen + 1);
      *((char *) mempcpy (newp, pattern, dirlen)) = '\0';
      dirname = newp;
      ++filename;

      if (filename[0] == '\0'
	  && dirlen > 1)
	/* "pattern/".  Expand "pattern", appending slashes.  */
	{
	  int val = glob (dirname, flags | GLOB_MARK, errfunc, pglob);
	  if (val == 0)
	    pglob->gl_flags = ((pglob->gl_flags & ~GLOB_MARK)
			       | (flags & GLOB_MARK));
	  return val;
	}
    }

  if (!(flags & GLOB_APPEND))
    {
      pglob->gl_pathc = 0;
      if (!(flags & GLOB_DOOFFS))
        pglob->gl_pathv = NULL;
      else
	{
	  size_t i;
	  pglob->gl_pathv = (char **) malloc ((pglob->gl_offs + 1)
					      * sizeof (char *));
	  if (pglob->gl_pathv == NULL)
	    return GLOB_NOSPACE;

	  for (i = 0; i <= pglob->gl_offs; ++i)
	    pglob->gl_pathv[i] = NULL;
	}
    }

  oldcount = pglob->gl_pathc + pglob->gl_offs;

#if defined ENABLE_GLOB_TILDE_EXPANSION
  if ((flags & (GLOB_TILDE|GLOB_TILDE_CHECK)) && dirname[0] == '~')
    {
      if (dirname[1] == '\0' || dirname[1] == '/')
	{
	  /* Look up home directory.  */
	  const char *home_dir = getenv ("HOME");
	  if (home_dir == NULL || home_dir[0] == '\0')
	    {
	      int success;
	      char *name;
# define GET_LOGIN_NAME_MAX()	sysconf (_SC_LOGIN_NAME_MAX)
	      size_t buflen = GET_LOGIN_NAME_MAX () + 1;

	      if (buflen == 0)
		/* `sysconf' does not support _SC_LOGIN_NAME_MAX.  Try
		   a moderate value.  */
		buflen = 20;
	      name = (char *) alloca (buflen);

	      success = getlogin_r (name, buflen) == 0;
	      if (success)
		{
		  struct passwd *p;
# define GETPW_R_SIZE_MAX()	sysconf (_SC_GETPW_R_SIZE_MAX)
		  long int pwbuflen = GETPW_R_SIZE_MAX ();
		  char *pwtmpbuf;
		  struct passwd pwbuf;
		  int save = errno;

		  pwtmpbuf = (char *) alloca (pwbuflen);

		  while (getpwnam_r (name, &pwbuf, pwtmpbuf, pwbuflen, &p)
			 != 0)
		    {
		      if (errno != ERANGE)
			{
			  p = NULL;
			  break;
			}
		      pwtmpbuf = extend_alloca (pwtmpbuf, pwbuflen,
						2 * pwbuflen);
		      __set_errno (save);
		    }
		  if (p != NULL)
		    home_dir = p->pw_dir;
		}
	    }
	  if (home_dir == NULL || home_dir[0] == '\0')
	    {
	      if (flags & GLOB_TILDE_CHECK)
		return GLOB_NOMATCH;
	      else
		home_dir = "~"; /* No luck.  */
	    }
	  /* Now construct the full directory.  */
	  if (dirname[1] == '\0')
	    dirname = home_dir;
	  else
	    {
	      char *newp;
	      size_t home_len = strlen (home_dir);
	      newp = (char *) alloca (home_len + dirlen);
	      mempcpy (mempcpy (newp, home_dir, home_len),
		       &dirname[1], dirlen);
	      dirname = newp;
	    }
	}
      else
	{
	  char *end_name = strchr (dirname, '/');
	  const char *user_name;
	  const char *home_dir;

	  if (end_name == NULL)
	    user_name = dirname + 1;
	  else
	    {
	      char *newp;
	      newp = (char *) alloca (end_name - dirname);
	      *((char *) mempcpy (newp, dirname + 1, end_name - dirname))
		= '\0';
	      user_name = newp;
	    }

	  /* Look up specific user's home directory.  */
	  {
	    struct passwd *p;
	    long int buflen = GETPW_R_SIZE_MAX ();
	    char *pwtmpbuf;
	    struct passwd pwbuf;
	    int save = errno;

	    pwtmpbuf = (char *) alloca (buflen);

	    while (getpwnam_r (user_name, &pwbuf, pwtmpbuf, buflen, &p) != 0)
	      {
		if (errno != ERANGE)
		  {
		    p = NULL;
		    break;
		  }
		pwtmpbuf = extend_alloca (pwtmpbuf, buflen, 2 * buflen);
		__set_errno (save);
	      }
	    if (p != NULL)
	      home_dir = p->pw_dir;
	    else
	      home_dir = NULL;
	  }
	  /* If we found a home directory use this.  */
	  if (home_dir != NULL)
	    {
	      char *newp;
	      size_t home_len = strlen (home_dir);
	      size_t rest_len = end_name == NULL ? 0 : strlen (end_name);
	      newp = (char *) alloca (home_len + rest_len + 1);
	      *((char *) mempcpy (mempcpy (newp, home_dir, home_len),
				  end_name, rest_len)) = '\0';
	      dirname = newp;
	    }
	  else
	    if (flags & GLOB_TILDE_CHECK)
	      /* We have to regard it as an error if we cannot find the
		 home directory.  */
	      return GLOB_NOMATCH;
	}
    }

  /* Now test whether we looked for "~" or "~NAME".  In this case we
     can give the answer now.  */
  if (filename == NULL)
    {
      struct stat st;
      struct_stat64 st64;

      /* Return the directory if we don't check for error or if it exists.  */
      if ((flags & GLOB_NOCHECK)
	  || (((flags & GLOB_ALTDIRFUNC)
	       ? ((*pglob->gl_stat) (dirname, &st) == 0
		  && S_ISDIR (st.st_mode))
	       : (__stat64 (dirname, &st64) == 0 && S_ISDIR (st64.st_mode)))))
	{
	  int newcount = pglob->gl_pathc + pglob->gl_offs;
	  char **new_gl_pathv;

	  new_gl_pathv
	    = (char **) realloc (pglob->gl_pathv,
				 (newcount + 1 + 1) * sizeof (char *));
	  if (new_gl_pathv == NULL)
	    {
	    nospace:
	      free (pglob->gl_pathv);
	      pglob->gl_pathv = NULL;
	      pglob->gl_pathc = 0;
	      return GLOB_NOSPACE;
	    }
	  pglob->gl_pathv = new_gl_pathv;

	   pglob->gl_pathv[newcount] = strdup (dirname);
	  if (pglob->gl_pathv[newcount] == NULL)
	    goto nospace;
	  pglob->gl_pathv[++newcount] = NULL;
	  ++pglob->gl_pathc;
	  pglob->gl_flags = flags;

	  return 0;
	}

      /* Not found.  */
      return GLOB_NOMATCH;
    }
#endif

  if (glob_pattern_p (dirname, !(flags & GLOB_NOESCAPE)))
    {
      /* The directory name contains metacharacters, so we
	 have to glob for the directory, and then glob for
	 the pattern in each directory found.  */
      glob_t dirs;
      size_t i;

      if ((flags & GLOB_ALTDIRFUNC) != 0)
	{
	  /* Use the alternative access functions also in the recursive
	     call.  */
	  dirs.gl_opendir = pglob->gl_opendir;
	  dirs.gl_readdir = pglob->gl_readdir;
	  dirs.gl_closedir = pglob->gl_closedir;
	  dirs.gl_stat = pglob->gl_stat;
	  dirs.gl_lstat = pglob->gl_lstat;
	}

      status = glob (dirname,
		     ((flags & (GLOB_ERR | GLOB_NOCHECK | GLOB_NOESCAPE
				| GLOB_ALTDIRFUNC))
		      | GLOB_NOSORT | GLOB_ONLYDIR),
		     errfunc, &dirs);
      if (status != 0)
	return status;

      /* We have successfully globbed the preceding directory name.
	 For each name we found, call glob_in_dir on it and FILENAME,
	 appending the results to PGLOB.  */
      for (i = 0; i < dirs.gl_pathc; ++i)
	{
	  int old_pathc;

	  old_pathc = pglob->gl_pathc;
	  status = glob_in_dir (filename, dirs.gl_pathv[i],
				((flags | GLOB_APPEND)
				 & ~(GLOB_NOCHECK | GLOB_NOMAGIC)),
				errfunc, pglob);
	  if (status == GLOB_NOMATCH)
	    /* No matches in this directory.  Try the next.  */
	    continue;

	  if (status != 0)
	    {
	      globfree (&dirs);
	      globfree (pglob);
	      pglob->gl_pathc = 0;
	      return status;
	    }

	  /* Stick the directory on the front of each name.  */
	  if (__prefix_array (dirs.gl_pathv[i],
			    &pglob->gl_pathv[old_pathc + pglob->gl_offs],
			    pglob->gl_pathc - old_pathc))
	    {
	      globfree (&dirs);
	      globfree (pglob);
	      pglob->gl_pathc = 0;
	      return GLOB_NOSPACE;
	    }
	}

      flags |= GLOB_MAGCHAR;

      /* We have ignored the GLOB_NOCHECK flag in the `glob_in_dir' calls.
	 But if we have not found any matching entry and the GLOB_NOCHECK
	 flag was set we must return the input pattern itself.  */
      if (pglob->gl_pathc + pglob->gl_offs == oldcount)
	{
	  /* No matches.  */
	  if (flags & GLOB_NOCHECK)
	    {
	      int newcount = pglob->gl_pathc + pglob->gl_offs;
	      char **new_gl_pathv;

	      new_gl_pathv = (char **) realloc (pglob->gl_pathv,
						(newcount + 2)
						* sizeof (char *));
	      if (new_gl_pathv == NULL)
		{
		  globfree (&dirs);
		  return GLOB_NOSPACE;
		}
	      pglob->gl_pathv = new_gl_pathv;

	      pglob->gl_pathv[newcount] = strdup (pattern);
	      if (pglob->gl_pathv[newcount] == NULL)
		{
		  globfree (&dirs);
		  globfree (pglob);
		  pglob->gl_pathc = 0;
		  return GLOB_NOSPACE;
		}

	      ++pglob->gl_pathc;
	      ++newcount;

	      pglob->gl_pathv[newcount] = NULL;
	      pglob->gl_flags = flags;
	    }
	  else
	    {
	      globfree (&dirs);
	      return GLOB_NOMATCH;
	    }
	}

      globfree (&dirs);
    }
  else
    {
      int old_pathc = pglob->gl_pathc;

      status = glob_in_dir (filename, dirname, flags, errfunc, pglob);
      if (status != 0)
	return status;

      if (dirlen > 0)
	{
	  /* Stick the directory on the front of each name.  */
	  if (__prefix_array (dirname,
			    &pglob->gl_pathv[old_pathc + pglob->gl_offs],
			    pglob->gl_pathc - old_pathc))
	    {
	      globfree (pglob);
	      pglob->gl_pathc = 0;
	      return GLOB_NOSPACE;
	    }
	}
    }

  if (flags & GLOB_MARK)
    {
      /* Append slashes to directory names.  */
      size_t i;
      struct stat st;
      struct_stat64 st64;

      for (i = oldcount; i < pglob->gl_pathc + pglob->gl_offs; ++i)
	if (((flags & GLOB_ALTDIRFUNC)
	     ? ((*pglob->gl_stat) (pglob->gl_pathv[i], &st) == 0
		&& S_ISDIR (st.st_mode))
	     : (__stat64 (pglob->gl_pathv[i], &st64) == 0
		&& S_ISDIR (st64.st_mode))))
	  {
	    size_t len = strlen (pglob->gl_pathv[i]) + 2;
	    char *new = realloc (pglob->gl_pathv[i], len);
	    if (new == NULL)
	      {
		globfree (pglob);
		pglob->gl_pathc = 0;
		return GLOB_NOSPACE;
	      }
	    strcpy (&new[len - 2], "/");
	    pglob->gl_pathv[i] = new;
	  }
    }

  if (!(flags & GLOB_NOSORT))
    {
      /* Sort the vector.  */
      qsort (&pglob->gl_pathv[oldcount],
	     pglob->gl_pathc + pglob->gl_offs - oldcount,
	     sizeof (char *), __collated_compare);
    }

  return 0;
}
#ifdef COMPILE_GLOB64
libc_hidden_def(glob64)
#else
libc_hidden_def(glob)
#endif


/* Free storage allocated in PGLOB by a previous `glob' call.  */
void
globfree (register glob_t *pglob)
{
  if (pglob->gl_pathv != NULL)
    {
      size_t i;
      for (i = 0; i < pglob->gl_pathc; ++i)
	if (pglob->gl_pathv[pglob->gl_offs + i] != NULL)
	  free (pglob->gl_pathv[pglob->gl_offs + i]);
      free (pglob->gl_pathv);
      pglob->gl_pathv = NULL;
    }
}
#ifdef COMPILE_GLOB64
libc_hidden_def(globfree64)
#else
libc_hidden_def(globfree)
#endif
