dnl GNU Parted - a library and front end for manipulation hard disk partitions
dnl Copyright (C) 1998-2002, 2005-2012 Free Software Foundation, Inc.
dnl
dnl This file may be modified and/or distributed without restriction.

AC_PREREQ([2.63])
AC_INIT([GNU parted], m4_esyscmd([build-aux/git-version-gen .version]),
        [bug-parted@gnu.org])

# When the most recent signed tag is v1.8.8.1,
# the above might set e.g., PACKAGE_VERSION='1.8.8.1.1-0bfc'

AC_CONFIG_SRCDIR([include/parted/parted.in.h])

AC_CONFIG_HEADERS([lib/config.h:lib/config.h.in])
AC_CONFIG_AUX_DIR([build-aux])

dnl Versioning
dnl Shamelessly pulled straight from glib's configure.in ...
dnl Making releases:
dnl    PED_MICRO_VERSION += 1;
dnl    PED_INTERFACE_AGE += 1;
dnl    PED_BINARY_AGE += 1;
dnl if any functions have been added, set PED_INTERFACE_AGE to 0.
dnl if backwards compatibility has been broken (eg. functions removed,
dnl function signatures changed),
dnl set PED_BINARY_AGE _and_ PED_INTERFACE_AGE to 0.

# Derive these numbers from $PACKAGE_VERSION, which is set
# when autoconf creates configure (see AC_INIT, above).
PED_MAJOR_VERSION=`echo "$PACKAGE_VERSION"|sed 's/\..*//'`
PED_MINOR_VERSION=`echo "$PACKAGE_VERSION"|sed 's/^[[0-9]][[0-9]]*\.//;s/\..*//'`
case $PACKAGE_VERSION in
  *.*.*.*.*)
    PED_MICRO_VERSION=`echo "$PACKAGE_VERSION" \
      | sed 's/^[^.]*\.[^.]*\.//;s/\..*//'`
    ;;
  *) PED_MICRO_VERSION=0;;
esac

PED_INTERFACE_AGE=0
PED_BINARY_AGE=0
PED_VERSION_SUFFIX=
PED_VERSION=$PED_MAJOR_VERSION.$PED_MINOR_VERSION.$PED_MICRO_VERSION$PED_VERSION_SUFFIX

LT_RELEASE=$PED_MAJOR_VERSION.$PED_MINOR_VERSION
LT_CURRENT=`expr $PED_MICRO_VERSION - $PED_INTERFACE_AGE`
LT_REVISION=$PED_INTERFACE_AGE
LT_AGE=`expr $PED_BINARY_AGE - $PED_INTERFACE_AGE`
AC_SUBST([LT_RELEASE])
AC_SUBST([LT_CURRENT])
AC_SUBST([LT_REVISION])
AC_SUBST([LT_AGE])

AM_INIT_AUTOMAKE([1.11 no-dist-gzip dist-xz color-tests parallel-tests])
AM_SILENT_RULES([yes]) # make --enable-silent-rules the default.

AC_CANONICAL_HOST
case "$host_os" in
	linux*) OS=linux ;;
	gnu*)	OS=gnu ;;
	beos*)	OS=beos ;;
	*)	AC_MSG_ERROR([Unknown or unsupported OS "$host_os".  Only "linux", "gnu" and "beos" are supported in this version of GNU Parted.]) ;;
esac
AC_SUBST([OS])

dnl Command-line options
AC_ARG_WITH([readline],
	[  --with-readline         support fancy command line editing], ,
	with_readline=yes
)

AC_ARG_ENABLE([mtrace],
	[  --enable-mtrace         enable malloc() debugging], ,
	enable_mtrace=no
)
if test "$enable_mtrace" = yes; then
	AC_DEFINE([ENABLE_MTRACE], [1], [Mtrace malloc() debugging])
fi

AC_SUBST([ENABLE_DEVICE_MAPPER])
ENABLE_DEVICE_MAPPER=yes
AC_ARG_ENABLE([device-mapper],
	[  --enable-device-mapper  enable device mapper support [default=yes]],
	[ENABLE_DEVICE_MAPPER=$enable_device_mapper])
if test $ENABLE_DEVICE_MAPPER = yes; then
	AC_DEFINE([ENABLE_DEVICE_MAPPER],
                  1, [device mapper (libdevmapper) support])
