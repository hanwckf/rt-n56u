# Check if a given program is available with a particular version.
#   CTNG_PROG_VERSION(VAR, HELP, PROG, SRCH, VERSION_CHECK[, CONFIG_OPT])
# Search for PROG under possible names of SRCH. Allow user overrides in variable
# VAR; display HELP message. Try to find a version that satisfies VERSION_CHECK
# regexp; if that is achieved, set CONFIG_OPT in the kconfig. Otherwise, settle
# for any version found.
# Sets ctng_version_VAR_ok to ':' if the version met the criterion, or false otherwise.
AC_DEFUN([CTNG_PROG_VERSION],
    [AS_IF([test -z "$EGREP"],
         [AC_MSG_ERROR([This macro can only be used after checking for EGREP])])
     CTNG_WITH_DEPRECATED([$3], [$1])
     AC_ARG_VAR([$1], [Specify the full path to $2])
     ctng_version_$1_ok=false
     # If a variable is already set, check if it an absolute path and convert if not.
     # Autoconf's AC_PATH_PROGS* macros just blindly trust $1 to be correct - but
     # AWK is set to just the command name by AC_INIT.
     AS_IF([test -n "$$1"],
         [ac_cv_path_$1="$$1"
          CTNG_PATH_ABSNAME([ac_cv_path_$1])
          CTNG_MSG_LOG_ENVVAR([ac_cv_path_$1])
          ver=$(eval $ac_cv_path_$1 --version 2>&1)
          CTNG_MSG_LOG([looking for '[$5]' regexp in])
          CTNG_MSG_LOG_ENVVAR([ver], [version info for $ac_cv_path_$1])
          ver=$(AS_ECHO(["$ver"]) | $EGREP '[$5]')
          test -n "$ver" && ctng_version_$1_ok=:],
         [AC_CACHE_CHECK([for $3], [ac_cv_path_$1],
             [AC_PATH_PROGS_FEATURE_CHECK([$1], [$4],
                  [CTNG_MSG_LOG_ENVVAR([ac_path_$1], [checking $1 at])
                   ver=$($ac_path_$1 --version 2>&1)
                   CTNG_MSG_LOG([looking for '[$5]' regexp in])
                   CTNG_MSG_LOG_ENVVAR([ver], [version info])
                   ver=$(AS_ECHO(["$ver"]) | $EGREP '[$5]')
                   test -z "$ac_cv_path_$1" && ac_cv_path_$1=$ac_path_$1
                   test -n "$ver" && ac_cv_path_$1="$ac_path_$1" ac_path_$1_found=: ctng_version_$1_ok=:])])])
     AC_MSG_CHECKING([for $2])
     AS_IF([$ctng_version_$1_ok],
         [AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])])
     AC_SUBST([$1], [$ac_cv_path_$1])
     AS_IF([test -n "$6"],
         [AS_IF([$ctng_version_$1_ok], [$6=y], [$6=])
          CTNG_SET_KCONFIG_OPTION([$6])])
    ])

# Same as above, but make it a fatal error if the tool is not found at all
# (i.e. "require any version, prefer version X or newer")
AC_DEFUN([CTNG_PROG_VERSION_REQ_ANY],
    [CTNG_PROG_VERSION([$1], [$2], [$3], [$4], [$5], [$6])
     AS_IF([test -z "$$1"],
         [AC_MSG_ERROR([Required tool not found: $3])])
    ])

# Same, but also require the version check to pass
# (i.e. "require version X or newer")
AC_DEFUN([CTNG_PROG_VERSION_REQ_STRICT],
    [CTNG_PROG_VERSION([$1], [$2], [$3], [$4], [$5], [$6])
     AS_IF([test -z "$$1" || ! $ctng_version_$1_ok],
         [AC_MSG_ERROR([Required tool not found: $2])])
    ])

