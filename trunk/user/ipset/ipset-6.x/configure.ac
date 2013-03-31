dnl Boilerplate
AC_INIT([ipset], [6.17], [kadlec@blackhole.kfki.hu])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([foreign subdir-objects tar-pax])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_ENABLE_STATIC
LT_INIT([dlopen])
LT_CONFIG_LTDL_DIR([libltdl])
LTDL_INIT([nonrecursive])

dnl Shortcut: Linux supported alone
case "$host" in
*-*-linux*) ;;
*) AC_MSG_ERROR([Linux systems supported exclusively!]);;
esac

dnl Optionnally disable building the kernel module
AC_ARG_WITH([kmod],
            AS_HELP_STRING([--with-kmod=yes/no],
                           [Build the kernel module (default: yes)]),
            [BUILDKMOD="$withval";],
            [BUILDKMOD="yes";])
AM_CONDITIONAL(WITH_KMOD, test "$BUILDKMOD" == "yes")

dnl Additional arguments
dnl Kernel build directory or source tree
AC_ARG_WITH([kbuild],
            AS_HELP_STRING([--with-kbuild=PATH],
                           [Path to kernel build directory]),
            [KBUILDDIR="$withval";])
AC_ARG_WITH([ksource],
            AS_HELP_STRING([--with-ksource=PATH],
                           [Path to kernel source directory, if not the same as the kernel build directory]),
            [KSOURCEDIR="$withval";])
AM_CONDITIONAL(WITH_KBUILDDIR, test "$KBUILDDIR" != "")
AC_SUBST(KBUILDDIR)

if test "$BUILDKMOD" == "yes"
then
dnl Sigh: check kernel version dependencies
if test "$KBUILDDIR" != ""
then
	kbuilddir="$KBUILDDIR"
else
	kbuilddir="/lib/modules/`uname -r`/build"
fi

if test -n "$KSOURCEDIR"; then
	ksourcedir="$KSOURCEDIR"
elif test -e "$kbuilddir/include/linux/netfilter/nfnetlink.h"; then
	ksourcedir="$kbuilddir"
else
	ksourcedir="/lib/modules/$(uname -r)/source"
fi
if test ! -e "$ksourcedir/include/linux/netfilter/nfnetlink.h"
then
	AC_MSG_ERROR([Invalid kernel source directory $ksourcedir])
fi

if test ! -e "$kbuilddir/.config"
then
	AC_MSG_ERROR([The kernel build directory $kbuilddir is not configured])
fi

AC_PROG_GREP

if ! $GREP -q "NFNL_SUBSYS_IPSET" "$ksourcedir/include/linux/netfilter/nfnetlink.h" && \
   ! $GREP -q "NFNL_SUBSYS_IPSET" "$ksourcedir/include/uapi/linux/netfilter/nfnetlink.h";
then
	AC_MSG_ERROR([The kernel source directory $ksourcedir is not patched with netlink.patch to support ipset])
fi
fi

dnl Maximal number of sets supported by the kernel, default 256
AC_ARG_WITH([maxsets],
	    AS_HELP_STRING([--with-maxsets=256],
	    		   [Maximal numer of sets supported by the kernel]),
	    [MAXSETS="$withval";])
AM_CONDITIONAL(WITH_MAXSETS, test "$MAXSETS" != "")
AC_SUBST(MAXSETS)

dnl Verbose compiling
AC_ARG_ENABLE([verbose],
	      AS_HELP_STRING([--enable-verbose],
	                     [Enable verbose mode at compiling/linking.]),
	      [case "${enableval}" in
	       yes)	enable_verbose=yes ;;
	       no)	enable_verbose=no ;;
	       *) AC_MSG_ERROR([bad value ${enableval} for --enable-verbose]) ;;
	       esac], [enable_verbose=no])
	      
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug], [enable debug messages @<:@default=disabled@:>@]),
	[], [enable_debug=no])
AS_IF([test "x$enable_debug" = "xyes"], [
	AC_DEFINE(ENABLE_DEBUG, [1], [Debug messages.])
])
AM_CONDITIONAL([ENABLE_DEBUG], [test "x$enable_debug" = xyes])

dnl Enable type modules
AC_ARG_ENABLE([settype_modules],
	          AS_HELP_STRING([--enable-settype-modules],
	          [Enable set type modules support]),
	          [enable_settype_modules="$enableval"],
	          [enable_settype_modules="no"])

AC_ARG_WITH([settype_modules_list],
	        AS_HELP_STRING([--with-settype-modules-list="mod1 mod2 ..."],
	                       [List of dynamic loading modules, ignored if settype-modules is disabled. It could be "all" to build all available settypes as modules]),
	                       [SETTYPE_MODLIST_RAW="$withval";])

SETTYPE_MODLIST=
if test "x$enable_settype_modules" = "xyes"; then
	for mod in $SETTYPE_MODLIST_RAW; do
		if echo $mod | grep "all"; then
			m="${mod}"
		else
			if echo $mod | grep "ipset_"; then
				m="${mod}.c"
			else
				m="ipset_${mod}.c"
			fi
		fi

		SETTYPE_MODLIST="${SETTYPE_MODLIST} $m"
	done

	AC_MSG_RESULT([checking for configuration with dynamic loading modules... $SETTYPE_MODLIST_RAW])
fi
AC_SUBST(SETTYPE_MODLIST)

AM_CONDITIONAL([ENABLE_SETTYPE_MODULES], [test "x$enable_settype_modules" = xyes])