fi

AC_ARG_ENABLE([selinux],
	[  --enable-selinux        enable SELinux support [default=no]], ,
	enable_selinux=no
)

AC_ARG_ENABLE([discover-only],
	[  --enable-discover-only  support only reading/probing [default=no]], ,
	enable_discover_only=no
)
if test "$enable_discover_only" = yes; then
	AC_DEFINE([DISCOVER_ONLY], [1], [Probing functionality only])
fi

PARTED_LIBS=""
AC_ARG_ENABLE([dynamic-loading],
[  --enable-dynamic-loading  support dynamic fs libraries [default=yes]], ,
	if test "$enable_discover_only" = yes; then
		enable_dynamic_loading=no
	else
		enable_dynamic_loading=yes
	fi
)
if test "$enable_discover_only" = yes \
 && test "$enable_dynamic_loading" = yes; then
	AC_MSG_ERROR(
[You can't use --enable-dynamic-loading and --disable-discover-only together]
	)
fi

AC_ARG_ENABLE([debug],
	[  --enable-debug          compile in assertions [default=yes]], ,
	enable_debug=yes
)

if test "$enable_debug" = yes; then
	AC_DEFINE([DEBUG], [1], [Enable assertions, etc.])
fi

AC_ARG_ENABLE([read-only],
	[  --enable-read-only      disable writing (for debugging) [default=no]]
	, ,
	enable_read_only=no
)
if test "$enable_read_only" = yes; then
	AC_DEFINE([READ_ONLY], [1], [Disable all writing code])
fi

PARTEDLDFLAGS=
AC_SUBST([PARTEDLDFLAGS])

AC_ARG_ENABLE([pc98],
	[  --enable-pc98          build with pc98 support [default=yes]], ,
	enable_pc98=yes
)
if test "$enable_pc98" = yes; then
	AC_DEFINE([ENABLE_PC98], [1],
		  [Include PC98 partition tables.  (Sometimes excluded to avoid
		   collisions with msdos partition tables])
fi

AC_ARG_ENABLE([hfs-extract-fs],
	[  --enable-hfs-extract-fs Extract special HFS files for debugging [default=no]], ,
	enable_hfs_extract_fs=no
)
if test "$enable_hfs_extract_fs" = yes; then
       AC_DEFINE([HFS_EXTRACT_FS], [1],
                 [Extract low level special HFS(+) files for debugging purposes
                  when using the "check" command (NOT FOR PACKAGING)])
fi

dnl make libc threadsafe (not required for us, but useful other users of
dnl libparted)
AM_CPPFLAGS="$AM_CPPFLAGS -D_REENTRANT"

dnl Check for programs.
AC_ISC_POSIX
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AM_PROG_CC_C_O

gl_EARLY
parted_FIND_USABLE_TEST_DIR

dnl This test must come as early as possible after the compiler configuration
dnl tests, because the choice of the file model can (in principle) affect
dnl whether functions and headers are available, whether they work, etc.
AC_SYS_LARGEFILE

gl_INIT

AC_ARG_ENABLE([gcc-warnings],
  [AS_HELP_STRING([--enable-gcc-warnings],
		  [turn on lots of GCC warnings (for developers)])],
  [case $enableval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $enableval for gcc-warnings option]) ;;
   esac
   gl_gcc_warnings=$enableval],
  [gl_gcc_warnings=no]
)

