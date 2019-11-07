#!/bin/sh

# build random configurations
# Usage:
# ARCH=i386 nohup ./extra/scripts/randconfig.sh & sleep 1800 && touch STOP
#
# The above builds random i386 configs and automatically stops after 30 minutes

test "x$AWK" = "x" && AWK=awk
test "x$ARCH" = "x" && ARCH=`uname -m`
KCONFIG_ALLCONFIG=.config.allconfig
(echo TARGET_$ARCH=y
) > $KCONFIG_ALLCONFIG
export KCONFIG_ALLCONFIG

if test "x$NCPU" = "x"
then
  test -r /proc/cpuinfo && \
  eval `$AWK 'BEGIN{NCPU=0}
/processor/{let NCPU++}
END{if (NCPU<1) {NCPU=1}; print("NCPU="NCPU);}' /proc/cpuinfo` || \
  NCPU=1
fi
MAKELEVEL="-j$NCPU"
i=0
while test ! -f STOP
do
  ARCH=$ARCH make $* randconfig > /dev/null
  ARCH=$ARCH make $* silentoldconfig > /dev/null
  if (make $MAKELEVEL $*) 2>&1 >& mk.log
  then
    :
  else
    i=`expr $i + 1`
    num=`printf "%.5d" $i`
    mv .config FAILED.$num.config
    mv mk.log FAILED.$num.log
  fi
  make distclean > /dev/null || true
done
rm -f STOP