AM_CONDITIONAL([ENABLE_STATIC], [test "x$enable_static" = xyes])
AM_CONDITIONAL([ENABLE_SHARED], [test "x$enable_shared" = xyes])

dnl Checks for programs
: ${CFLAGS=""}

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S

dnl Checks for libraries
PKG_CHECK_MODULES([libmnl], [libmnl >= 1])

dnl Checks for header files

dnl Checks for declarations
AC_CHECK_DECLS([NLA_F_NESTED, NLA_F_NET_BYTEORDER, NLA_TYPE_MASK],,
		[AC_MSG_ERROR([System kernel header files are older than 2.6.24, use CFLAGS for non-default location])],
		[#include <sys/socket.h>
#include <linux/netlink.h>])

dnl Checks for typedefs, structures
AC_CHECK_TYPES([union nf_inet_addr],,,[#include <linux/types.h>
#include <netinet/in.h>
#include <linux/netfilter.h>])

dnl Checks for functions
AC_CHECK_FUNCS(gethostbyname2)

dnl Checks for  compiler characteristics.
dnl Check extra warning flags except
dnl	-Wconversion		-> we need it
dnl	-Wunreachable-code	-> fails with ntoh*
dnl	-Wpointer-arith		-> limbnl uses it
dnl	-Wsign-conversion	-> libmnl
if test "x$enable_debug" = "xyes"
then
AX_CFLAGS_GCC_OPTION(-Waggregate-return)
AX_CFLAGS_GCC_OPTION(-Wbad-function-cast)
AX_CFLAGS_GCC_OPTION(-Wcast-align)
AX_CFLAGS_GCC_OPTION(-Wcast-qual)
AX_CFLAGS_GCC_OPTION(-Werror)
AX_CFLAGS_GCC_OPTION(-Wextra)
AX_CFLAGS_GCC_OPTION(-Wfloat-equal)
AX_CFLAGS_GCC_OPTION(-Wformat=2)
AX_CFLAGS_GCC_OPTION(-Wjump-misses-init)
AX_CFLAGS_GCC_OPTION(-Winit-self)
AX_CFLAGS_GCC_OPTION(-Winline)
AX_CFLAGS_GCC_OPTION(-Wlogical-op)
AX_CFLAGS_GCC_OPTION(-Wmissing-declarations)
AX_CFLAGS_GCC_OPTION(-Wmissing-format-attribute)
AX_CFLAGS_GCC_OPTION(-Wmissing-prototypes)
AX_CFLAGS_GCC_OPTION(-Wnested-externs)
AX_CFLAGS_GCC_OPTION(-Wno-missing-field-initializers)
AX_CFLAGS_GCC_OPTION(-Wold-style-definition)
AX_CFLAGS_GCC_OPTION(-Woverlength-strings)
AX_CFLAGS_GCC_OPTION(-Wpacked)
AX_CFLAGS_GCC_OPTION(-Wredundant-decls)
AX_CFLAGS_GCC_OPTION(-Wrwrite-strings)
AX_CFLAGS_GCC_OPTION(-Wshadow)
AX_CFLAGS_GCC_OPTION(-Wsign-compare)
AX_CFLAGS_GCC_OPTION(-Wstrict-prototypes)
AX_CFLAGS_GCC_OPTION(-Wswitch-default)
AX_CFLAGS_GCC_OPTION(-Wundef)
AX_CFLAGS_GCC_OPTION(-Wuninitialized)
AX_CFLAGS_GCC_OPTION(-Wunused)
AX_CFLAGS_GCC_OPTION(-Wvla)
AX_CFLAGS_GCC_OPTION(-Wwrite-strings)
fi
dnl Checks for library functions.

dnl Generate output
AC_CONFIG_FILES([Makefile include/libipset/Makefile
	lib/Makefile lib/libipset.pc src/Makefile])
AC_OUTPUT

dnl Summary
AC_MSG_RESULT([])
AC_MSG_RESULT([$PACKAGE userspace tool configuration:])
if test "x$enable_settype_modules" != "xyes"; then
	AC_MSG_RESULT([    Dynamic module loading: disabled])
else
	AC_MSG_RESULT([    Dynamic module loading: enabled])
fi
IPSET_ALL_MODULES="`ls ${srcdir}/lib/ipset_*.c|sed -e 's/^.*lib\///' -e 's/\.c$//'`"
AC_MSG_RESULT([    Static modules:])
if test "x$SETTYPE_MODLIST" = "x"; then
	for mod in $IPSET_ALL_MODULES; do
		AC_MSG_RESULT([        ${mod}])
	done
	AC_MSG_RESULT([    Dynamic modules:])
elif echo $SETTYPE_MODLIST | grep "all" >/dev/null; then
	AC_MSG_RESULT([    Dynamic modules:])
	for mod in $IPSET_ALL_MODULES; do
		AC_MSG_RESULT([        ${mod}])
	done
else
	for mod in $IPSET_ALL_MODULES; do
		if echo $SETTYPE_MODLIST | grep $mod >/dev/null; then
			:
		else
			AC_MSG_RESULT([        ${mod}])
		fi
	done
	AC_MSG_RESULT([    Dynamic modules:])
	for mod in $IPSET_ALL_MODULES; do
		if echo $SETTYPE_MODLIST | grep $mod >/dev/null; then
			AC_MSG_RESULT([        ${mod}])
		fi
	done
fi
