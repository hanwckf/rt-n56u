# CTNG_MSG_LOG(MSG)
# Log the MSG message to config.log
AC_DEFUN([CTNG_MSG_LOG],
   [AS_ECHO(["$as_me:${as_lineno-$LINENO}: AS_ESCAPE([$1])"]) >&AS_MESSAGE_LOG_FD])

# CTNG_MSG_LOG_CMD(CMD, [DESC])
# Log the output of a command CMD to config.log, prepending the
# description DESC.
AC_DEFUN([CTNG_MSG_LOG_CMD],
    [AS_IF([test -n "AS_ESCAPE([$2])"],
           [AS_ECHO(["$as_me:${as_lineno-$LINENO}: AS_ESCAPE([$2]):"]) >&AS_MESSAGE_LOG_FD],
           [AS_ECHO(["$as_me:${as_lineno-$LINENO}: output from command '$1':"]) >&AS_MESSAGE_LOG_FD])
     $1 | sed 's/^/| /' >&AS_MESSAGE_LOG_FD])

# CTNG_MSG_LOG_ENVVAR(VAR, [DESC])
# Log the contents of an environment variable VAR to config.log, prepending the
# description DESC.
AC_DEFUN([CTNG_MSG_LOG_ENVVAR],
    [AS_IF([test -n "AS_ESCAPE([$2])"],
           [CTNG_MSG_LOG_CMD([AS_ECHO(["$$1"])], [$2])],
           [CTNG_MSG_LOG_CMD([AS_ECHO(["$$1"])], [variable $1 is set to])])])

# CTNG_MSG_LOG_FILE(FILE, [DESC])
# Log the contents of a file FILE to config.log, prepending the
# description DESC.
AC_DEFUN([CTNG_MSG_LOG_FILE],
    [AS_IF([test -n "AS_ESCAPE([$2])"],
           [CTNG_MSG_LOG_CMD([cat $1], [$2])],
           [CTNG_MSG_LOG_CMD([cat $1], [contents of $1])])])
