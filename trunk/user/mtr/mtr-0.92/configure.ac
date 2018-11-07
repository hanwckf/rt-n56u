AC_PREREQ([2.59])
AC_INIT([mtr],
  [m4_esyscmd([build-aux/git-version-gen .tarball-version])],
  [R.E.Wolff@BitWizard.nl], [],
  [http://www.BitWizard.nl/mtr/])
AC_CONFIG_SRCDIR([ui/mtr.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE([
  1.7.9
  foreign
  subdir-objects
])

# --enable-silent-rules
m4_ifdef([AM_SILENT_RULES],
  [AM_SILENT_RULES([yes])],
  [AC_SUBST([AM_DEFAULT_VERBOSITY], [1])])

AC_CANONICAL_HOST
AC_PROG_CC

# Check pkg-config availability.
m4_ifndef([PKG_PROG_PKG_CONFIG],
  [m4_fatal(
[Could not locate the pkg-config autoconf macros.  These are usually located
in /usr/share/aclocal/pkg.m4.  If your macros are in a different location,
try setting the environment variable ACLOCAL_OPTS="-I/other/macro/dir"
before running ./bootstrap.sh again.])
])
PKG_PROG_PKG_CONFIG

AM_CONDITIONAL([CYGWIN], [test "$host_os" = cygwin])

# Check bytes in types.
AC_CHECK_SIZEOF([unsigned char], [1])
AC_CHECK_SIZEOF([unsigned short], [2])
AC_CHECK_SIZEOF([unsigned int], [4])
AC_CHECK_SIZEOF([unsigned long], [4])

# Check headers.
AC_CHECK_HEADERS([ \
  arpa/nameser_compat.h \
  curses.h \
  cursesX.h \
  error.h \
  fcntl.h \
  linux/icmp.h \
  ncurses.h \
  ncurses/curses.h \
  netinet/in.h \
  socket.h \
  sys/cdefs.h \
  sys/limits.h \
  sys/socket.h \
  stdio_ext.h \
  sys/types.h \
  sys/xti.h \
  values.h \
])

# Check functions.
AC_CHECK_FUNCS([ \
  __fpending \
  fcntl \
])

AC_CHECK_FUNC([error], [with_error=no],
  [AC_CHECK_FUNCS([verr verrx vwarn vwarnx], [with_error=yes],
    [AC_MSG_ERROR([cannot find working error printing function])
  ])
])
AM_CONDITIONAL([WITH_ERROR], [test "x$with_error" = "xyes"])

AC_CHECK_FUNC([getopt_long], [with_getopt=no], [with_getopt=yes])
AS_IF([test "x$with_getopt" = "xno"], [
  AC_DEFINE([HAVE_GETOPT], [1], [Define if libc has getopt_long])
])
AM_CONDITIONAL([WITH_GETOPT], [test "x$with_getopt" = "xyes"])

AC_CHECK_LIB([m], [floor], [], [AC_MSG_ERROR([No math library found])])

# Find GTK
AC_ARG_WITH([gtk],
  [AS_HELP_STRING([--without-gtk], [Build without the GTK+2.0 interface])],
  [], [with_gtk=yes])
AS_IF([test "x$with_gtk" = "xyes"],
  [PKG_CHECK_MODULES([GTK], [gtk+-2.0],
    [AC_DEFINE([HAVE_GTK], [1], [Define if gtk+-2.0 library available])],
    [with_gtk=no])
])
AM_CONDITIONAL([WITH_GTK], [test "x$with_gtk" = xyes])

# Find ncurses
AC_ARG_WITH([ncurses],
  [AS_HELP_STRING([--without-ncurses], [Build without the ncurses interface])],
  [], [with_ncurses=yes])
AS_IF([test "x$with_ncurses" = "xyes"],

  # Prefer ncurses over curses, if both are available.
  # (On Solaris 11.3, ncurses builds and links for us, but curses does not.)
  [AC_SEARCH_LIBS(
    [initscr], [ncursesw curses],
    [AC_DEFINE([HAVE_CURSES], [1], [Define if a curses library available])],
    [with_ncurses=no])
])
AM_CONDITIONAL([WITH_CURSES], [test "x$with_ncurses" = xyes])

# Enable ipinfo
AC_ARG_WITH([ipinfo],
  [AS_HELP_STRING([--without-ipinfo], [Do not try to use ipinfo lookup at all])],
  [], [with_ipinfo=yes])
AM_CONDITIONAL([WITH_IPINFO], [test "x$with_ipinfo" = "xyes"])
AS_IF([test "x$with_ipinfo" = "xyes"], [
  AC_DEFINE([HAVE_IPINFO], [1], [Define when ipinfo lookups are in use])
])

AC_ARG_ENABLE([ipv6],
  [AS_HELP_STRING([--disable-ipv6], [Do not enable IPv6])],
  [WANTS_IPV6=$enableval], [WANTS_IPV6=yes])

AS_IF([test "x$WANTS_IPV6" = "xyes"], [
  AC_DEFINE([ENABLE_IPV6], [1], [Define to enable IPv6])
  USES_IPV6=yes
])

AC_CHECK_FUNC([socket], [],
  [AC_CHECK_LIB([socket], [socket], [], [AC_MSG_ERROR([No socket library found])])])

AC_CHECK_FUNC([gethostbyname], [],
  [AC_CHECK_LIB([nsl], [gethostbyname], [], [AC_MSG_ERROR([No nameservice library found])])])

# Find resolver library.
AC_CHECK_FUNC([res_query], [RESOLV_LIBS=""], [
  AC_CHECK_LIB([resolv], [__res_query], [RESOLV_LIBS="-lresolv"], [
    AC_CHECK_LIB([resolv], [res_query], [RESOLV_LIBS="-lresolv"], [
      AC_CHECK_LIB([bind], [res_query], [RESOLV_LIBS="-lbind"], [
        AC_MSG_ERROR([No resolver library found])
      ])
    ])
  ])
])
dnl MacOS has res_query in libc, but needs libresolv for dn_expand().
AS_IF([test "x" = "x$RESOLV_LIBS"], [
  AC_CHECK_LIB([resolv], [dn_expand], [RESOLV_LIBS="-lresolv"])
])
AC_SUBST([RESOLV_LIBS])

# Check errno and socket data types.
AC_CHECK_DECLS([errno], [], [], [[
#include <errno.h>
#include <sys/errno.h>
  ]])

AC_CHECK_TYPE([socklen_t],
  [AC_DEFINE([HAVE_SOCKLEN_T], [], [Define if your system has socklen_t])], [],
  [[#include <netinet/in.h>
#ifdef HAVE_SOCKET_H
#include <socket.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif]])

AC_CHECK_TYPES([time_t], [], [], [[
#include <time.h>
]])

# Add C flags to display more warnings
AC_MSG_CHECKING([for C flags to get more warnings])
ac_save_CFLAGS="$CFLAGS"

AS_IF([test "x$ac_cv_c_compiler_gnu" = "xyes"], [
  dnl gcc is the easiest C compiler
  warning_CFLAGS="-Wall"
  # Check if compiler supports -Wno-pointer-sign and add it if supports
  CFLAGS_saved="$CFLAGS"
  CFLAGS="$CFLAGS -Wno-pointer-sign"
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[int foo;]])],
    [warning_CFLAGS="${warning_CFLAGS} -Wno-pointer-sign"], [])
  CFLAGS="$CFLAGS_saved"
], [
  dnl Vendor supplied C compilers are a bit tricky
  AS_CASE([$host_os],
    dnl SGI IRIX with the MipsPRO C compiler
    [irix*], [
      CFLAGS="$CFLAGS -fullwarn"
      AC_COMPILE_IFELSE([
        AC_LANG_PROGRAM(
          [[#include <stdio.h>]],
          [[printf("test");]])],
        [warning_CFLAGS="-fullwarn"], []
      )
    ],
    dnl SunOS 4.x with the SparcWorks(?) acc compiler
    [sunos*], [
      AS_IF([test "$CC" = "acc"], [
        CFLAGS="$CFLAGS -vc"
        AC_COMPILE_IFELSE([
          AC_LANG_PROGRAM(
            [[#include <stdio.h>]],
            [[printf("test");]])],
          [warning_CFLAGS="-vc"], []
        )
      ])
    ]
    dnl Unknown, do nothing
    [*], [
      warning_CFLAGS="none"
    ]
  )
])
CFLAGS="$ac_save_CFLAGS"

AS_IF([test "$warning_CFLAGS" = "none"], [
  AC_MSG_RESULT([none])
], [
  CFLAGS="$CFLAGS $warning_CFLAGS"
  AC_MSG_RESULT([$warning_CFLAGS])
])

# bash-completion
AC_ARG_WITH([bashcompletiondir],
  AS_HELP_STRING([--with-bashcompletiondir=DIR], [Bash completions directory]),
  [],
  [AS_IF([`$PKG_CONFIG --exists bash-completion`], [
    with_bashcompletiondir=`$PKG_CONFIG --variable=completionsdir bash-completion`
  ], [
    with_bashcompletiondir=${datadir}/bash-completion/completions
  ])
])
AC_SUBST([bashcompletiondir], [$with_bashcompletiondir])
AC_ARG_ENABLE([bash-completion],
  AS_HELP_STRING([--disable-bash-completion], [do not install bash completion files]),
  [], [enable_bash_completion=yes]
)
AM_CONDITIONAL([BUILD_BASH_COMPLETION], [test "x$enable_bash_completion" = xyes])

# Prepare config.h, Makefile, and output them.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
