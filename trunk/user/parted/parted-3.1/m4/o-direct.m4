#serial 2
# Find a directory in which a disk-simulating file is usable by parted.
# The problem is that on systems supporting O_DIRECT, open with O_DIRECT
# fails for some file system types (e.g., tmpfs on linux-2.6.21).

# Copyright (C) 2007-2012 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# From Jim Meyering.

# Set PARTED_USABLE_TEST_DIR to the name of the first usable directory
# from the list below.  If none is usable, set it to the empty string.
# Consider $TMPDIR only if it specifies an absolute name, and that
# name contains no shell meta-character.  Likewise for $HOME.

# This code is pretty picky.  The chosen partition must support aligned reads
# and writes in blocks of size 512B and 4KB to a file descriptor opened with
# O_RDWR|O_DIRECT.  Reiserfs doesn't support 512-byte reads.  On tmpfs,
# the open fails.

# The candidate directories:
#   . $HOME $TMPDIR /tmp /var/tmp /dev/shm
AC_DEFUN([parted_FIND_USABLE_TEST_DIR],
[
  AC_CACHE_CHECK([for a usable (O_DIRECT-supporting) temporary dir],
    [parted_cv_func_open_O_DIRECT_temp_dir],
    [
      # First of all, if there is no O_DIRECT definition, use ".",
      # and skip the run-test.
      AC_EGREP_CPP([frobnozzle], [
#include <fcntl.h>
#ifdef O_DIRECT
frobnozzle
#endif
		  ], pe_have_O_DIRECT=yes, pe_have_O_DIRECT=no)
      if test $pe_have_O_DIRECT = no; then
	  # With no O_DIRECT definition, "." is fine.
	  pe_cand_dirs=.
      else
	  pe_cand_dirs=.
	  for pe_dir in "$HOME" "$TMPDIR"; do
	      case $pe_dir in
	      /tmp) ;;
	      /var/tmp) ;;
	      /dev/shm) ;;
	      /*) case $pe_dir in
		  # Accept $HOME or $TMP only if the value is nice and boring.
		  *[^/a-zA-Z0-9_.-]*) ;;
		  *) pe_cand_dirs="$pe_cand_dirs $pe_dir";;
		  esac
	      esac
	  done

	  case $PARTED_TMPDIR in
	      *[^/a-zA-Z0-9_.-]*) ;;
	      *) pe_cand_dirs="$PARTED_TMPDIR $pe_cand_dirs";;
	  esac

	  # This is the list of candidate directories.
	  pe_cand_dirs="$pe_cand_dirs /tmp /var/tmp /dev/shm"

	  PARTED_CANDIDATE_DIRS=$pe_cand_dirs
	  export PARTED_CANDIDATE_DIRS

	  AC_RUN_IFELSE(
	    [AC_LANG_SOURCE(
	      [[
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LOGICAL_BLOCK_SIZE 4096
static char g_buf[2 * MAX_LOGICAL_BLOCK_SIZE];

static inline void *
ptr_align (void const *ptr, size_t alignment)
{
  char const *p0 = ptr;
  char const *p1 = p0 + alignment - 1;
  return (void *) (p1 - (size_t) p1 % alignment);
}

static int
create_input_file (char const *file, char const *buf, size_t n_bytes)
{
  int fd = open (file, O_CREAT | O_WRONLY, 0600);
  if (fd < 0)
    return 1;
  if (write (fd, buf, n_bytes) != n_bytes)
    {
      close (fd);
      return 1;
    }
  return !! close (fd);
}

static int
try_o_direct (char const *file, size_t block_size)
{
  char *p = ptr_align (g_buf, MAX_LOGICAL_BLOCK_SIZE);
  int fd;

  if (!(p + block_size < g_buf + sizeof g_buf))
    return 4;

  fd = open (file, O_RDWR | O_DIRECT);
  if (fd < 0)
    return 1;

  if (write (fd, p, block_size) != block_size)
    return 1;

  if (lseek (fd, 0, SEEK_SET) != 0)
    return 1;

  if (read (fd, p, block_size) != block_size)
    return 1;

  return !! close (fd);
}

#undef stpcpy
#define stpcpy(a, b) my_stpcpy (a, b)
static char *
my_stpcpy (char *dest, const char *src)
{
  char *d = dest;
  const char *s = src;
  do *d++ = *s; while (*s++ != '\0');
  return d - 1;
}

/* The base name of the file we'll create in the mkdtemp-returned
   temporary directory.  */
#define BASENAME "x"

/* Return 0 upon failure, else the 1-based index of the first
   useful directory name from PARTED_CANDIDATE_DIRS.  */
int
main ()
{
  char const *env_dirs;
  char *dirs;
  char *dir;
  unsigned int n;
  int found = 0;
  size_t dirs_len;

  if ((env_dirs = getenv ("PARTED_CANDIDATE_DIRS")) == NULL)
    return 0;

  dirs_len = strlen (env_dirs);
  if ((dirs = strndup (env_dirs, dirs_len)) == NULL)
    return 0;
  dir = dirs;

  for (n = 1; ; n++)
    {
      size_t dirname_len;
      char *space;

      /* Skip any leading spaces.  */
      while (*dir == ' ')
	++dir;

      space = strchr (dir, ' ');
      if (space)
	{
	  *space = '\0';
	  dirname_len = space - dir;
	}
      else
	{
	  dirname_len = strlen (dir);
	}

      if (dirname_len != 0)
	{
	  /* Create an mkdtemp template starting with dir.  */
	  char *tmp;
	  char *endp;
	  char const *base = "partedOD.XXXXXX";
	  /* Allocate enough space not just for the dir name, but
	     also for the name of the file to create within it.  */
	  char *template = malloc (dirname_len + 1 + strlen (base)
				   + 1 + strlen (BASENAME) + 1);
	  if (template != NULL
	      && (endp = stpcpy (stpcpy (stpcpy (template, dir), "/"), base))
	      && (tmp = mkdtemp (template)) != NULL)
	    {
	      /* Append "/BASENAME" to create the file name.  */
	      stpcpy (stpcpy (endp, "/"), BASENAME);

	      if (create_input_file (tmp, g_buf, sizeof g_buf) == 0
		  && try_o_direct (tmp, 512) == 0
		  && try_o_direct (tmp, MAX_LOGICAL_BLOCK_SIZE) == 0)
		found = 1;

	      unlink (tmp); /* ignore failure */
	      *endp = '\0';
	      rmdir (tmp); /* ignore failure */
	    }
	  free (template);
	}

      if (found)
	break;

      dir += dirname_len + 1;
      if (dirs + dirs_len < dir)
	{
	  n = 0;
	  break;
	}
    }
  free (dirs);

  return n;
}
	      ]])],
	    # If the above program exits with status 0, then
	    # there it found no useful directory.  Use ".".
	    [parted_cv_func_open_O_DIRECT_temp_dir=.],

	    # It found one.  The exit status is an index into the list.
	    # We also run this code when the program fails to compile or
	    # to link, as will happen on systems without a mkdtemp function.
	    [pe_err=$?; set _ $pe_cand_dirs; shift
	      eval parted_cv_func_open_O_DIRECT_temp_dir='$'$pe_err],

	    # When cross-compiling, use ".".
	    [parted_cv_func_open_O_DIRECT_temp_dir=.]
	    )
      fi
    ])
  PARTED_USABLE_TEST_DIR=$parted_cv_func_open_O_DIRECT_temp_dir
  AC_SUBST([PARTED_USABLE_TEST_DIR])

  # If the result is ".", don't cache it.  The next user of
  # the cache may well be running from a different file system.
  dnl Here, I'm using "$as_unset", which is a non-published (i.e., internal)
  dnl part of autoconf, but we don't expect its name to change any time soon.
  dnl and by then, it'll probably be ok to use "unset" all by itself.
  if test "$parted_cv_func_open_O_DIRECT_temp_dir" = .; then
    $as_unset parted_cv_func_open_O_DIRECT_temp_dir
  fi
])
