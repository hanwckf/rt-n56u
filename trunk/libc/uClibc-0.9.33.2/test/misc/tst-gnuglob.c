/* Test the GNU extensions in glob which allow the user to provide callbacks
   for the filesystem access functions.
   Copyright (C) 2001-2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2001.

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

#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


// #define DEBUG
#ifdef DEBUG
# define PRINTF(fmt, args...) printf (fmt, ##args)
#else
# define PRINTF(fmt, args...)
#endif


#ifdef GLOB_ALTDIRFUNC
static struct
{
  const char *name;
  int level;
  int type;
} filesystem[] =
{
  { ".", 1, DT_DIR },
  { "..", 1, DT_DIR },
  { "file1lev1", 1, DT_REG },
  { "file2lev1", 1, DT_UNKNOWN },
  { "dir1lev1", 1, DT_UNKNOWN },
    { ".", 2, DT_DIR },
    { "..", 2, DT_DIR },
    { "file1lev2", 2, DT_REG },
    { "dir1lev2", 2, DT_DIR },
      { ".", 3, DT_DIR },
      { "..", 3, DT_DIR },
    { "dir2lev2", 2, DT_DIR },
      { ".", 3, DT_DIR },
      { "..", 3, DT_DIR },
      { ".foo", 3, DT_REG },
      { "dir1lev3", 3, DT_DIR },
        { ".", 4, DT_DIR },
        { "..", 4, DT_DIR },
        { "file1lev4", 4, DT_REG },
      { "file1lev3", 3, DT_REG },
      { "file2lev3", 3, DT_REG },
    { "file2lev2", 2, DT_REG },
    { "file3lev2", 2, DT_REG },
    { "dir3lev2", 2, DT_DIR },
      { ".", 3, DT_DIR },
      { "..", 3, DT_DIR },
      { "file3lev3", 3, DT_REG },
      { "file4lev3", 3, DT_REG },
  { "dir2lev1", 1, DT_DIR },
    { ".", 2, DT_DIR },
    { "..", 2, DT_DIR },
    { "dir1lev2", 2, DT_UNKNOWN },
      { ".", 3, DT_DIR },
      { "..", 3, DT_DIR },
      { ".foo", 3, DT_REG },
      { ".dir", 3, DT_DIR },
        { ".", 4, DT_DIR },
        { "..", 4, DT_DIR },
        { "hidden", 4, DT_REG }
};
#define nfiles (sizeof (filesystem) / sizeof (filesystem[0]))


typedef struct
{
  int level;
  int idx;
  struct dirent d;
  char room_for_dirent[NAME_MAX];
} my_DIR;


static long int
find_file (const char *s)
{
  int level = 1;
  long int idx = 0;

  if (strcmp (s, ".") == 0)
    return 0;

  if (s[0] == '.' && s[1] == '/')
    s += 2;

  while (*s != '\0')
    {
      char *endp = strchrnul (s, '/');

      PRINTF ("looking for %.*s, level %d\n", (int) (endp - s), s, level);

      while (idx < nfiles && filesystem[idx].level >= level)
	{
	  if (filesystem[idx].level == level
	      && memcmp (s, filesystem[idx].name, endp - s) == 0
	      && filesystem[idx].name[endp - s] == '\0')
	    break;
	  ++idx;
	}

      if (idx == nfiles || filesystem[idx].level < level)
	{
	  errno = ENOENT;
	  return -1;
	}

      if (*endp == '\0')
	return idx + 1;

      if (filesystem[idx].type != DT_DIR
	  && (idx + 1 >= nfiles
	      || filesystem[idx].level >= filesystem[idx + 1].level))
	{
	  errno = ENOTDIR;
	  return -1;
	}

      ++idx;

      s = endp + 1;
      ++level;
    }

  errno = ENOENT;
  return -1;
}


static void *
my_opendir (const char *s)
{
  long int idx = find_file (s);
  my_DIR *dir;


  if (idx == -1)
    {
      PRINTF ("my_opendir(\"%s\") == NULL\n", s);
      return NULL;
    }

  dir = (my_DIR *) malloc (sizeof (my_DIR));
  if (dir == NULL)
    error (EXIT_FAILURE, errno, "cannot allocate directory handle");

  dir->level = filesystem[idx].level;
  dir->idx = idx;

  PRINTF ("my_opendir(\"%s\") == { level: %d, idx: %ld }\n",
	  s, filesystem[idx].level, idx);

  return dir;
}


static struct dirent *
my_readdir (void *gdir)
{
  my_DIR *dir = gdir;

  if (dir->idx == -1)
    {
      PRINTF ("my_readdir ({ level: %d, idx: %ld }) = NULL\n",
	      dir->level, (long int) dir->idx);
      return NULL;
    }

  while (dir->idx < nfiles && filesystem[dir->idx].level > dir->level)
    ++dir->idx;

  if (dir->idx == nfiles || filesystem[dir->idx].level < dir->level)
    {
      dir->idx = -1;
      PRINTF ("my_readdir ({ level: %d, idx: %ld }) = NULL\n",
	      dir->level, (long int) dir->idx);
      return NULL;
    }

  dir->d.d_ino = dir->idx;

#ifdef _DIRENT_HAVE_D_TYPE
  dir->d.d_type = filesystem[dir->idx].type;
#endif

  strcpy (dir->d.d_name, filesystem[dir->idx].name);

#ifdef _DIRENT_HAVE_D_TYPE
  PRINTF ("my_readdir ({ level: %d, idx: %ld }) = { d_ino: %ld, d_type: %d, d_name: \"%s\" }\n",
	  dir->level, (long int) dir->idx, dir->d.d_ino, dir->d.d_type,
	  dir->d.d_name);
#else
  PRINTF ("my_readdir ({ level: %d, idx: %ld }) = { d_ino: %ld, d_name: \"%s\" }\n",
	  dir->level, (long int) dir->idx, dir->d.d_ino,
	  dir->d.d_name);
#endif

  ++dir->idx;

  return &dir->d;
}


static void
my_closedir (void *dir)
{
  PRINTF ("my_closedir ()\n");
  free (dir);
}


/* We use this function for lstat as well since we don't have any.  */
static int
my_stat (const char *name, struct stat *st)
{
  long int idx = find_file (name);

  if (idx == -1)
    {
      PRINTF ("my_stat (\"%s\", ...) = -1 (%s)\n", name, strerror (errno));
      return -1;
    }

  memset (st, '\0', sizeof (*st));

  if (filesystem[idx].type == DT_UNKNOWN)
    st->st_mode = DTTOIF (idx + 1 < nfiles
			  && filesystem[idx].level < filesystem[idx + 1].level
			  ? DT_DIR : DT_REG) | 0777;
  else
    st->st_mode = DTTOIF (filesystem[idx].type) | 0777;

  PRINTF ("my_stat (\"%s\", { st_mode: %o }) = 0\n", name, st->st_mode);

  return 0;
}