if test "$gl_gcc_warnings" = yes; then
  gl_WARN_ADD([-Werror], [WERROR_CFLAGS])
  AC_SUBST([WERROR_CFLAGS])

  nw=
  # This, $nw, is the list of warnings we disable.
  nw="$nw -Wdeclaration-after-statement" # too useful to forbid
  nw="$nw -Waggregate-return"       # anachronistic
  nw="$nw -Wlong-long"              # C90 is anachronistic (lib/gethrxtime.h)
  nw="$nw -Wc++-compat"             # We don't care about C++ compilers
  nw="$nw -Wundef"                  # Warns on '#if GNULIB_FOO' etc in gnulib
  nw="$nw -Wtraditional"            # Warns on #elif which we use often
  nw="$nw -Wcast-qual"              # Too many warnings for now
  nw="$nw -Wconversion"             # Too many warnings for now
  nw="$nw -Wsystem-headers"         # Don't let system headers trigger warnings
  nw="$nw -Wsign-conversion"        # Too many warnings for now
  nw="$nw -Wtraditional-conversion" # Too many warnings for now
  nw="$nw -Wunreachable-code"       # Too many warnings for now
  nw="$nw -Wpadded"                 # Our structs are not padded
  nw="$nw -Wredundant-decls"        # openat.h declares e.g., mkdirat
  nw="$nw -Wlogical-op"             # any use of fwrite provokes this
  nw="$nw -Wvla"                    # warnings in gettext.h
  nw="$nw -Wnested-externs"         # use of XARGMATCH/verify_function__
  nw="$nw -Wswitch-enum"            # Too many warnings for now
  nw="$nw -Wswitch-default"         # Too many warnings for now
  # Too many warnings for now
  nw="$nw -Wattributes"
  nw="$nw -Wstrict-prototypes"
  nw="$nw -Wold-style-definition"
  nw="$nw -Wpacked"
  nw="$nw -Wmissing-prototypes"
  nw="$nw -Wmissing-declarations"
  nw="$nw -Wmissing-noreturn"
  # things I might fix soon:
  nw="$nw -Wfloat-equal"            # sort.c, seq.c
  nw="$nw -Wmissing-format-attribute" # copy.c
  nw="$nw -Wunsafe-loop-optimizations" # a few src/*.c
  nw="$nw -Winline"                 # system.h's readdir_ignoring_dot_and_dotdot
  nw="$nw -Wstrict-overflow"        # expr.c, pr.c, tr.c, factor.c
  nw="$nw -Wstack-protector"        # libparted/label/gpt.c
  # ?? -Wstrict-overflow

  gl_MANYWARN_ALL_GCC([ws])
  gl_MANYWARN_COMPLEMENT([ws], [$ws], [$nw])
  for w in $ws; do
    gl_WARN_ADD([$w])
  done
  gl_WARN_ADD([-Wno-missing-field-initializers]) # We need this one
  gl_WARN_ADD([-Wno-sign-compare])     # Too many warnings for now
  gl_WARN_ADD([-Wno-pointer-sign])     # Too many warnings for now
  gl_WARN_ADD([-Wno-unused-parameter]) # Too many warnings for now
  gl_WARN_ADD([-Wno-unused-macros]) # Too many warnings for now
  gl_WARN_ADD([-Wno-format-nonliteral])

  # FIXME: investigate these
  gl_WARN_ADD([-Wno-jump-misses-init])

  # In spite of excluding -Wlogical-op above, it is enabled, as of
  # gcc 4.5.0 20090517, and it provokes warnings in cat.c, dd.c, truncate.c
  gl_WARN_ADD([-Wno-logical-op])

  gl_WARN_ADD([-fdiagnostics-show-option])

  AC_SUBST([WARN_CFLAGS])

  AC_DEFINE([lint], [1], [Define to 1 if the compiler is checking for lint.])
  AC_DEFINE([_FORTIFY_SOURCE], [2],
    [enable compile-time and run-time bounds-checking, and some warnings])
  AC_DEFINE([GNULIB_PORTCHECK], [1], [enable some gnulib portability checks])
fi



AC_CHECK_SIZEOF([off_t], [64], [
	#include <stdio.h>
	#include <sys/types.h>
	#include <unistd.h>
])

AM_ENABLE_SHARED
if test "$OS" = linux && test $ac_cv_sizeof_off_t -lt 8; then
	dnl Need to disable shared libraries, to get llseek() to work.  Long
	dnl story.  The short story is: lseek() isn't in glibc, so a syscall
	dnl must be made.  syscalls can't be made from within shared libraries,
	dnl because of a bug (?) in gcc.
	AC_MSG_WARN(
off_t is less than 8 bytes.  Using llseek syscall, and disabling shared
libraries.)
	AM_DISABLE_SHARED
fi
AC_PROG_LIBTOOL
LT_INIT

AM_GNU_GETTEXT_VERSION([0.18])
AM_GNU_GETTEXT([external])
if test "$USE_INCLUDED_LIBINTL" = "yes"; then
	AC_MSG_ERROR([
GNU Parted requires gettext to be installed for compilation -
if native language support is desired.  Either disable native language support
with:
	$ ./configure --disable-nls
Or install gettext.  GNU gettext is available from
	http://ftp.gnu.org/gnu/gettext
])
fi

