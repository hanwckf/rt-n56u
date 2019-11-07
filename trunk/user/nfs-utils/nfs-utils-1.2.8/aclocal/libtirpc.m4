dnl Checks for TI-RPC library and headers
dnl
AC_DEFUN([AC_LIBTIRPC], [

  PKG_PROG_PKG_CONFIG([0.9.0])
  AS_IF(
    [test "$enable_tirpc" != "no"],
    [PKG_CHECK_MODULES([TIRPC], [libtirpc >= 0.2.4],
                       [LIBTIRPC="${TIRPC_LIBS}"
                        AM_CPPFLAGS="${AM_CPPFLAGS} ${TIRPC_CFLAGS}"
                        AC_DEFINE([HAVE_LIBTIRPC], [1],
                                  [Define to 1 if you have and wish to use libtirpc.])],
                       [AS_IF([test "$enable_tirpc" = "yes"],
                              [AC_MSG_ERROR([libtirpc not found.])],
                              [LIBTIRPC=""])])])

  AC_SUBST([AM_CPPFLAGS])

  AC_SUBST(LIBTIRPC)

])dnl
