#  Copyright 2009 Christoph Brill.
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  on the rights to use, copy, modify, merge, publish, distribute, sub
#  license, and/or sell copies of the Software, and to permit persons to whom
#  the Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice (including the next
#  paragraph) shall be included in all copies or substantial portions of the
#  Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
#  ADAM JACKSON BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Process this file with autoconf to produce a configure script

AC_PREREQ(2.57)
AC_INIT([inadyn],
		1.96.3)

AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_AUX_DIR(.)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([arpa/inet.h] [netdb.h] [netinet/in.h] [sys/socket.h] [syslog.h])

# Check for functions.
AC_CHECK_FUNCS([gethostbyname memset socket strchr strerror strpbrk strstr])
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_PID_T
AC_TYPE_UID_T

AC_MSG_CHECKING([Checking build target])
case ${target} in
  *-linux* | *-irix* | *-darwin* | *-solaris* | *-aix* | *-osf*)
    AC_MSG_RESULT([POSIX compatible])
    ;;
  *-mingw* | *-cygwin*)
    AC_MSG_RESULT([Windows])
    NEED_WINSOCK="yes"
    ;;
  *)
    echo Warning: Host type ${target} not yet supported.
    ;;
esac
AM_CONDITIONAL([NEED_WINSOCK], [test "x${NEED_WINSOCK}" = "xyes"])
if test "x${NEED_WINSOCK}" = "xyes" ; then
	AC_DEFINE(NEED_WINSOCK, 1, [Linker needs to include winsock])
fi

AM_CONDITIONAL([use_cache_file], [test "x${use_cache_file}" = "xyes"])
if test "x${use_cache_file}" = "xyes" ; then
	AC_DEFINE(USE_CACHE_FILE, 1, [Use cache file instead of cache dir])
fi

AC_OUTPUT([Makefile
		src/Makefile
		include/Makefile])