dnl Check for libdl, if we are doing dynamic loading
DL_LIBS=""
AC_SUBST([DYNAMIC_LOADING])
DYNAMIC_LOADING=no
if test "$enable_dynamic_loading" = yes; then
	AC_CHECK_LIB([dl], [dlopen],
		DL_LIBS="-ldl"
		PARTED_LIBS="$PARTED_LIBS -ldl"
		DYNAMIC_LOADING=yes
		AC_DEFINE([DYNAMIC_LOADING], [1], [Lazy linking to fs libs]),
		AC_MSG_ERROR(
			[-ldl not found!  Try using --disable-dynamic-loading]
		)
	)
fi
AC_SUBST([DL_LIBS])

dnl Check for libuuid
UUID_LIBS=""
AC_CHECK_LIB([uuid], [uuid_generate], [UUID_LIBS="-luuid"],
	[AC_MSG_ERROR(dnl
[GNU Parted requires libuuid - a part of the util-linux-ng package (but
usually distributed separately in libuuid-devel, uuid-dev or similar)
This can probably be found on your distribution's CD or FTP site or at:
    http://userweb.kernel.org/~kzak/util-linux-ng/
Note: originally, libuuid was part of the e2fsprogs package.  Later, it
moved to util-linux-ng-2.16, and that package is now the preferred source.])])
AC_SUBST([UUID_LIBS])

dnl Check for libdevmapper
DM_LIBS=
if test $ENABLE_DEVICE_MAPPER = yes; then
  AC_CHECK_LIB([devmapper], [dm_task_create],
    [DM_LIBS=-ldevmapper],
    [AC_MSG_ERROR(dnl
[libdevmapper could not be found, but is required for the
--enable-device-mapper option, which is enabled by default.  Either disable
device-mapper support with --disable-device-mapper or download and install
device-mapper from:
	http://sources.redhat.com/dm/
Note: if you are using precompiled packages you will need the development
package as well (it may be called device-mapper-devel or something similar).]
    )]
  )
fi
AC_SUBST([DM_LIBS])

dnl Check for SELinux
SELINUX_LIBS=""
if test "$enable_selinux" = yes; then
	SELINUX_LIBS="-lselinux -lsepol"
fi
AC_SUBST([SELINUX_LIBS])

dnl Check for libreiserfs
REISER_LIBS=""
if test "$enable_dynamic_loading" = no && test "$enable_discover_only" = no; then
	OLD_LIBS="$LIBS"
	AC_CHECK_LIB([dal], [dal_equals],
		LIBS="-ldal"
		AC_CHECK_LIB([reiserfs], [reiserfs_fs_probe],
			REISER_LIBS="-ldal -lreiserfs"
			AC_DEFINE([HAVE_LIBREISERFS], [1], [Have libreiserfs])
		)
		AC_CHECK_LIB([reiserfs], [reiserfs_fs_check],
			AC_DEFINE([HAVE_REISERFS_FS_CHECK], [1], [Have reiserfs_fs_check()])
		)
	)
	LIBS="$OLD_LIBS"
fi
AC_SUBST([REISER_LIBS])

dnl Check for termcap
if test "$with_readline" = yes; then
	OLD_LIBS="$LIBS"
	LIBS=""
	AC_SEARCH_LIBS([tgetent], [tinfo ncurses curses termcap termlib],
		[PARTED_LIBS="$PARTED_LIBS $LIBS"],
		[AC_MSG_ERROR(dnl
[termcap could not be found which is required for the
--with-readline option (which is enabled by default).  Either disable readline
support with --without-readline or download and install termcap from:
	ftp.gnu.org/gnu/termcap
Note: if you are using precompiled packages you will also need the development
  package as well (which may be called termcap-devel or something similar).
Note: (n)curses also seems to work as a substitute for termcap.  This was
  not found either - but you could try installing that as well.])])
	LIBS="$OLD_LIBS"
fi

