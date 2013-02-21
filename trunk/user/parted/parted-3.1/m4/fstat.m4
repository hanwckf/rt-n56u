# fstat.m4 serial 1
dnl Copyright (C) 2011-2012 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FSTAT],
[
  AC_REQUIRE([gl_SYS_STAT_H_DEFAULTS])
  AC_REQUIRE([gl_MSVC_INVAL])
  if test $HAVE_MSVC_INVALID_PARAMETER_HANDLER = 1; then
    REPLACE_FSTAT=1
  fi
  dnl Replace fstat() for supporting the gnulib-defined open() on directories.
  m4_ifdef([gl_FUNC_FCHDIR], [
    gl_TEST_FCHDIR
    if test $HAVE_FCHDIR = 0 \
       && test "$gl_cv_func_open_directory_works" != yes; then
      REPLACE_FSTAT=1
    fi
  ])
])

# Prerequisites of lib/fstat.c.
AC_DEFUN([gl_PREREQ_FSTAT],
[
  AC_REQUIRE([AC_C_INLINE])
])
