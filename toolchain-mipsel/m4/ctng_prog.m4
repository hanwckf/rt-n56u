# Several convenience wrappers for checking the programs

# Convert a pre-set tool variable to absolute path if it is not already.
AC_DEFUN([CTNG_PATH_ABSNAME],
    [CTNG_MSG_LOG_ENVVAR([$1], [must determine absolute path for '$$1'])
     AS_CASE([$$1],
         [/*],,
         [*\ *],,
         [?*],[AC_MSG_CHECKING([for absolute path to $$1])
               $1=$(which $$1)
               AC_MSG_RESULT([$$1])])])

# Check for required tool
AC_DEFUN([CTNG_CHECK_TOOL_REQ],
    [AC_CHECK_TOOLS([$1], [$2])
     AS_IF(
        [test -z "$$1"],
        [AC_MSG_ERROR([missing required tool: $2])])
    ])

# Check for required tool, set variable to full pathname
AC_DEFUN([CTNG_PATH_TOOL_REQ],
    [AC_ARG_VAR([$1], [Specify the full path to GNU $3])
     CTNG_CHECK_TOOL_REQ([$1], [$2])
     CTNG_PATH_ABSNAME([$1])])

# Check for required program
AC_DEFUN([CTNG_CHECK_PROGS_REQ],
    [AC_CHECK_PROGS([$1], [$2])
     AS_IF(
        [test -z "$$1"],
        [AC_MSG_ERROR([missing required tool: $2])])
    ])

# Check for path to required program
AC_DEFUN([CTNG_PATH_PROGS_REQ],
    [AC_PATH_PROGS([$1], [$2])
     AS_IF(
        [test -z "$$1"],
        [AC_MSG_ERROR([missing required tool: $2])])
   ])