dnl Check for readline
dnl NOTE: We need to remove the gl_cv_ignore_unused_libraries flag if we
dnl detected one earlier.  libreadline on some platforms (e.g., RHEL and
dnl Fedora) is left with
if test "$with_readline" = yes; then
	OLD_LIBS="$LIBS"
	LIBS="$LIBS $PARTED_LIBS"
	found_working_libreadline=no
	AC_CHECK_LIB([readline], [readline],
		found_working_libreadline=yes,
		AC_MSG_ERROR(
[GNU Readline could not be found which is required for the
--with-readline (which is enabled by default).  Either disable readline support with
--without-readline or downloaded and install it from:
        ftp.gnu.org/gnu/readline
Note: if you are using precompiled packages you will also need the development
package as well (which may be called readline-devel or something similar).
])
		,
		$PARTED_LIBS
	)
	LIBS="$OLD_LIBS $PARTED_LIBS"

	# See if libreadline is too old to be used.
	# The readline function in Debian's libreadline5 5.0-10 fails to
	# print to stdout the response (from stdin) to a prompt, when stdout
	# is redirected, while 5.2-3 works fine.  That failure would cause
	# several of parted's tests to failure.
	# The purist approach would be to write a run-test, but that's
	# not friendly to cross-compilers, so here's a compromise:
	#
	# See if libreadline defines one of these symbols:
	# [this is the list of public symbols that are in 5.2, but not 5.0]
	#
	# $ diff -u /tmp/readline-5.[02]|grep '+T.[^_]'
	# +T rl_vi_rubout
	# +T rl_variable_value
	# +T rl_reset_screen_size
	# +T alloc_history_entry
	# +T copy_history_entry
	# +T replace_history_data
	#
	# If not, then reject this readline lib.
	AC_CHECK_LIB([readline], [rl_variable_value],
		     ,
		     AC_MSG_ERROR(
[Your version of libreadline is too old to be used.
Consider upgrading to version 5.2 or newer.])
		     found_working_libreadline=no,
		     $PARTED_LIBS)

	if test $found_working_libreadline = yes; then
		PARTED_LIBS="-lreadline $PARTED_LIBS"
		AC_DEFINE([HAVE_LIBREADLINE], [1], [have readline])
	fi
	LIBS="$OLD_LIBS"
fi

AC_SUBST([PARTED_LIBS])

dnl Check for OS specific libraries

dnl GNU/Hurd:
if test "$OS" = gnu; then
dnl libshouldbeinlibc
	AC_CHECK_LIB([shouldbeinlibc], [vm_deallocate],
		OS_LIBS="$OS_LIBS -lshouldbeinlibc",
		AC_MSG_ERROR(
[GNU Parted requires libshouldbeinlibc when running on
GNU/Hurd systems.  It is a standard part of a GNU/Hurd system.]
		)
	)

dnl libstore may depend on libparted being present.
dnl Aren't circular dependencies wonderful?
	OLD_LIBS="$LIBS"
	LIBS=

	AC_CHECK_LIB([parted], [ped_device_read])

dnl libstore
	AC_CHECK_LIB([store], [store_open],
		OS_LIBS="$OS_LIBS -lstore",
		AC_MSG_ERROR(
[GNU Parted requires libstore when running on GNU/Hurd
systems.  It is a standard part of a GNU/Hurd system.]
		)
		,
		$OS_LIBS $UUID_LIBS $DM_LIBS $LIBS
	)
	LIBS="$OLD_LIBS"
fi

dnl BeOS/ZETA/Haiku:

if test "$OS" = beos; then
	dnl Include the socket library, as it is a depedency of libuuid
	dnl and so also of us (due to socket() call in libuuid)
	OS_LIBS="$OS_LIBS -lsocket"
fi

AC_SUBST([OS_LIBS])

dnl One day, gettext might support libtool...
dnl if test "$USE_INCLUDED_LIBINTL" = "yes"; then
dnl	INTLINCS='-I$(top_srcdir)/intl'
dnl fi
AC_SUBST([INTLINCS])


