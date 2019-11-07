#! /bin/sh

# usage:
#
# make \
#	REAL_CC=gcc-mine \
#	CC=extra/scripts/cppcheck.sh \
#	CPPCHECK_FLAGS="--enable=style,performance,portability,information,missingInclude --max-configs=256 -j $(($(getconf _NPROCESSORS_ONLN)-1))" \
#	CPPCHECK_LIMIT="yes"

# CPPCHECK_FLAGS are optional and are not set per default.
# CPPCHECK_LIMIT limits cppcheck to the -D and -U that would be passed to CC.
# Setting CPPCHECK_LIMIT greatly improves the check-time but obviously
# just checks a small subset of the defines found in a file.

: ${REAL_CC:=gcc}
${REAL_CC} $@
args=""
limits=""
next_arg=0
next_limit=0

for i in $@
do
  if [ $next_arg -eq 1 ] ; then
	next_arg=0
	case "/$i" in
	/-*) exit 0 ;;
	esac
    [ "x$args" = "x" ] && args="$i" || args="$args $i"
	continue
  fi
  if [ $next_limit -eq 1 ] ; then
	next_limit=0
    [ "x$limits" = "x" ] && limits="$i" || limits="$limits $i"
	continue
  fi
  case "/$i" in
  /-c) next_arg=1 ;;
  /-isystem)
		next_arg=1;
		[ "x$args" = "x" ] && args="-I" || args="$args -I" ;;
  /-I)
		next_arg=1;
		[ "x$args" = "x" ] && args="$i" || args="$args $i" ;;
  /-I*) [ "x$args" = "x" ] && args="$i" || args="$args $i" ;;
  /-D|/-U)
		next_limit=1;
		[ "x$limit" = "x" ] && limit="$i" || limit="$limit $i" ;;
  /-D*) [ "x$limits" = "x" ] && limits="$i" || limits="$limits $i" ;;
  /-s|/-S|/-dump*|/--print*|/-print*) exit 0 ;;
  *) ;;
  esac
done
[ -z "${CPPCHECK_LIMIT}" ] && limits=""
[ -z "${args}" ] || exec cppcheck ${CPPCHECK_FLAGS} ${args} ${limits}
