dnl Checks for keyutils library and headers
dnl
AC_DEFUN([AC_KEYUTILS], [

  dnl Check for libkeyutils; do not add to LIBS if found
  AC_CHECK_LIB([keyutils], [keyctl_instantiate], [LIBKEYUTILS=-lkeyutils], ,)
  AC_SUBST(LIBKEYUTILS)

  AC_CHECK_HEADERS([keyutils.h])

])dnl
