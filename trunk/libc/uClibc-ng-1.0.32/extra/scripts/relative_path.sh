#! /bin/sh
#
# Copyright 2003 Alexandre Oliva <aoliva@redhat.com>
# Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
#

# This script computes a relative pathname from $1 to $2
# They are assumed to not contain . or .. pathname components,
# but if both directories exist and cd/pwd canonicalizes pathnames,
# this shouldn't matter.  PWD_CMD may be set to some pwd command that does.

from=`(cd $1 > /dev/null && ${PWD_CMD-pwd} || echo $1) 2>/dev/null | sed 's,//*,/,g;s,/*$,,'`
target=`(cd $2 > /dev/null && ${PWD_CMD-pwd} || echo $2) 2>/dev/null | sed 's,//*,/,g;s,/*$,,'`

case $from in /* | "") ;; *) from=`${PWD_CMD-pwd}`/$from ;; esac
case $target in /* | "") ;; *) target=`${PWD_CMD-pwd}`/$target ;; esac

case $target in
"$from" | "$from/"*)
  dots=`echo $from | sed s,.,.,g`
  echo $target | sed "s,^$dots/*,,;s,[^/]$,&/,"
  exit 0
  ;;
esac

case $from in
"$target/"*)
  dots=`echo $target | sed s,.,.,g`
  echo $from/ | sed "s,^$dots/*,,;s,[^/]$,&/,;s,[^/]*/*,../,g;s,[^/]$,&/,"
  exit 0
  ;;
esac

# Without trailing slash, from=/usr/lib and target=/uclibc/lib
# mistakenly concludes that prefix=/u
#prefix=`echo $from///$target | sed 's,\(\(/[^/]*\)*\).*///\1.*,\1,'`
prefix=`echo $from///$target | sed 's,\(\(/[^/]*\)*/\).*///\1.*,\1,'`
dots=`echo $prefix | sed s,.,.,g`
from=`echo $from | sed "s,^$dots,,"`
target=`echo $target | sed "s,^$dots,,"`

from=`echo $from | sed 's,[^/][^/]*,..,g;s,.$,&/,'`
echo ${from}$target/

exit 0