dnl Checks for header files.
AC_CHECK_HEADER([uuid/uuid.h], ,
	[AC_MSG_ERROR(
[GNU Parted requires libuuid - a part of the e2fsprogs package.
You seem to have the library installed but not the headers.  These are usually
found in a corresponding development package (usually called e2fsprogs-devel).
If you can't find one try:
        http://web.mit.edu/tytso/www/linux/e2fsprogs.html])]
)

AC_CHECK_HEADERS([getopt.h])
AC_CHECK_HEADERS([linux/ext2_fs.h])

dnl required for libparted/llseek.c  (TODO: make linux-x86 only)
if test "$OS" = linux; then
	AC_CHECK_HEADER([linux/unistd.h])
fi

if test "$with_readline" = yes; then
	AC_CHECK_HEADERS([readline/readline.h readline/history.h], ,
		[AC_MSG_ERROR(
[The headers for GNU Readline could not be found which
are required for the --with-readline option.  You seem to have the GNU readline
library installed but not the headers.  These are usually found in a
corresponding development package (usually called readline-devel).  If you can't
find one try:
        ftp.gnu.org/gnu/readline
Alternatively you can disable readline support with --without-readline])]
	)
fi

AC_CHECK_HEADERS([termcap.h])

if test "$USE_NLS" = yes; then
	AC_CHECK_HEADERS([wctype.h], ,
		[AC_MSG_ERROR(
[One or more of the header files that are required for
native language support (wctype.h) could not be found.  Either get a newer
version of GNU libc and its headers - which can be obtained from:
	ftp.gnu.org/gnu/glibc
Or disable native language support with the --disable-nls option])]
	)
fi

AC_CHECK_HEADER([execinfo.h], [
	AC_CHECK_LIB([c], [backtrace], [
		AC_DEFINE([HAVE_BACKTRACE], [1], [Has backtrace support])
		LDFLAGS="$LDFLAGS -rdynamic"
	])
])

AM_CONDITIONAL([COMPILE_FOR_S390], [test "$host_cpu" = s390 || test "$host_cpu" = s390x])

dnl check for "check", unit testing library/header
PKG_CHECK_MODULES([CHECK], [check >= 0.9.3], have_check=yes, have_check=no)
if test "$have_check" != "yes"; then
    AC_MSG_RESULT([Unable to locate check version 0.9.3 or higher: not building])
fi
AM_CONDITIONAL([HAVE_CHECK], [test "$have_check" = yes])

dnl Checks for typedefs, structures and compiler characteristics.
AC_PROG_LD

AC_C_BIGENDIAN
AC_C_INLINE
AC_C_CONST
AC_C_RESTRICT

dnl Checks for library functions.
AC_CHECK_FUNCS([sigaction])
AC_CHECK_FUNCS([getuid])

dnl NOTE: We need to remove the gl_cv_ignore_unused_libraries flag if we
dnl detected one earlier.  libreadline on some platforms (e.g., RHEL and
dnl Fedora) is left with
if test "$with_readline" = yes; then
	OLD_LIBS="$LIBS"
	LIBS="$LIBS $PARTED_LIBS -lreadline"
	AC_CHECK_FUNCS([rl_completion_matches])
	LIBS="$OLD_LIBS"
fi

AC_CHECK_FUNCS([canonicalize_file_name])

DATE=$(date '+%d %b %Y %H:%M')
USER=$(whoami)
HOST=$(hostname)
BUILDINFO="$USER@$HOST, $DATE"
AC_SUBST([BUILDINFO])

LIB_BLKID=
AC_SUBST([LIB_BLKID])
pe_saved_libs=$LIBS
  AC_SEARCH_LIBS([blkid_probe_get_topology], [blkid],
    [test "$ac_cv_search_blkid_probe_get_topology" = "none required" \
     || LIB_BLKID=$ac_cv_search_blkid_probe_get_topology])
  AC_CHECK_FUNC([blkid_probe_get_topology], [use_blkid=1], [use_blkid=0])
LIBS=$pe_saved_libs
AC_DEFINE_UNQUOTED([USE_BLKID], [$use_blkid],
  [Define if you have sufficient blkid support.])
AC_CHECK_HEADERS_ONCE([blkid/blkid.h])

AC_OUTPUT([
Makefile
lib/Makefile
include/Makefile
include/parted/Makefile
libparted/Makefile
libparted/labels/Makefile
libparted/fs/Makefile
libparted/tests/Makefile
libparted.pc
parted/Makefile
partprobe/Makefile
doc/Makefile
doc/C/Makefile
doc/pt_BR/Makefile
debug/Makefile
debug/test/Makefile
tests/Makefile
po/Makefile.in
])

echo
echo Type \'make\' to compile parted.
