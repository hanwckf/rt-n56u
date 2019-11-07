# FIXME retire after 1.24
#
# CTNG_WITH_DEPRECATED(PROG, VAR)
#   Declare a deprecated --with option: instead of --with-PROG=xxx, must use VAR=xxx
AC_DEFUN([CTNG_WITH_DEPRECATED],
    [AC_ARG_WITH([$1],
                 [AS_HELP_STRING([--with-$1=PATH],
                                 [Deprecated; use $2=PATH instead])],
                 [AC_MSG_ERROR([--with-$1=$withval deprecated; use $2=$withval instead])])
    ])