static const char *glob_errstring[] =
{
  [GLOB_NOSPACE] = "out of memory",
  [GLOB_ABORTED] = "read error",
  [GLOB_NOMATCH] = "no matches found"
};
#define nglob_errstring (sizeof (glob_errstring) / sizeof (glob_errstring[0]))


static const char *
flagstr (int flags)
{
  const char *strs[] =
  {
    "GLOB_ERR", "GLOB_MARK", "GLOB_NOSORT", "GLOB_DOOFSS", "GLOB_NOCHECK",
    "GLOB_APPEND", "GLOB_NOESCAPE", "GLOB_PERIOD", "GLOB_MAGCHAR",
    "GLOB_ALTDIRFUNC", "GLOB_BRACE", "GLOB_NOMAGIC", "GLOB_TILDE",
    "GLOB_ONLYDIR", "GLOB_TILDECHECK"
  };
#define nstrs (sizeof (strs) / sizeof (strs[0]))
  static char buf[100];
  char *cp = buf;
  int cnt;

  for (cnt = 0; cnt < nstrs; ++cnt)
    if (flags & (1 << cnt))
      {
	flags &= ~(1 << cnt);
	if (cp != buf)
	  *cp++ = '|';
	cp = stpcpy (cp, strs[cnt]);
      }

  if (flags != 0)
    {
      if (cp != buf)
	*cp++ = '|';
      sprintf (cp, "%#x", flags);
    }

  return buf;
}


static int
test_result (const char *fmt, int flags, glob_t *gl, const char *str[])
{
  size_t cnt;
  int result = 0;

  printf ("results for glob (\"%s\", %s)\n", fmt, flagstr (flags));
  for (cnt = 0; cnt < gl->gl_pathc && str[cnt] != NULL; ++cnt)
    {
      int ok = strcmp (gl->gl_pathv[cnt], str[cnt]) == 0;
      const char *errstr = "";

      if (! ok)
	{
	  size_t inner;

	  for (inner = 0; str[inner] != NULL; ++inner)
	    if (strcmp (gl->gl_pathv[cnt], str[inner]) == 0)
	      break;

	  if (str[inner] == NULL)
	    errstr =  ok ? "" : " *** WRONG";
	  else
	    errstr = ok ? "" : " * wrong position";

	  result = 1;
	}

      printf ("  %s%s\n", gl->gl_pathv[cnt], errstr);
    }
  puts ("");

  if (str[cnt] != NULL || cnt < gl->gl_pathc)
    {
      puts ("  *** incorrect number of entries");
      result = 1;
    }

  return result;
}


int
main (void)
{
  glob_t gl;
  int errval;
  int result = 0;
  const char *fmt;
  int flags;

  memset (&gl, '\0', sizeof (gl));

  gl.gl_closedir = my_closedir;
  gl.gl_readdir = my_readdir;
  gl.gl_opendir = my_opendir;
  gl.gl_lstat = my_stat;
  gl.gl_stat = my_stat;

#define test(a, b, c...) \
  fmt = a;								      \
  flags = b;								      \
  errval = glob (fmt, flags, NULL, &gl);				      \
  if (errval != 0)							      \
    {									      \
      printf ("glob (\"%s\", %s) failed: %s\n", fmt, flagstr (flags),	      \
	      errval >= 0 && errval < nglob_errstring			      \
	      ? glob_errstring[errval] : "???");			      \
      result = 1;							      \
    }									      \
  else									      \
    result |= test_result (fmt, flags, &gl, (const char *[]) { c, NULL })

  test ("*/*/*", GLOB_ALTDIRFUNC,
	"dir1lev1/dir2lev2/dir1lev3",
	"dir1lev1/dir2lev2/file1lev3",
	"dir1lev1/dir2lev2/file2lev3",
	"dir1lev1/dir3lev2/file3lev3",
	"dir1lev1/dir3lev2/file4lev3");

  test ("*/*/*", GLOB_ALTDIRFUNC | GLOB_PERIOD,
	"dir1lev1/dir1lev2/.",
	"dir1lev1/dir1lev2/..",
	"dir1lev1/dir2lev2/.",
	"dir1lev1/dir2lev2/..",
	"dir1lev1/dir2lev2/.foo",
	"dir1lev1/dir2lev2/dir1lev3",
	"dir1lev1/dir2lev2/file1lev3",
	"dir1lev1/dir2lev2/file2lev3",
	"dir1lev1/dir3lev2/.",
	"dir1lev1/dir3lev2/..",
	"dir1lev1/dir3lev2/file3lev3",
	"dir1lev1/dir3lev2/file4lev3",
	"dir2lev1/dir1lev2/.",
	"dir2lev1/dir1lev2/..",
	"dir2lev1/dir1lev2/.dir",
	"dir2lev1/dir1lev2/.foo");

  test ("*/*/.*", GLOB_ALTDIRFUNC,
	"dir1lev1/dir1lev2/.",
	"dir1lev1/dir1lev2/..",
	"dir1lev1/dir2lev2/.",
	"dir1lev1/dir2lev2/..",
	"dir1lev1/dir2lev2/.foo",
	"dir1lev1/dir3lev2/.",
	"dir1lev1/dir3lev2/..",
	"dir2lev1/dir1lev2/.",
	"dir2lev1/dir1lev2/..",
	"dir2lev1/dir1lev2/.dir",
	"dir2lev1/dir1lev2/.foo");

  test ("*1*/*2*/.*", GLOB_ALTDIRFUNC,
	"dir1lev1/dir1lev2/.",
	"dir1lev1/dir1lev2/..",
	"dir1lev1/dir2lev2/.",
	"dir1lev1/dir2lev2/..",
	"dir1lev1/dir2lev2/.foo",
	"dir1lev1/dir3lev2/.",
	"dir1lev1/dir3lev2/..",
	"dir2lev1/dir1lev2/.",
	"dir2lev1/dir1lev2/..",
	"dir2lev1/dir1lev2/.dir",
	"dir2lev1/dir1lev2/.foo");

  test ("*1*/*1*/.*", GLOB_ALTDIRFUNC,
	"dir1lev1/dir1lev2/.",
	"dir1lev1/dir1lev2/..",
	"dir2lev1/dir1lev2/.",
	"dir2lev1/dir1lev2/..",
	"dir2lev1/dir1lev2/.dir",
	"dir2lev1/dir1lev2/.foo");

  globfree (&gl);

  return result;
}

#else
int main(void) { return 0; }
#endif
